/*
 * FileManager - File system model with navigation for NakumiFiles
 */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QFileInfo>
#include <QQmlEngine>

struct FileEntry {
    QString name;
    QString path;
    QString size;
    QString modified;
    QString icon;
    bool isDir;
};

class FileManager : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY pathChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        SizeRole,
        ModifiedRole,
        IconRole,
        IsDirRole,
    };

    explicit FileManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentPath() const { return m_currentPath; }
    void setCurrentPath(const QString &path);
    bool canGoBack() const { return !m_history.isEmpty(); }

    Q_INVOKABLE void open(int index);
    Q_INVOKABLE void goUp();
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goHome();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void deleteFile(int index);
    Q_INVOKABLE void createDirectory(const QString &name);

signals:
    void pathChanged();
    void historyChanged();
    void fileOpened(const QString &path);
    void errorOccurred(const QString &message);

private:
    void loadDirectory(const QString &path);
    static QString formatSize(qint64 bytes);

    QString m_currentPath;
    QVector<FileEntry> m_entries;
    QStringList m_history;
};

#endif // FILEMANAGER_H
