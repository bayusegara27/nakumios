/**
 * NakumiOS Window Manager
 * 
 * Manages window positioning, z-ordering, focus, and animations.
 * Provides window list model for QML bindings.
 */

#ifndef NAKUMIOS_WINDOWMANAGER_H
#define NAKUMIOS_WINDOWMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QtWaylandCompositor/QWaylandXdgToplevel>
#include <QtWaylandCompositor/QWaylandXdgSurface>
#include <QList>
#include <QPoint>
#include <QSize>

namespace NakumiOS {

struct WindowInfo {
    QWaylandXdgToplevel *toplevel;
    QWaylandXdgSurface *xdgSurface;
    QPoint position;
    QSize size;
    bool isActive;
    bool isMinimized;
    bool isMaximized;
    bool isFullscreen;
    int zOrder;
};

class WindowManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int activeWindowIndex READ activeWindowIndex NOTIFY activeWindowChanged)

public:
    enum Roles {
        ToplevelRole = Qt::UserRole + 1,
        XdgSurfaceRole,
        TitleRole,
        AppIdRole,
        PositionXRole,
        PositionYRole,
        WidthRole,
        HeightRole,
        IsActiveRole,
        IsMinimizedRole,
        IsMaximizedRole,
        IsFullscreenRole,
        ZOrderRole
    };
    
    explicit WindowManager(QObject *parent = nullptr);
    ~WindowManager() override;
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    int count() const;
    int activeWindowIndex() const;
    
    void addWindow(QWaylandXdgToplevel *toplevel, QWaylandXdgSurface *xdgSurface);
    void removeWindow(QWaylandXdgToplevel *toplevel);
    
    Q_INVOKABLE void activateWindow(int index);
    Q_INVOKABLE void closeWindow(int index);
    Q_INVOKABLE void minimizeWindow(int index);
    Q_INVOKABLE void maximizeWindow(int index);
    Q_INVOKABLE void restoreWindow(int index);
    Q_INVOKABLE void moveWindow(int index, int x, int y);
    Q_INVOKABLE void resizeWindow(int index, int width, int height);
    Q_INVOKABLE void raiseWindow(int index);

signals:
    void countChanged();
    void activeWindowChanged();
    void windowAdded(int index);
    void windowRemoved(int index);
    void windowActivated(int index);

private slots:
    void onToplevelDestroyed();
    void onTitleChanged();
    void onActivatedChanged();

private:
    int findWindow(QWaylandXdgToplevel *toplevel) const;
    
    QList<WindowInfo> m_windows;
    int m_activeIndex;
    int m_nextZOrder;
};

} // namespace NakumiOS

#endif // NAKUMIOS_WINDOWMANAGER_H
