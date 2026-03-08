/*
 * TaskManager - Taskbar model using wlr-foreign-toplevel-management
 *
 * Provides a list model of open windows for the taskbar.
 */

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QAbstractListModel>
#include <QQmlEngine>

struct TaskEntry {
    QString title;
    QString appId;
    bool active = false;
};

class TaskManager : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        AppIdRole,
        ActiveRole,
    };

    explicit TaskManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void activate(int index);
    Q_INVOKABLE void close(int index);
    Q_INVOKABLE void refresh();

private:
    QVector<TaskEntry> m_tasks;
};

#endif // TASKMANAGER_H
