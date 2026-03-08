/**
 * NakumiOS Wayland Compositor
 * 
 * A QtWayland-based compositor providing a modern, fluid desktop experience.
 * Manages window decoration, animations, and Wayland protocol handling.
 */

#ifndef NAKUMIOS_COMPOSITOR_H
#define NAKUMIOS_COMPOSITOR_H

#include <QObject>
#include <QQmlEngine>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandOutput>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandXdgShell>
#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QtWaylandCompositor/QWaylandXdgToplevel>
#include <QtWaylandCompositor/QWaylandQuickOutput>

#include <QList>
#include <QQuickWindow>

namespace NakumiOS {

class WindowManager;

class Compositor : public QWaylandCompositor
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(WindowManager* windowManager READ windowManager CONSTANT)
    Q_PROPERTY(QWaylandSeat* defaultSeat READ defaultSeat CONSTANT)

public:
    explicit Compositor(QQuickWindow *window = nullptr);
    ~Compositor() override;
    
    WindowManager *windowManager() const;
    
    Q_INVOKABLE void handleShellSurface(QWaylandXdgSurface *xdgSurface);

signals:
    void surfaceCreated(QWaylandSurface *surface);
    void surfaceDestroyed(QWaylandSurface *surface);
    void xdgToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface);

public slots:
    void onSurfaceCreated(QWaylandSurface *surface);
    void onXdgSurfaceCreated(QWaylandXdgSurface *xdgSurface);
    void onToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface);

protected:
    void create() override;

private:
    QQuickWindow *m_window;
    QWaylandQuickOutput *m_output;
    QWaylandXdgShell *m_xdgShell;
    WindowManager *m_windowManager;
};

} // namespace NakumiOS

#endif // NAKUMIOS_COMPOSITOR_H
