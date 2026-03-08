/*
 * AppModel implementation
 */

#include "appmodel.h"

#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

AppModel::AppModel(QObject *parent)
    : QAbstractListModel(parent) {
    loadDesktopFiles();
}

void AppModel::loadDesktopFiles() {
    beginResetModel();
    m_apps.clear();

    QStringList dirs = {
        QStringLiteral("/usr/share/applications"),
        QStringLiteral("/usr/local/share/applications"),
        QDir::homePath() + QStringLiteral("/.local/share/applications"),
    };

    for (const auto &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        const auto entries = dir.entryInfoList(
            QStringList{QStringLiteral("*.desktop")}, QDir::Files);
        for (const auto &fi : entries) {
            QSettings desktop(fi.absoluteFilePath(), QSettings::IniFormat);
            desktop.beginGroup(QStringLiteral("Desktop Entry"));

            /* Skip hidden or NoDisplay entries */
            if (desktop.value(QStringLiteral("NoDisplay"), false).toBool())
                continue;
            if (desktop.value(QStringLiteral("Hidden"), false).toBool())
                continue;

            AppEntry entry;
            entry.name = desktop.value(QStringLiteral("Name")).toString();
            entry.exec = desktop.value(QStringLiteral("Exec")).toString();
            entry.icon = desktop.value(QStringLiteral("Icon")).toString();
            entry.comment = desktop.value(QStringLiteral("Comment")).toString();
            entry.categories = desktop.value(QStringLiteral("Categories")).toString();

            if (!entry.name.isEmpty() && !entry.exec.isEmpty()) {
                m_apps.append(entry);
            }

            desktop.endGroup();
        }
    }

    /* Sort alphabetically */
    std::sort(m_apps.begin(), m_apps.end(),
              [](const AppEntry &a, const AppEntry &b) {
                  return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
              });

    endResetModel();
}

int AppModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_apps.count();
}

QVariant AppModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_apps.count())
        return {};

    const auto &app = m_apps.at(index.row());
    switch (role) {
    case NameRole:       return app.name;
    case ExecRole:       return app.exec;
    case IconRole:       return app.icon;
    case CommentRole:    return app.comment;
    case CategoriesRole: return app.categories;
    default:             return {};
    }
}

QHash<int, QByteArray> AppModel::roleNames() const {
    return {
        {NameRole, "name"},
        {ExecRole, "exec"},
        {IconRole, "icon"},
        {CommentRole, "comment"},
        {CategoriesRole, "categories"},
    };
}

void AppModel::launch(int index) {
    if (index < 0 || index >= m_apps.count()) return;

    QString cmd = m_apps.at(index).exec;
    /* Remove field codes (%f, %F, %u, %U, etc.) */
    cmd.remove(QRegularExpression(QStringLiteral("%[fFuUdDnNickvm]")));
    cmd = cmd.trimmed();

    QProcess::startDetached(QStringLiteral("/bin/sh"),
                            QStringList{QStringLiteral("-c"), cmd});
}

void AppModel::refresh() {
    loadDesktopFiles();
}

/* ======================================================================
 * AppFilterModel
 * ====================================================================== */

AppFilterModel::AppFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void AppFilterModel::setFilterText(const QString &text) {
    if (m_filterText != text) {
        m_filterText = text;
        invalidateFilter();
        emit filterTextChanged();
    }
}

bool AppFilterModel::filterAcceptsRow(int sourceRow,
                                      const QModelIndex &sourceParent) const {
    if (m_filterText.isEmpty()) return true;

    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    QString name = idx.data(AppModel::NameRole).toString();
    QString comment = idx.data(AppModel::CommentRole).toString();

    return name.contains(m_filterText, Qt::CaseInsensitive) ||
           comment.contains(m_filterText, Qt::CaseInsensitive);
}
