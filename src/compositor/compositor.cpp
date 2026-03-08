/**
 * NakumiOS Wayland Compositor Implementation
 */

#include "compositor.h"
#include "windowmanager.h"

#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandQuickCompositor>

namespace NakumiOS {

Compositor::Compositor(QQuickWindow *window)
    : QWaylandCompositor()
    , m_window(window)
    , m_output(nullptr)
    , m_xdgShell(nullptr)
    , m_windowManager(new WindowManager(this))
{
    if (m_window) {
        // Create the compositor when the window is ready
        create();
    }
}

Compositor::~Compositor()
{
    delete m_windowManager;
}

void Compositor::create()
{
    // Create the XDG shell for window management
    m_xdgShell = new QWaylandXdgShell(this);
    
    // Connect XDG shell signals
    connect(m_xdgShell, &QWaylandXdgShell::xdgSurfaceCreated,
            this, &Compositor::onXdgSurfaceCreated);
    connect(m_xdgShell, &QWaylandXdgShell::toplevelCreated,
            this, &Compositor::onToplevelCreated);
    
    // Connect surface signals
    connect(this, &QWaylandCompositor::surfaceCreated,
            this, &Compositor::onSurfaceCreated);
    
    // Create output if we have a window
    if (m_window) {
        m_output = new QWaylandQuickOutput(this, m_window);
        m_output->setAutomaticFrameCallback(true);
    }
    
    // Call parent create
    QWaylandCompositor::create();
}

WindowManager *Compositor::windowManager() const
{
    return m_windowManager;
}

void Compositor::onSurfaceCreated(QWaylandSurface *surface)
{
    emit surfaceCreated(surface);
    
    connect(surface, &QWaylandSurface::destroyed, this, [this, surface]() {
        emit surfaceDestroyed(surface);
    });
}

void Compositor::onXdgSurfaceCreated(QWaylandXdgSurface *xdgSurface)
{
    handleShellSurface(xdgSurface);
}

void Compositor::onToplevelCreated(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface)
{
    m_windowManager->addWindow(toplevel, xdgSurface);
    emit xdgToplevelCreated(toplevel, xdgSurface);
}

void Compositor::handleShellSurface(QWaylandXdgSurface *xdgSurface)
{
    // The surface will handle its own configuration
    Q_UNUSED(xdgSurface)
}

} // namespace NakumiOS
