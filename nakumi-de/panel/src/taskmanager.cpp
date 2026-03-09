/*
 * TaskManager implementation
 *
 * In a full implementation, this would use the wlr-foreign-toplevel-management
 * Wayland protocol to track open windows. For now, we provide a basic
 * list model that can be populated.
 */

#include "taskmanager.h"

TaskManager::TaskManager(QObject *parent)
    : QAbstractListModel(parent) {
}

int TaskManager::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_tasks.count();
}

QVariant TaskManager::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_tasks.count())
        return {};

    const auto &task = m_tasks.at(index.row());
    switch (role) {
    case TitleRole:
        return task.title;
    case AppIdRole:
        return task.appId;
    case ActiveRole:
        return task.active;
    default:
        return {};
    }
}

QHash<int, QByteArray> TaskManager::roleNames() const {
    return {
        {TitleRole, "title"},
        {AppIdRole, "appId"},
        {ActiveRole, "active"},
    };
}

void TaskManager::activate(int index) {
    if (index < 0 || index >= m_tasks.count()) return;

    /* Deactivate all, activate the selected one */
    for (int i = 0; i < m_tasks.count(); ++i) {
        m_tasks[i].active = (i == index);
    }
    emit dataChanged(this->index(0), this->index(m_tasks.count() - 1),
                     {ActiveRole});
}

void TaskManager::close(int index) {
    if (index < 0 || index >= m_tasks.count()) return;
    beginRemoveRows(QModelIndex(), index, index);
    m_tasks.removeAt(index);
    endRemoveRows();
}

void TaskManager::refresh() {
    /* Placeholder: In full implementation, query the compositor
     * via wlr-foreign-toplevel-management protocol */
}
