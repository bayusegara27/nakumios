/**
 * NakumiOS File Manager Controller
 * 
 * Backend for the file manager providing directory listing,
 * file operations, and bookmarks management.
 */

#ifndef NAKUMIOS_FILEMANAGERCONTROLLER_H
#define NAKUMIOS_FILEMANAGERCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QFileSystemModel>
#include <QVariantList>
#include <QUrl>
#include <QStringList>

namespace NakumiOS {

class FileManagerController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
    Q_PROPERTY(QVariantList files READ files NOTIFY filesChanged)
    Q_PROPERTY(QVariantList bookmarks READ bookmarks NOTIFY bookmarksChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY historyChanged)
    Q_PROPERTY(QString homeDirectory READ homeDirectory CONSTANT)
    Q_PROPERTY(int viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(bool showHiddenFiles READ showHiddenFiles WRITE setShowHiddenFiles NOTIFY showHiddenFilesChanged)

public:
    enum ViewMode {
        IconView = 0,
        ListView = 1
    };
    Q_ENUM(ViewMode)
    
    explicit FileManagerController(QObject *parent = nullptr);
    ~FileManagerController() override;
    
    static FileManagerController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    
    QString currentPath() const;
    void setCurrentPath(const QString &path);
    QVariantList files() const;
    QVariantList bookmarks() const;
    
    bool canGoBack() const;
    bool canGoForward() const;
    QString homeDirectory() const;
    
    int viewMode() const;
    void setViewMode(int mode);
    bool showHiddenFiles() const;
    void setShowHiddenFiles(bool show);
    
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goForward();
    Q_INVOKABLE void goUp();
    Q_INVOKABLE void goHome();
    Q_INVOKABLE void refresh();
    
    Q_INVOKABLE void openItem(const QString &path);
    Q_INVOKABLE void createFolder(const QString &name);
    Q_INVOKABLE void createFile(const QString &name);
    Q_INVOKABLE void deleteItem(const QString &path);
    Q_INVOKABLE void renameItem(const QString &oldPath, const QString &newName);
    Q_INVOKABLE void copyItem(const QString &path);
    Q_INVOKABLE void cutItem(const QString &path);
    Q_INVOKABLE void paste();
    
    Q_INVOKABLE void addBookmark(const QString &path);
    Q_INVOKABLE void removeBookmark(const QString &path);
    
    Q_INVOKABLE QString formatFileSize(qint64 bytes) const;
    Q_INVOKABLE QString getFileIcon(const QString &path) const;

signals:
    void currentPathChanged();
    void filesChanged();
    void bookmarksChanged();
    void historyChanged();
    void viewModeChanged();
    void showHiddenFilesChanged();
    void errorOccurred(const QString &error);
    void fileOpened(const QString &path);

private:
    void loadDirectory(const QString &path);
    void loadBookmarks();
    void saveBookmarks();
    void addToHistory(const QString &path);
    
    QString m_currentPath;
    QStringList m_history;
    int m_historyIndex;
    QVariantList m_files;
    QStringList m_bookmarks;
    int m_viewMode;
    bool m_showHiddenFiles;
    QString m_clipboardPath;
    bool m_clipboardIsCut;
};

} // namespace NakumiOS

#endif // NAKUMIOS_FILEMANAGERCONTROLLER_H
