/*
 * LayerShellWindow implementation
 */

#include "layershellwindow.h"

#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

/* ======================================================================
 * LayerShellV1 - Global extension
 * ====================================================================== */

LayerShellV1::LayerShellV1()
    : QWaylandClientExtensionTemplate<LayerShellV1>(4 /* version */) {
    initialize();
}

/* ======================================================================
 * LayerSurfaceV1 - Per-surface
 * ====================================================================== */

LayerSurfaceV1::LayerSurfaceV1(LayerShellV1 *shell,
                               struct ::wl_surface *surface,
                               struct ::wl_output *output,
                               LayerShellV1::Layer layer,
                               const QString &nameSpace)
    : QtWayland::zwlr_layer_surface_v1(
          shell->get_layer_surface(surface, output,
                                   static_cast<uint32_t>(layer),
                                   nameSpace)) {
}

LayerSurfaceV1::~LayerSurfaceV1() {
    destroy();
}

void LayerSurfaceV1::setAnchor(uint32_t anchor) {
    set_anchor(anchor);
}

void LayerSurfaceV1::setExclusiveZone(int32_t zone) {
    set_exclusive_zone(zone);
}

void LayerSurfaceV1::setSize(uint32_t width, uint32_t height) {
    set_size(width, height);
}

void LayerSurfaceV1::setMargin(int32_t top, int32_t right,
                               int32_t bottom, int32_t left) {
    set_margin(top, right, bottom, left);
}

void LayerSurfaceV1::setKeyboardInteractivity(uint32_t mode) {
    set_keyboard_interactivity(mode);
}

void LayerSurfaceV1::zwlr_layer_surface_v1_configure(uint32_t serial,
                                                      uint32_t width,
                                                      uint32_t height) {
    emit configured(serial, width, height);
    ack_configure(serial);
}

void LayerSurfaceV1::zwlr_layer_surface_v1_closed() {
    emit closed();
}

/* ======================================================================
 * LayerShellWindow
 * ====================================================================== */

LayerShellWindow::LayerShellWindow(QQuickWindow *window, QObject *parent)
    : QObject(parent), m_window(window) {
    m_shell = new LayerShellV1();

    /* Wait for the window to have a native Wayland surface */
    connect(m_window, &QWindow::visibleChanged, this, [this](bool visible) {
        if (visible) {
            QTimer::singleShot(0, this, &LayerShellWindow::onNativeReady);
        }
    });
}

LayerShellWindow::~LayerShellWindow() {
    delete m_surface;
    delete m_shell;
}

void LayerShellWindow::setPanelHeight(int height) {
    if (m_panelHeight != height) {
        m_panelHeight = height;
        emit panelHeightChanged();
    }
}

void LayerShellWindow::attachToLayer() {
    m_window->show();
}

void LayerShellWindow::onNativeReady() {
    if (!m_shell->isActive()) {
        qWarning("LayerShellWindow: layer-shell extension not available");
        return;
    }

    auto *waylandWindow =
        dynamic_cast<QtWaylandClient::QWaylandWindow *>(m_window->handle());
    if (!waylandWindow) {
        qWarning("LayerShellWindow: not a Wayland window");
        return;
    }

    struct ::wl_surface *wlSurface = waylandWindow->wlSurface();
    if (!wlSurface) {
        qWarning("LayerShellWindow: no wl_surface available");
        return;
    }

    /* Create the layer surface: bottom layer, anchored to bottom edge */
    m_surface = new LayerSurfaceV1(
        m_shell, wlSurface, nullptr,
        LayerShellV1::LayerTop,
        QStringLiteral("nakumi-panel"));

    /* Anchor to left+bottom+right (full-width dock at bottom) */
    const uint32_t anchor =
        QtWayland::zwlr_layer_surface_v1::anchor_bottom |
        QtWayland::zwlr_layer_surface_v1::anchor_left |
        QtWayland::zwlr_layer_surface_v1::anchor_right;
    m_surface->setAnchor(anchor);
    m_surface->setSize(0, m_panelHeight);
    m_surface->setExclusiveZone(m_panelHeight);
    m_surface->setKeyboardInteractivity(0);

    connect(m_surface, &LayerSurfaceV1::configured,
            this, &LayerShellWindow::onConfigured);
    connect(m_surface, &LayerSurfaceV1::closed,
            qApp, &QCoreApplication::quit);

    /* Commit the surface to trigger the configure event */
    wl_surface_commit(wlSurface);

    m_ready = true;
    emit readyChanged();
}

void LayerShellWindow::onConfigured(uint32_t serial,
                                    uint32_t width, uint32_t height) {
    if (width > 0 && height > 0) {
        m_window->resize(static_cast<int>(width), static_cast<int>(height));
    }
}
