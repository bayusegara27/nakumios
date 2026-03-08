/*
 * LayerShellWindow - C++ wrapper for wlr-layer-shell-unstable-v1 protocol
 *
 * This class handles the layer-shell Wayland protocol to position the panel
 * as a dock at the bottom of the screen, with exclusive zone reservation.
 *
 * IMPORTANT: Qt6 does NOT expose layer-shell natively to QML.
 * We use qtwaylandscanner-generated bindings to interact with the protocol.
 */

#ifndef LAYERSHELLWINDOW_H
#define LAYERSHELLWINDOW_H

#include <QObject>
#include <QQuickWindow>
#include <QWaylandClientExtension>
#include <wayland-client.h>

#include "qwayland-wlr-layer-shell-unstable-v1.h"
#include "wayland-wlr-layer-shell-unstable-v1-client-protocol.h"

/* ======================================================================
 * LayerShellV1 - Global registry object for zwlr_layer_shell_v1
 * ====================================================================== */

class LayerShellV1 : public QWaylandClientExtensionTemplate<LayerShellV1>,
                     public QtWayland::zwlr_layer_shell_v1 {
    Q_OBJECT
public:
    explicit LayerShellV1();

    /* Layer types matching the protocol enum */
    enum Layer {
        LayerBackground = 0,
        LayerBottom      = 1,
        LayerTop         = 2,
        LayerOverlay     = 3,
    };
    Q_ENUM(Layer)
};

/* ======================================================================
 * LayerSurfaceV1 - Per-surface layer shell state
 * ====================================================================== */

class LayerSurfaceV1 : public QObject,
                       public QtWayland::zwlr_layer_surface_v1 {
    Q_OBJECT
public:
    explicit LayerSurfaceV1(LayerShellV1 *shell,
                            struct ::wl_surface *surface,
                            struct ::wl_output *output,
                            LayerShellV1::Layer layer,
                            const QString &nameSpace);
    ~LayerSurfaceV1() override;

    void setAnchor(uint32_t anchor);
    void setExclusiveZone(int32_t zone);
    void setSize(uint32_t width, uint32_t height);
    void setMargin(int32_t top, int32_t right, int32_t bottom, int32_t left);
    void setKeyboardInteractivity(uint32_t mode);

signals:
    void configured(uint32_t serial, uint32_t width, uint32_t height);
    void closed();

protected:
    /* Overrides from QtWayland::zwlr_layer_surface_v1 */
    void zwlr_layer_surface_v1_configure(uint32_t serial, uint32_t width,
                                         uint32_t height) override;
    void zwlr_layer_surface_v1_closed() override;
};

/* ======================================================================
 * LayerShellWindow - High-level QQuickWindow subclass with layer-shell
 * ====================================================================== */

class LayerShellWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(int panelHeight READ panelHeight WRITE setPanelHeight NOTIFY panelHeightChanged)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

public:
    explicit LayerShellWindow(QQuickWindow *window, QObject *parent = nullptr);
    ~LayerShellWindow() override;

    int panelHeight() const { return m_panelHeight; }
    void setPanelHeight(int height);
    bool isReady() const { return m_ready; }

    /* Attach layer-shell semantics to the window */
    Q_INVOKABLE void attachToLayer();

signals:
    void panelHeightChanged();
    void readyChanged();

private slots:
    void onNativeReady();
    void onConfigured(uint32_t serial, uint32_t width, uint32_t height);

private:
    QQuickWindow *m_window = nullptr;
    LayerShellV1 *m_shell = nullptr;
    LayerSurfaceV1 *m_surface = nullptr;
    int m_panelHeight = 48;
    bool m_ready = false;
};

#endif // LAYERSHELLWINDOW_H
