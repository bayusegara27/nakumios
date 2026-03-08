/**
 * NakumiOS Window Manager Implementation
 */

#include "windowmanager.h"

#include <QtWaylandCompositor/QWaylandSurface>
#include <QGuiApplication>
#include <QScreen>

namespace NakumiOS {

WindowManager::WindowManager(QObject *parent)
    : QAbstractListModel(parent)
    , m_activeIndex(-1)
    , m_nextZOrder(0)
{
}

WindowManager::~WindowManager()
{
}

int WindowManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_windows.count();
}

QVariant WindowManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_windows.count()) {
        return QVariant();
    }
    
    const WindowInfo &info = m_windows.at(index.row());
    
    switch (role) {
    case ToplevelRole:
        return QVariant::fromValue(info.toplevel);
    case XdgSurfaceRole:
        return QVariant::fromValue(info.xdgSurface);
    case TitleRole:
        return info.toplevel ? info.toplevel->title() : QString();
    case AppIdRole:
        return info.toplevel ? info.toplevel->appId() : QString();
    case PositionXRole:
        return info.position.x();
    case PositionYRole:
        return info.position.y();
    case WidthRole:
        return info.size.width();
    case HeightRole:
        return info.size.height();
    case IsActiveRole:
        return info.isActive;
    case IsMinimizedRole:
        return info.isMinimized;
    case IsMaximizedRole:
        return info.isMaximized;
    case IsFullscreenRole:
        return info.isFullscreen;
    case ZOrderRole:
        return info.zOrder;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WindowManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ToplevelRole] = "toplevel";
    roles[XdgSurfaceRole] = "xdgSurface";
    roles[TitleRole] = "title";
    roles[AppIdRole] = "appId";
    roles[PositionXRole] = "positionX";
    roles[PositionYRole] = "positionY";
    roles[WidthRole] = "width";
    roles[HeightRole] = "height";
    roles[IsActiveRole] = "isActive";
    roles[IsMinimizedRole] = "isMinimized";
    roles[IsMaximizedRole] = "isMaximized";
    roles[IsFullscreenRole] = "isFullscreen";
    roles[ZOrderRole] = "zOrder";
    return roles;
}

int WindowManager::count() const
{
    return m_windows.count();
}

int WindowManager::activeWindowIndex() const
{
    return m_activeIndex;
}

void WindowManager::addWindow(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface)
{
    if (!toplevel || !xdgSurface) {
        return;
    }
    
    // Calculate initial position (cascade from top-left)
    int offset = m_windows.count() * 30;
    QPoint initialPos(100 + offset, 100 + offset);
    
    // Default size
    QSize initialSize(800, 600);
    
    WindowInfo info;
    info.toplevel = toplevel;
    info.xdgSurface = xdgSurface;
    info.position = initialPos;
    info.size = initialSize;
    info.isActive = true;
    info.isMinimized = false;
    info.isMaximized = false;
    info.isFullscreen = false;
    info.zOrder = m_nextZOrder++;
    
    // Connect signals
    connect(toplevel, &QObject::destroyed, this, &WindowManager::onToplevelDestroyed);
    connect(toplevel, &QWaylandXdgToplevel::titleChanged, this, &WindowManager::onTitleChanged);
    connect(toplevel, &QWaylandXdgToplevel::activatedChanged, this, &WindowManager::onActivatedChanged);
    
    // Configure the toplevel window
    toplevel->sendConfigure(initialSize, QList<QWaylandXdgToplevel::State>());
    
    beginInsertRows(QModelIndex(), m_windows.count(), m_windows.count());
    m_windows.append(info);
    endInsertRows();
    
    // Activate this new window
    int newIndex = m_windows.count() - 1;
    activateWindow(newIndex);
    
    emit countChanged();
    emit windowAdded(newIndex);
}

void WindowManager::removeWindow(QWaylandXdgToplevel *toplevel)
{
    int idx = findWindow(toplevel);
    if (idx < 0) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), idx, idx);
    m_windows.removeAt(idx);
    endRemoveRows();
    
    // Update active index
    if (m_activeIndex >= m_windows.count()) {
        m_activeIndex = m_windows.count() - 1;
        emit activeWindowChanged();
    } else if (m_activeIndex == idx) {
        m_activeIndex = m_windows.count() > 0 ? m_windows.count() - 1 : -1;
        if (m_activeIndex >= 0) {
            m_windows[m_activeIndex].isActive = true;
        }
        emit activeWindowChanged();
    }
    
    emit countChanged();
    emit windowRemoved(idx);
}

void WindowManager::activateWindow(int index)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    // Deactivate current
    if (m_activeIndex >= 0 && m_activeIndex < m_windows.count()) {
        m_windows[m_activeIndex].isActive = false;
        QModelIndex modelIndex = createIndex(m_activeIndex, 0);
        emit dataChanged(modelIndex, modelIndex, {IsActiveRole});
    }
    
    // Activate new
    m_windows[index].isActive = true;
    m_windows[index].zOrder = m_nextZOrder++;
    m_activeIndex = index;
    
    // Send activated state to toplevel
    if (m_windows[index].toplevel) {
        QList<QWaylandXdgToplevel::State> states;
        states << QWaylandXdgToplevel::ActivatedState;
        if (m_windows[index].isMaximized) {
            states << QWaylandXdgToplevel::MaximizedState;
        }
        if (m_windows[index].isFullscreen) {
            states << QWaylandXdgToplevel::FullscreenState;
        }
        m_windows[index].toplevel->sendConfigure(m_windows[index].size, states);
    }
    
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {IsActiveRole, ZOrderRole});
    emit activeWindowChanged();
    emit windowActivated(index);
}

void WindowManager::closeWindow(int index)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    if (m_windows[index].toplevel) {
        m_windows[index].toplevel->sendClose();
    }
}

void WindowManager::minimizeWindow(int index)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    m_windows[index].isMinimized = true;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {IsMinimizedRole});
}

void WindowManager::maximizeWindow(int index)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    m_windows[index].isMaximized = true;
    m_windows[index].isMinimized = false;
    
    // Get screen size
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect geom = screen->availableGeometry();
        m_windows[index].position = geom.topLeft();
        m_windows[index].size = geom.size();
        
        // Send maximized state
        if (m_windows[index].toplevel) {
            QList<QWaylandXdgToplevel::State> states;
            states << QWaylandXdgToplevel::MaximizedState;
            if (m_windows[index].isActive) {
                states << QWaylandXdgToplevel::ActivatedState;
            }
            m_windows[index].toplevel->sendConfigure(m_windows[index].size, states);
        }
    }
    
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {IsMaximizedRole, IsMinimizedRole, PositionXRole, PositionYRole, WidthRole, HeightRole});
}

void WindowManager::restoreWindow(int index)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    m_windows[index].isMinimized = false;
    m_windows[index].isMaximized = false;
    m_windows[index].isFullscreen = false;
    
    // Restore to default size
    m_windows[index].size = QSize(800, 600);
    m_windows[index].position = QPoint(100 + index * 30, 100 + index * 30);
    
    if (m_windows[index].toplevel) {
        QList<QWaylandXdgToplevel::State> states;
        if (m_windows[index].isActive) {
            states << QWaylandXdgToplevel::ActivatedState;
        }
        m_windows[index].toplevel->sendConfigure(m_windows[index].size, states);
    }
    
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {IsMinimizedRole, IsMaximizedRole, IsFullscreenRole, PositionXRole, PositionYRole, WidthRole, HeightRole});
}

void WindowManager::moveWindow(int index, int x, int y)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    m_windows[index].position = QPoint(x, y);
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {PositionXRole, PositionYRole});
}

void WindowManager::resizeWindow(int index, int width, int height)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    m_windows[index].size = QSize(width, height);
    
    if (m_windows[index].toplevel) {
        QList<QWaylandXdgToplevel::State> states;
        if (m_windows[index].isActive) {
            states << QWaylandXdgToplevel::ActivatedState;
        }
        if (m_windows[index].isMaximized) {
            states << QWaylandXdgToplevel::MaximizedState;
        }
        m_windows[index].toplevel->sendConfigure(m_windows[index].size, states);
    }
    
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {WidthRole, HeightRole});
}

void WindowManager::raiseWindow(int index)
{
    if (index < 0 || index >= m_windows.count()) {
        return;
    }
    
    m_windows[index].zOrder = m_nextZOrder++;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {ZOrderRole});
}

void WindowManager::onToplevelDestroyed()
{
    QWaylandXdgToplevel *toplevel = qobject_cast<QWaylandXdgToplevel*>(sender());
    if (toplevel) {
        removeWindow(toplevel);
    }
}

void WindowManager::onTitleChanged()
{
    QWaylandXdgToplevel *toplevel = qobject_cast<QWaylandXdgToplevel*>(sender());
    if (!toplevel) {
        return;
    }
    
    int idx = findWindow(toplevel);
    if (idx >= 0) {
        QModelIndex modelIndex = createIndex(idx, 0);
        emit dataChanged(modelIndex, modelIndex, {TitleRole});
    }
}

void WindowManager::onActivatedChanged()
{
    QWaylandXdgToplevel *toplevel = qobject_cast<QWaylandXdgToplevel*>(sender());
    if (!toplevel) {
        return;
    }
    
    int idx = findWindow(toplevel);
    if (idx >= 0 && toplevel->activated()) {
        activateWindow(idx);
    }
}

void WindowManager::updateZOrder()
{
    // Sort by zOrder and reassign
    for (int i = 0; i < m_windows.count(); ++i) {
        m_windows[i].zOrder = i;
    }
    
    emit dataChanged(createIndex(0, 0), createIndex(m_windows.count() - 1, 0), {ZOrderRole});
}

int WindowManager::findWindow(QWaylandXdgToplevel *toplevel) const
{
    for (int i = 0; i < m_windows.count(); ++i) {
        if (m_windows[i].toplevel == toplevel) {
            return i;
        }
    }
    return -1;
}

} // namespace NakumiOS
