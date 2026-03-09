/*
 * FileManager implementation
 */

#include "filemanager.h"

#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QProcess>

FileManager::FileManager(QObject *parent)
    : QAbstractListModel(parent) {
    goHome();
}

void FileManager::loadDirectory(const QString &path) {
    beginResetModel();
    m_entries.clear();

    QDir dir(path);
    if (!dir.exists()) {
        endResetModel();
        emit errorOccurred(QStringLiteral("Directory does not exist: ") + path);
        return;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    const auto entries = dir.entryInfoList();
    for (const auto &fi : entries) {
        FileEntry entry;
        entry.name = fi.fileName();
        entry.path = fi.absoluteFilePath();
        entry.isDir = fi.isDir();
        entry.size = fi.isDir() ? QStringLiteral("--") : formatSize(fi.size());
        entry.modified = fi.lastModified().toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        entry.icon = fi.isDir() ? QStringLiteral("folder") : QStringLiteral("text-x-generic");
        m_entries.append(entry);
    }

    endResetModel();
}

QString FileManager::formatSize(qint64 bytes) {
    if (bytes < 1024) return QString::number(bytes) + QStringLiteral(" B");
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + QStringLiteral(" KB");
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + QStringLiteral(" MB");
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 1) + QStringLiteral(" GB");
}

int FileManager::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_entries.count();
}

QVariant FileManager::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_entries.count())
        return {};

    const auto &entry = m_entries.at(index.row());
    switch (role) {
    case NameRole:     return entry.name;
    case PathRole:     return entry.path;
    case SizeRole:     return entry.size;
    case ModifiedRole: return entry.modified;
    case IconRole:     return entry.icon;
    case IsDirRole:    return entry.isDir;
    default:           return {};
    }
}

QHash<int, QByteArray> FileManager::roleNames() const {
    return {
        {NameRole, "fileName"},
        {PathRole, "filePath"},
        {SizeRole, "fileSize"},
        {ModifiedRole, "fileModified"},
        {IconRole, "fileIcon"},
        {IsDirRole, "isDir"},
    };
}

void FileManager::setCurrentPath(const QString &path) {
    if (m_currentPath != path) {
        m_history.append(m_currentPath);
        m_currentPath = path;
        loadDirectory(path);
        emit pathChanged();
        emit historyChanged();
    }
}

void FileManager::open(int index) {
    if (index < 0 || index >= m_entries.count()) return;
    const auto &entry = m_entries.at(index);
    if (entry.isDir) {
        setCurrentPath(entry.path);
    } else {
        emit fileOpened(entry.path);
        QDesktopServices::openUrl(QUrl::fromLocalFile(entry.path));
    }
}

void FileManager::goUp() {
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        setCurrentPath(dir.absolutePath());
    }
}

void FileManager::goBack() {
    if (m_history.isEmpty()) return;
    QString prev = m_history.takeLast();
    m_currentPath = prev;
    loadDirectory(prev);
    emit pathChanged();
    emit historyChanged();
}

void FileManager::goHome() {
    setCurrentPath(QDir::homePath());
}

void FileManager::refresh() {
    loadDirectory(m_currentPath);
}

void FileManager::deleteFile(int index) {
    if (index < 0 || index >= m_entries.count()) return;
    const auto &entry = m_entries.at(index);
    QDir dir;
    bool ok = entry.isDir ? dir.rmdir(entry.path) : QFile::remove(entry.path);
    if (ok) {
        refresh();
    } else {
        emit errorOccurred(QStringLiteral("Failed to delete: ") + entry.name);
    }
}

void FileManager::createDirectory(const QString &name) {
    QDir dir(m_currentPath);
    if (dir.mkdir(name)) {
        refresh();
    } else {
        emit errorOccurred(QStringLiteral("Failed to create directory: ") + name);
    }
}
