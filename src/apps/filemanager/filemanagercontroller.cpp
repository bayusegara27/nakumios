/**
 * NakumiOS File Manager Controller Implementation
 */

#include "filemanagercontroller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeDatabase>
#include <QDateTime>
#include <algorithm>

namespace NakumiOS {

FileManagerController::FileManagerController(QObject *parent)
    : QObject(parent)
    , m_historyIndex(-1)
    , m_viewMode(IconView)
    , m_showHiddenFiles(false)
    , m_clipboardIsCut(false)
{
    loadBookmarks();
    setCurrentPath(QDir::homePath());
}

FileManagerController::~FileManagerController()
{
    saveBookmarks();
}

FileManagerController *FileManagerController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(jsEngine)
    
    auto *controller = new FileManagerController();
    QQmlEngine::setObjectOwnership(controller, QQmlEngine::JavaScriptOwnership);
    
    return controller;
}

QString FileManagerController::currentPath() const
{
    return m_currentPath;
}

void FileManagerController::setCurrentPath(const QString &path)
{
    QString normalizedPath = QDir::cleanPath(path);
    
    QFileInfo info(normalizedPath);
    if (!info.exists() || !info.isDir()) {
        emit errorOccurred(QStringLiteral("Directory does not exist: ") + path);
        return;
    }
    
    if (m_currentPath != normalizedPath) {
        addToHistory(normalizedPath);
        m_currentPath = normalizedPath;
        loadDirectory(normalizedPath);
        emit currentPathChanged();
    }
}

QVariantList FileManagerController::files() const
{
    return m_files;
}

QVariantList FileManagerController::bookmarks() const
{
    QVariantList list;
    
    // Default system locations
    QVariantMap home;
    home[QStringLiteral("name")] = QStringLiteral("Home");
    home[QStringLiteral("path")] = QDir::homePath();
    home[QStringLiteral("icon")] = QStringLiteral("🏠");
    list.append(home);
    
    QVariantMap desktop;
    desktop[QStringLiteral("name")] = QStringLiteral("Desktop");
    desktop[QStringLiteral("path")] = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    desktop[QStringLiteral("icon")] = QStringLiteral("🖥️");
    list.append(desktop);
    
    QVariantMap documents;
    documents[QStringLiteral("name")] = QStringLiteral("Documents");
    documents[QStringLiteral("path")] = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    documents[QStringLiteral("icon")] = QStringLiteral("📄");
    list.append(documents);
    
    QVariantMap downloads;
    downloads[QStringLiteral("name")] = QStringLiteral("Downloads");
    downloads[QStringLiteral("path")] = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    downloads[QStringLiteral("icon")] = QStringLiteral("⬇️");
    list.append(downloads);
    
    QVariantMap music;
    music[QStringLiteral("name")] = QStringLiteral("Music");
    music[QStringLiteral("path")] = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    music[QStringLiteral("icon")] = QStringLiteral("🎵");
    list.append(music);
    
    QVariantMap pictures;
    pictures[QStringLiteral("name")] = QStringLiteral("Pictures");
    pictures[QStringLiteral("path")] = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    pictures[QStringLiteral("icon")] = QStringLiteral("🖼️");
    list.append(pictures);
    
    QVariantMap videos;
    videos[QStringLiteral("name")] = QStringLiteral("Videos");
    videos[QStringLiteral("path")] = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    videos[QStringLiteral("icon")] = QStringLiteral("🎬");
    list.append(videos);
    
    // Custom bookmarks
    for (const QString &path : m_bookmarks) {
        QFileInfo info(path);
        if (info.exists()) {
            QVariantMap bookmark;
            bookmark[QStringLiteral("name")] = info.fileName();
            bookmark[QStringLiteral("path")] = path;
            bookmark[QStringLiteral("icon")] = QStringLiteral("📁");
            bookmark[QStringLiteral("custom")] = true;
            list.append(bookmark);
        }
    }
    
    return list;
}

bool FileManagerController::canGoBack() const
{
    return m_historyIndex > 0;
}

bool FileManagerController::canGoForward() const
{
    return m_historyIndex < m_history.size() - 1;
}

QString FileManagerController::homeDirectory() const
{
    return QDir::homePath();
}

int FileManagerController::viewMode() const
{
    return m_viewMode;
}

void FileManagerController::setViewMode(int mode)
{
    if (m_viewMode != mode) {
        m_viewMode = mode;
        emit viewModeChanged();
    }
}

bool FileManagerController::showHiddenFiles() const
{
    return m_showHiddenFiles;
}

void FileManagerController::setShowHiddenFiles(bool show)
{
    if (m_showHiddenFiles != show) {
        m_showHiddenFiles = show;
        loadDirectory(m_currentPath);
        emit showHiddenFilesChanged();
    }
}

void FileManagerController::goBack()
{
    if (canGoBack()) {
        m_historyIndex--;
        m_currentPath = m_history[m_historyIndex];
        loadDirectory(m_currentPath);
        emit currentPathChanged();
        emit historyChanged();
    }
}

void FileManagerController::goForward()
{
    if (canGoForward()) {
        m_historyIndex++;
        m_currentPath = m_history[m_historyIndex];
        loadDirectory(m_currentPath);
        emit currentPathChanged();
        emit historyChanged();
    }
}

void FileManagerController::goUp()
{
    QDir dir(m_currentPath);
    if (dir.cdUp()) {
        setCurrentPath(dir.absolutePath());
    }
}

void FileManagerController::goHome()
{
    setCurrentPath(QDir::homePath());
}

void FileManagerController::refresh()
{
    loadDirectory(m_currentPath);
}

void FileManagerController::openItem(const QString &path)
{
    QFileInfo info(path);
    
    if (info.isDir()) {
        setCurrentPath(path);
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        emit fileOpened(path);
    }
}

void FileManagerController::createFolder(const QString &name)
{
    QDir dir(m_currentPath);
    if (!dir.mkdir(name)) {
        emit errorOccurred(QStringLiteral("Failed to create folder: ") + name);
    } else {
        refresh();
    }
}

void FileManagerController::createFile(const QString &name)
{
    QString filePath = m_currentPath + QDir::separator() + name;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QStringLiteral("Failed to create file: ") + name);
    } else {
        file.close();
        refresh();
    }
}

void FileManagerController::deleteItem(const QString &path)
{
    QFileInfo info(path);
    bool success = false;
    
    if (info.isDir()) {
        QDir dir(path);
        success = dir.removeRecursively();
    } else {
        success = QFile::remove(path);
    }
    
    if (!success) {
        emit errorOccurred(QStringLiteral("Failed to delete: ") + path);
    } else {
        refresh();
    }
}

void FileManagerController::renameItem(const QString &oldPath, const QString &newName)
{
    QFileInfo info(oldPath);
    QString newPath = info.dir().filePath(newName);
    
    if (!QFile::rename(oldPath, newPath)) {
        emit errorOccurred(QStringLiteral("Failed to rename: ") + oldPath);
    } else {
        refresh();
    }
}

void FileManagerController::copyItem(const QString &path)
{
    m_clipboardPath = path;
    m_clipboardIsCut = false;
}

void FileManagerController::cutItem(const QString &path)
{
    m_clipboardPath = path;
    m_clipboardIsCut = true;
}

bool FileManagerController::copyDirectoryRecursive(const QString &srcPath, const QString &destPath)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists()) {
        return false;
    }
    
    QDir destDir(destPath);
    if (!destDir.exists()) {
        destDir.mkpath(destPath);
    }
    
    // Copy files
    const QStringList files = srcDir.entryList(QDir::Files | QDir::Hidden);
    for (const QString &file : files) {
        QString srcFile = srcPath + QDir::separator() + file;
        QString destFile = destPath + QDir::separator() + file;
        if (!QFile::copy(srcFile, destFile)) {
            return false;
        }
    }
    
    // Recursively copy subdirectories
    const QStringList dirs = srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    for (const QString &dir : dirs) {
        QString srcSubDir = srcPath + QDir::separator() + dir;
        QString destSubDir = destPath + QDir::separator() + dir;
        if (!copyDirectoryRecursive(srcSubDir, destSubDir)) {
            return false;
        }
    }
    
    return true;
}

void FileManagerController::paste()
{
    if (m_clipboardPath.isEmpty()) {
        return;
    }
    
    QFileInfo srcInfo(m_clipboardPath);
    QString destPath = m_currentPath + QDir::separator() + srcInfo.fileName();
    
    // Check if destination already exists
    if (QFileInfo::exists(destPath)) {
        emit errorOccurred(QStringLiteral("Destination already exists: ") + srcInfo.fileName());
        return;
    }
    
    bool success = false;
    
    if (srcInfo.isDir()) {
        // Copy directory recursively
        success = copyDirectoryRecursive(m_clipboardPath, destPath);
    } else {
        success = QFile::copy(m_clipboardPath, destPath);
    }
    
    if (success && m_clipboardIsCut) {
        if (srcInfo.isDir()) {
            QDir(m_clipboardPath).removeRecursively();
        } else {
            QFile::remove(m_clipboardPath);
        }
        m_clipboardPath.clear();
    }
    
    if (!success) {
        emit errorOccurred(QStringLiteral("Failed to paste: ") + srcInfo.fileName());
    } else {
        refresh();
    }
}

void FileManagerController::addBookmark(const QString &path)
{
    if (!m_bookmarks.contains(path)) {
        m_bookmarks.append(path);
        saveBookmarks();
        emit bookmarksChanged();
    }
}

void FileManagerController::removeBookmark(const QString &path)
{
    if (m_bookmarks.removeAll(path) > 0) {
        saveBookmarks();
        emit bookmarksChanged();
    }
}

QString FileManagerController::formatFileSize(qint64 bytes) const
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QStringLiteral("%1 %2").arg(size, 0, 'f', 1).arg(QLatin1String(units[unitIndex]));
}

QString FileManagerController::getFileIcon(const QString &path) const
{
    QFileInfo info(path);
    
    if (info.isDir()) {
        return QStringLiteral("📁");
    }
    
    QString suffix = info.suffix().toLower();
    
    // Text/documents
    if (suffix == QLatin1String("txt") || suffix == QLatin1String("md") || suffix == QLatin1String("rtf")) {
        return QStringLiteral("📄");
    }
    if (suffix == QLatin1String("pdf")) {
        return QStringLiteral("📕");
    }
    if (suffix == QLatin1String("doc") || suffix == QLatin1String("docx") || suffix == QLatin1String("odt")) {
        return QStringLiteral("📝");
    }
    if (suffix == QLatin1String("xls") || suffix == QLatin1String("xlsx") || suffix == QLatin1String("ods")) {
        return QStringLiteral("📊");
    }
    
    // Images
    if (suffix == QLatin1String("jpg") || suffix == QLatin1String("jpeg") || suffix == QLatin1String("png") ||
        suffix == QLatin1String("gif") || suffix == QLatin1String("bmp") || suffix == QLatin1String("svg")) {
        return QStringLiteral("🖼️");
    }
    
    // Audio
    if (suffix == QLatin1String("mp3") || suffix == QLatin1String("wav") || suffix == QLatin1String("flac") ||
        suffix == QLatin1String("ogg") || suffix == QLatin1String("m4a")) {
        return QStringLiteral("🎵");
    }
    
    // Video
    if (suffix == QLatin1String("mp4") || suffix == QLatin1String("avi") || suffix == QLatin1String("mkv") ||
        suffix == QLatin1String("mov") || suffix == QLatin1String("webm")) {
        return QStringLiteral("🎬");
    }
    
    // Archives
    if (suffix == QLatin1String("zip") || suffix == QLatin1String("tar") || suffix == QLatin1String("gz") ||
        suffix == QLatin1String("7z") || suffix == QLatin1String("rar")) {
        return QStringLiteral("📦");
    }
    
    // Code
    if (suffix == QLatin1String("cpp") || suffix == QLatin1String("c") || suffix == QLatin1String("h") ||
        suffix == QLatin1String("py") || suffix == QLatin1String("js") || suffix == QLatin1String("ts") ||
        suffix == QLatin1String("java") || suffix == QLatin1String("rs") || suffix == QLatin1String("go")) {
        return QStringLiteral("💻");
    }
    
    // Executables
    if (info.isExecutable()) {
        return QStringLiteral("⚙️");
    }
    
    return QStringLiteral("📄");
}

void FileManagerController::loadDirectory(const QString &path)
{
    m_files.clear();
    
    QDir dir(path);
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot;
    if (m_showHiddenFiles) {
        filters |= QDir::Hidden;
    }
    
    const QFileInfoList entries = dir.entryInfoList(filters, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
    
    for (const QFileInfo &entry : entries) {
        QVariantMap file;
        file[QStringLiteral("name")] = entry.fileName();
        file[QStringLiteral("path")] = entry.absoluteFilePath();
        file[QStringLiteral("isDir")] = entry.isDir();
        file[QStringLiteral("isHidden")] = entry.isHidden();
        file[QStringLiteral("isSymLink")] = entry.isSymLink();
        file[QStringLiteral("size")] = entry.size();
        file[QStringLiteral("sizeFormatted")] = entry.isDir() ? QStringLiteral("--") : formatFileSize(entry.size());
        file[QStringLiteral("modified")] = entry.lastModified().toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        file[QStringLiteral("icon")] = getFileIcon(entry.absoluteFilePath());
        file[QStringLiteral("permissions")] = entry.isReadable() ? QStringLiteral("r") : QStringLiteral("-");
        
        m_files.append(file);
    }
    
    emit filesChanged();
}

void FileManagerController::loadBookmarks()
{
    QSettings settings(QStringLiteral("NakumiOS"), QStringLiteral("FileManager"));
    m_bookmarks = settings.value(QStringLiteral("bookmarks")).toStringList();
}

void FileManagerController::saveBookmarks()
{
    QSettings settings(QStringLiteral("NakumiOS"), QStringLiteral("FileManager"));
    settings.setValue(QStringLiteral("bookmarks"), m_bookmarks);
}

void FileManagerController::addToHistory(const QString &path)
{
    // Remove forward history when navigating to new path
    while (m_history.size() > m_historyIndex + 1) {
        m_history.removeLast();
    }
    
    m_history.append(path);
    m_historyIndex = m_history.size() - 1;
    
    // Limit history size
    while (m_history.size() > 50) {
        m_history.removeFirst();
        m_historyIndex--;
    }
    
    emit historyChanged();
}

} // namespace NakumiOS
