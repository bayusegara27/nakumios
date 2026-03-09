/*
 * nakumi-wm - NakumiOS Wayland Compositor
 *
 * A wlroots-based compositor using the wlr_scene graph API
 * for efficient rendering and damage tracking.
 *
 * Supports:
 *   - xdg-shell (standard application windows)
 *   - wlr-layer-shell-unstable-v1 (panels, launchers, overlays)
 *   - Keyboard/pointer input via libinput
 *   - Software rendering fallback (pixman) for VM compatibility
 */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include "wlr-layer-shell-unstable-v1-protocol.h"
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include <xkbcommon/xkbcommon.h>

/* ======================================================================
 * Data Structures
 * ====================================================================== */

enum nakumi_cursor_mode {
    NAKUMI_CURSOR_PASSTHROUGH,
    NAKUMI_CURSOR_MOVE,
    NAKUMI_CURSOR_RESIZE,
};

struct nakumi_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    /* Scene graph (wlr_scene API — no manual rendering) */
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    /* Scene tree layers for z-ordering */
    struct wlr_scene_tree *layer_background;
    struct wlr_scene_tree *layer_bottom;
    struct wlr_scene_tree *layer_tiled;
    struct wlr_scene_tree *layer_floating;
    struct wlr_scene_tree *layer_top;
    struct wlr_scene_tree *layer_overlay;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;

    /* XDG shell for standard app windows */
    struct wlr_xdg_shell *xdg_shell;
    struct wl_list toplevels;
    struct wl_listener new_xdg_toplevel;
    struct wl_listener new_xdg_popup;

    /* Layer shell for panels, docks, overlays */
    struct wlr_layer_shell_v1 *layer_shell;
    struct wl_list layer_surfaces;
    struct wl_listener new_layer_surface;

    /* Input */
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;

    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;

    enum nakumi_cursor_mode cursor_mode;
    struct nakumi_toplevel *grabbed_toplevel;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;
};

struct nakumi_output {
    struct wl_list link;
    struct nakumi_server *server;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

struct nakumi_toplevel {
    struct wl_list link;
    struct nakumi_server *server;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
};

struct nakumi_popup {
    struct wlr_xdg_popup *xdg_popup;
    struct wl_listener commit;
    struct wl_listener destroy;
};

struct nakumi_layer_surface {
    struct wl_list link;
    struct nakumi_server *server;
    struct wlr_layer_surface_v1 *layer_surface;
    struct wlr_scene_layer_surface_v1 *scene_layer;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
};

struct nakumi_keyboard {
    struct wl_list link;
    struct nakumi_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

/* ======================================================================
 * Focus Handling
 * ====================================================================== */

static void focus_toplevel(struct nakumi_toplevel *toplevel,
                           struct wlr_surface *surface) {
    if (toplevel == NULL) {
        return;
    }
    struct nakumi_server *server = toplevel->server;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;

    if (prev_surface == surface) {
        return; /* Already focused */
    }

    if (prev_surface) {
        struct wlr_xdg_toplevel *prev_toplevel =
            wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
        if (prev_toplevel != NULL) {
            wlr_xdg_toplevel_set_activated(prev_toplevel, false);
        }
    }

    /* Move the toplevel to the front of the scene tree */
    wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
    /* Move to front of server list */
    wl_list_remove(&toplevel->link);
    wl_list_insert(&server->toplevels, &toplevel->link);

    wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);

    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard != NULL) {
        wlr_seat_keyboard_notify_enter(seat, toplevel->xdg_toplevel->base->surface,
                                       keyboard->keycodes, keyboard->num_keycodes,
                                       &keyboard->modifiers);
    }
}

/* Find the topmost toplevel at (lx, ly) coordinates */
static struct nakumi_toplevel *desktop_toplevel_at(
    struct nakumi_server *server, double lx, double ly,
    struct wlr_surface **surface, double *sx, double *sy) {

    struct wlr_scene_node *node =
        wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
    if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) {
        return NULL;
    }
    *surface = scene_surface->surface;

    /* Walk up the scene tree to find the toplevel node */
    struct wlr_scene_tree *tree = node->parent;
    while (tree != NULL && tree->node.data == NULL) {
        tree = tree->node.parent;
    }
    if (tree == NULL) {
        return NULL;
    }
    return tree->node.data;
}

/* ======================================================================
 * Keyboard Handling
 * ====================================================================== */

static bool handle_keybinding(struct nakumi_server *server, xkb_keysym_t sym) {
    /* Compositor-level keybindings */
    switch (sym) {
    case XKB_KEY_Escape:
        /* Super+Escape: Terminate compositor */
        wl_display_terminate(server->wl_display);
        return true;
    default:
        return false;
    }
}

static void keyboard_handle_modifiers(struct wl_listener *listener,
                                      void *data) {
    struct nakumi_keyboard *keyboard =
        wl_container_of(listener, keyboard, modifiers);
    wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
                                       &keyboard->wlr_keyboard->modifiers);
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
    struct nakumi_keyboard *keyboard =
        wl_container_of(listener, keyboard, key);
    struct nakumi_server *server = keyboard->server;
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = server->seat;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms =
        xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
    if ((modifiers & WLR_MODIFIER_LOGO) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybinding(server, syms[i]);
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode,
                                     event->state);
    }
}

static void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
    struct nakumi_keyboard *keyboard =
        wl_container_of(listener, keyboard, destroy);
    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);
    free(keyboard);
}

static void server_new_keyboard(struct nakumi_server *server,
                                struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct nakumi_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    /* Set up XKB keymap */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap =
        xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    /* Set up listeners */
    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
    keyboard->destroy.notify = keyboard_handle_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wlr_seat_set_keyboard(server->seat, wlr_keyboard);
    wl_list_insert(&server->keyboards, &keyboard->link);
}

/* ======================================================================
 * Pointer / Cursor Handling
 * ====================================================================== */

static void process_cursor_move(struct nakumi_server *server, uint32_t time) {
    struct nakumi_toplevel *toplevel = server->grabbed_toplevel;
    wlr_scene_node_set_position(&toplevel->scene_tree->node,
                                server->cursor->x - server->grab_x,
                                server->cursor->y - server->grab_y);
}

static void process_cursor_resize(struct nakumi_server *server, uint32_t time) {
    struct nakumi_toplevel *toplevel = server->grabbed_toplevel;
    double border_x = server->cursor->x - server->grab_x;
    double border_y = server->cursor->y - server->grab_y;

    int new_left = server->grab_geobox.x;
    int new_right = server->grab_geobox.x + server->grab_geobox.width;
    int new_top = server->grab_geobox.y;
    int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

    if (server->resize_edges & WLR_EDGE_TOP) {
        new_top = border_y;
        if (new_top >= new_bottom) new_top = new_bottom - 1;
    } else if (server->resize_edges & WLR_EDGE_BOTTOM) {
        new_bottom = border_y;
        if (new_bottom <= new_top) new_bottom = new_top + 1;
    }
    if (server->resize_edges & WLR_EDGE_LEFT) {
        new_left = border_x;
        if (new_left >= new_right) new_left = new_right - 1;
    } else if (server->resize_edges & WLR_EDGE_RIGHT) {
        new_right = border_x;
        if (new_right <= new_left) new_right = new_left + 1;
    }

    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo_box);
    wlr_scene_node_set_position(&toplevel->scene_tree->node,
                                new_left - geo_box.x, new_top - geo_box.y);
    int new_width = new_right - new_left;
    int new_height = new_bottom - new_top;
    wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, new_width, new_height);
}

static void process_cursor_motion(struct nakumi_server *server, uint32_t time) {
    if (server->cursor_mode == NAKUMI_CURSOR_MOVE) {
        process_cursor_move(server, time);
        return;
    } else if (server->cursor_mode == NAKUMI_CURSOR_RESIZE) {
        process_cursor_resize(server, time);
        return;
    }

    /* Find toplevel under cursor */
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct nakumi_toplevel *toplevel =
        desktop_toplevel_at(server, server->cursor->x, server->cursor->y,
                            &surface, &sx, &sy);
    if (!toplevel) {
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
    }
    if (surface) {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(server->seat);
    }
}

static void server_cursor_motion(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, cursor_motion);
    struct wlr_pointer_motion_event *event = data;
    wlr_cursor_move(server->cursor, &event->pointer->base, event->delta_x,
                    event->delta_y);
    process_cursor_motion(server, event->time_msec);
}

static void server_cursor_motion_absolute(struct wl_listener *listener,
                                          void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x,
                             event->y);
    process_cursor_motion(server, event->time_msec);
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, cursor_button);
    struct wlr_pointer_button_event *event = data;
    wlr_seat_pointer_notify_button(server->seat, event->time_msec,
                                   event->button, event->state);
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct nakumi_toplevel *toplevel =
        desktop_toplevel_at(server, server->cursor->x, server->cursor->y,
                            &surface, &sx, &sy);
    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
        server->cursor_mode = NAKUMI_CURSOR_PASSTHROUGH;
    } else {
        focus_toplevel(toplevel, surface);
    }
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    wlr_seat_pointer_notify_axis(server->seat, event->time_msec,
                                 event->orientation, event->delta,
                                 event->delta_discrete, event->source,
                                 event->relative_direction);
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, cursor_frame);
    wlr_seat_pointer_notify_frame(server->seat);
}

/* ======================================================================
 * New Input Device
 * ====================================================================== */

static void server_new_pointer(struct nakumi_server *server,
                               struct wlr_input_device *device) {
    wlr_cursor_attach_input_device(server->cursor, device);
}

static void server_new_input(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        server_new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        server_new_pointer(server, device);
        break;
    default:
        break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

static void seat_request_cursor(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused_client =
        server->seat->pointer_state.focused_client;
    if (focused_client == event->seat_client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
                               event->hotspot_x, event->hotspot_y);
    }
}

static void seat_request_set_selection(struct wl_listener *listener,
                                       void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

/* ======================================================================
 * XDG Shell: Toplevel (Application Windows)
 * ====================================================================== */

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, map);
    wl_list_insert(&toplevel->server->toplevels, &toplevel->link);
    focus_toplevel(toplevel, toplevel->xdg_toplevel->base->surface);
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, unmap);
    if (toplevel == toplevel->server->grabbed_toplevel) {
        toplevel->server->grabbed_toplevel = NULL;
        toplevel->server->cursor_mode = NAKUMI_CURSOR_PASSTHROUGH;
    }
    wl_list_remove(&toplevel->link);
}

static void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, commit);
    if (toplevel->xdg_toplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
    }
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, destroy);
    wl_list_remove(&toplevel->map.link);
    wl_list_remove(&toplevel->unmap.link);
    wl_list_remove(&toplevel->commit.link);
    wl_list_remove(&toplevel->destroy.link);
    wl_list_remove(&toplevel->request_move.link);
    wl_list_remove(&toplevel->request_resize.link);
    wl_list_remove(&toplevel->request_maximize.link);
    wl_list_remove(&toplevel->request_fullscreen.link);
    free(toplevel);
}

static void begin_interactive(struct nakumi_toplevel *toplevel,
                              enum nakumi_cursor_mode mode, uint32_t edges) {
    struct nakumi_server *server = toplevel->server;
    struct wlr_surface *focused_surface =
        server->seat->pointer_state.focused_surface;
    if (toplevel->xdg_toplevel->base->surface !=
        wlr_surface_get_root_surface(focused_surface)) {
        return;
    }
    server->grabbed_toplevel = toplevel;
    server->cursor_mode = mode;

    if (mode == NAKUMI_CURSOR_MOVE) {
        server->grab_x = server->cursor->x - toplevel->scene_tree->node.x;
        server->grab_y = server->cursor->y - toplevel->scene_tree->node.y;
    } else {
        struct wlr_box geo_box;
        wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo_box);
        double border_x =
            (toplevel->scene_tree->node.x + geo_box.x) +
            ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
        double border_y =
            (toplevel->scene_tree->node.y + geo_box.y) +
            ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
        server->grab_x = server->cursor->x - border_x;
        server->grab_y = server->cursor->y - border_y;
        server->grab_geobox = geo_box;
        server->grab_geobox.x += toplevel->scene_tree->node.x;
        server->grab_geobox.y += toplevel->scene_tree->node.y;
        server->resize_edges = edges;
    }
}

static void xdg_toplevel_request_move(struct wl_listener *listener,
                                      void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, request_move);
    begin_interactive(toplevel, NAKUMI_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener,
                                        void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    begin_interactive(toplevel, NAKUMI_CURSOR_RESIZE, event->edges);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener,
                                          void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, request_maximize);
    if (toplevel->xdg_toplevel->base->surface->mapped) {
        wlr_xdg_toplevel_set_maximized(toplevel->xdg_toplevel, false);
    }
}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener,
                                            void *data) {
    struct nakumi_toplevel *toplevel =
        wl_container_of(listener, toplevel, request_fullscreen);
    if (toplevel->xdg_toplevel->base->surface->mapped) {
        wlr_xdg_toplevel_set_fullscreen(toplevel->xdg_toplevel, false);
    }
}

static void server_new_xdg_toplevel(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, new_xdg_toplevel);
    struct wlr_xdg_toplevel *xdg_toplevel = data;

    struct nakumi_toplevel *toplevel = calloc(1, sizeof(*toplevel));
    toplevel->server = server;
    toplevel->xdg_toplevel = xdg_toplevel;
    toplevel->scene_tree =
        wlr_scene_xdg_surface_create(server->layer_tiled,
                                     xdg_toplevel->base);
    toplevel->scene_tree->node.data = toplevel;
    xdg_toplevel->base->data = toplevel->scene_tree;

    toplevel->map.notify = xdg_toplevel_map;
    wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
    toplevel->unmap.notify = xdg_toplevel_unmap;
    wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
    toplevel->commit.notify = xdg_toplevel_commit;
    wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);
    toplevel->destroy.notify = xdg_toplevel_destroy;
    wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);

    toplevel->request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&xdg_toplevel->events.request_move, &toplevel->request_move);
    toplevel->request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&xdg_toplevel->events.request_resize, &toplevel->request_resize);
    toplevel->request_maximize.notify = xdg_toplevel_request_maximize;
    wl_signal_add(&xdg_toplevel->events.request_maximize, &toplevel->request_maximize);
    toplevel->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
    wl_signal_add(&xdg_toplevel->events.request_fullscreen, &toplevel->request_fullscreen);
}

/* ======================================================================
 * XDG Popup
 * ====================================================================== */

static void xdg_popup_commit(struct wl_listener *listener, void *data) {
    struct nakumi_popup *popup = wl_container_of(listener, popup, commit);
    if (popup->xdg_popup->base->initial_commit) {
        wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
    }
}

static void xdg_popup_destroy(struct wl_listener *listener, void *data) {
    struct nakumi_popup *popup = wl_container_of(listener, popup, destroy);
    wl_list_remove(&popup->commit.link);
    wl_list_remove(&popup->destroy.link);
    free(popup);
}

static void server_new_xdg_popup(struct wl_listener *listener, void *data) {
    struct wlr_xdg_popup *xdg_popup = data;

    struct nakumi_popup *popup = calloc(1, sizeof(*popup));
    popup->xdg_popup = xdg_popup;

    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
    assert(parent != NULL);
    struct wlr_scene_tree *parent_tree = parent->data;
    xdg_popup->base->data =
        wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);

    popup->commit.notify = xdg_popup_commit;
    wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);
    popup->destroy.notify = xdg_popup_destroy;
    wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}

/* ======================================================================
 * Layer Shell (Panels, Overlays, Docks)
 * ====================================================================== */

static struct wlr_scene_tree *layer_get_scene_tree(
    struct nakumi_server *server,
    enum zwlr_layer_shell_v1_layer layer_type) {
    switch (layer_type) {
    case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
        return server->layer_background;
    case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
        return server->layer_bottom;
    case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
        return server->layer_top;
    case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
        return server->layer_overlay;
    default:
        return server->layer_top;
    }
}

static void layer_surface_map(struct wl_listener *listener, void *data) {
    struct nakumi_layer_surface *layer =
        wl_container_of(listener, layer, map);
    wlr_log(WLR_INFO, "Layer surface mapped: %s",
             layer->layer_surface->namespace);
}

static void layer_surface_unmap(struct wl_listener *listener, void *data) {
    struct nakumi_layer_surface *layer =
        wl_container_of(listener, layer, unmap);
    wlr_log(WLR_INFO, "Layer surface unmapped: %s",
             layer->layer_surface->namespace);
}

static void layer_surface_commit(struct wl_listener *listener, void *data) {
    struct nakumi_layer_surface *layer =
        wl_container_of(listener, layer, commit);
    struct wlr_layer_surface_v1 *lsurf = layer->layer_surface;
    struct wlr_output *output = lsurf->output;

    if (!output) {
        return;
    }

    if (lsurf->initial_commit) {
        /* Arrange the layer surface on initial commit */
        struct wlr_box full_area = {0};
        wlr_output_effective_resolution(output,
                                        &full_area.width, &full_area.height);
        wlr_scene_layer_surface_v1_configure(layer->scene_layer,
                                             &full_area, &full_area);
    }
}

static void layer_surface_destroy(struct wl_listener *listener, void *data) {
    struct nakumi_layer_surface *layer =
        wl_container_of(listener, layer, destroy);
    wl_list_remove(&layer->map.link);
    wl_list_remove(&layer->unmap.link);
    wl_list_remove(&layer->commit.link);
    wl_list_remove(&layer->destroy.link);
    wl_list_remove(&layer->link);
    free(layer);
}

static void server_new_layer_surface(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, new_layer_surface);
    struct wlr_layer_surface_v1 *layer_surface = data;

    wlr_log(WLR_INFO, "New layer surface: namespace=%s, layer=%d",
             layer_surface->namespace, layer_surface->pending.layer);

    /* Assign to first output if none specified */
    if (!layer_surface->output) {
        struct nakumi_output *output;
        wl_list_for_each(output, &server->outputs, link) {
            layer_surface->output = output->wlr_output;
            break;
        }
    }

    if (!layer_surface->output) {
        wlr_layer_surface_v1_destroy(layer_surface);
        return;
    }

    struct nakumi_layer_surface *layer = calloc(1, sizeof(*layer));
    layer->server = server;
    layer->layer_surface = layer_surface;

    struct wlr_scene_tree *parent =
        layer_get_scene_tree(server, layer_surface->pending.layer);
    layer->scene_layer =
        wlr_scene_layer_surface_v1_create(parent, layer_surface);

    layer->map.notify = layer_surface_map;
    wl_signal_add(&layer_surface->surface->events.map, &layer->map);
    layer->unmap.notify = layer_surface_unmap;
    wl_signal_add(&layer_surface->surface->events.unmap, &layer->unmap);
    layer->commit.notify = layer_surface_commit;
    wl_signal_add(&layer_surface->surface->events.commit, &layer->commit);
    layer->destroy.notify = layer_surface_destroy;
    wl_signal_add(&layer_surface->events.destroy, &layer->destroy);

    wl_list_insert(&server->layer_surfaces, &layer->link);
}

/* ======================================================================
 * Output (Monitor) Handling
 * ====================================================================== */

static void output_frame(struct wl_listener *listener, void *data) {
    struct nakumi_output *output =
        wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;

    struct wlr_scene_output *scene_output =
        wlr_scene_get_scene_output(scene, output->wlr_output);
    wlr_scene_output_commit(scene_output, NULL);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void output_request_state(struct wl_listener *listener, void *data) {
    struct nakumi_output *output =
        wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    wlr_output_commit_state(output->wlr_output, event->state);
}

static void output_destroy(struct wl_listener *listener, void *data) {
    struct nakumi_output *output =
        wl_container_of(listener, output, destroy);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
    struct nakumi_server *server =
        wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    /* Initialize renderer on this output */
    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    /* Set preferred mode (typically native resolution) */
    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }
    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    struct nakumi_output *output = calloc(1, sizeof(*output));
    output->server = server;
    output->wlr_output = wlr_output;

    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->request_state.notify = output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);
    output->destroy.notify = output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);

    struct wlr_output_layout_output *l_output =
        wlr_output_layout_add_auto(server->output_layout, wlr_output);
    output->scene_output =
        wlr_scene_output_create(server->scene, wlr_output);
    wlr_scene_output_layout_add_output(server->scene_layout, l_output,
                                       output->scene_output);
}

/* ======================================================================
 * Server Initialization & Main
 * ====================================================================== */

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_DEBUG, NULL);

    char *startup_cmd = NULL;
    int c;
    while ((c = getopt(argc, argv, "s:h")) != -1) {
        switch (c) {
        case 's':
            startup_cmd = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-s startup command]\n", argv[0]);
            return 1;
        }
    }
    if (optind < argc) {
        fprintf(stderr, "Usage: %s [-s startup command]\n", argv[0]);
        return 1;
    }

    struct nakumi_server server = {0};

    /* Create Wayland display */
    server.wl_display = wl_display_create();
    assert(server.wl_display);

    /* Create backend (auto-detects DRM/KMS or headless) */
    server.backend = wlr_backend_autocreate(
        wl_display_get_event_loop(server.wl_display), NULL);
    assert(server.backend);

    /* Create renderer (GPU or pixman fallback) */
    server.renderer = wlr_renderer_autocreate(server.backend);
    assert(server.renderer);
    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    /* Create allocator */
    server.allocator =
        wlr_allocator_autocreate(server.backend, server.renderer);
    assert(server.allocator);

    /* Create compositor and subcompositor globals */
    wlr_compositor_create(server.wl_display, 5, server.renderer);
    wlr_subcompositor_create(server.wl_display);
    wlr_data_device_manager_create(server.wl_display);

    /* === Scene Graph Setup (wlr_scene API) ===
     * All rendering is handled by the scene graph — no manual render passes.
     * Layers provide z-ordering for different surface types. */
    server.scene = wlr_scene_create();
    server.layer_background = wlr_scene_tree_create(&server.scene->tree);
    server.layer_bottom      = wlr_scene_tree_create(&server.scene->tree);
    server.layer_tiled        = wlr_scene_tree_create(&server.scene->tree);
    server.layer_floating     = wlr_scene_tree_create(&server.scene->tree);
    server.layer_top          = wlr_scene_tree_create(&server.scene->tree);
    server.layer_overlay      = wlr_scene_tree_create(&server.scene->tree);

    /* Output layout and scene integration */
    server.output_layout = wlr_output_layout_create(server.wl_display);
    wl_list_init(&server.outputs);
    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);
    server.scene_layout =
        wlr_scene_attach_output_layout(server.scene, server.output_layout);

    /* === XDG Shell (Standard Application Windows) === */
    wl_list_init(&server.toplevels);
    server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 3);
    server.new_xdg_toplevel.notify = server_new_xdg_toplevel;
    wl_signal_add(&server.xdg_shell->events.new_toplevel,
                  &server.new_xdg_toplevel);
    server.new_xdg_popup.notify = server_new_xdg_popup;
    wl_signal_add(&server.xdg_shell->events.new_popup,
                  &server.new_xdg_popup);

    /* === Layer Shell (Panels, Docks, Overlays) ===
     * CRITICAL: Explicitly initialized and attached to the scene graph. */
    wl_list_init(&server.layer_surfaces);
    server.layer_shell = wlr_layer_shell_v1_create(server.wl_display, 4);
    server.new_layer_surface.notify = server_new_layer_surface;
    wl_signal_add(&server.layer_shell->events.new_surface,
                  &server.new_layer_surface);

    /* === Cursor === */
    server.cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server.cursor, server.output_layout);
    server.cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    server.cursor_mode = NAKUMI_CURSOR_PASSTHROUGH;
    server.cursor_motion.notify = server_cursor_motion;
    wl_signal_add(&server.cursor->events.motion, &server.cursor_motion);
    server.cursor_motion_absolute.notify = server_cursor_motion_absolute;
    wl_signal_add(&server.cursor->events.motion_absolute,
                  &server.cursor_motion_absolute);
    server.cursor_button.notify = server_cursor_button;
    wl_signal_add(&server.cursor->events.button, &server.cursor_button);
    server.cursor_axis.notify = server_cursor_axis;
    wl_signal_add(&server.cursor->events.axis, &server.cursor_axis);
    server.cursor_frame.notify = server_cursor_frame;
    wl_signal_add(&server.cursor->events.frame, &server.cursor_frame);

    /* === Seat (Input) === */
    wl_list_init(&server.keyboards);
    server.seat = wlr_seat_create(server.wl_display, "seat0");
    server.request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server.seat->events.request_set_cursor,
                  &server.request_cursor);
    server.request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server.seat->events.request_set_selection,
                  &server.request_set_selection);

    server.new_input.notify = server_new_input;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);

    /* === Start the Wayland socket === */
    const char *socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
        wlr_log(WLR_ERROR, "Failed to create Wayland socket");
        wlr_backend_destroy(server.backend);
        return 1;
    }

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        wlr_backend_destroy(server.backend);
        wl_display_destroy(server.wl_display);
        return 1;
    }

    setenv("WAYLAND_DISPLAY", socket, true);
    wlr_log(WLR_INFO,
             "NakumiOS compositor running on Wayland display %s", socket);

    /* Launch startup command (e.g., panel, launcher) */
    if (startup_cmd) {
        wlr_log(WLR_INFO, "Launching startup command: %s", startup_cmd);
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (char *)NULL);
            _exit(1);
        }
    }

    /* Run the event loop */
    wl_display_run(server.wl_display);

    /* Cleanup */
    wl_display_destroy_clients(server.wl_display);
    wlr_scene_node_destroy(&server.scene->tree.node);
    wlr_xcursor_manager_destroy(server.cursor_mgr);
    wlr_cursor_destroy(server.cursor);
    wlr_allocator_destroy(server.allocator);
    wlr_renderer_destroy(server.renderer);
    wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);

    return 0;
}
