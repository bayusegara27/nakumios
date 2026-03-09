/*
 * AppModel - Application list model
 *
 * Reads .desktop files from /usr/share/applications/ and provides
 * them as a filterable list model for the launcher.
 */

#ifndef APPMODEL_H
#define APPMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QSortFilterProxyModel>

struct AppEntry {
    QString name;
    QString exec;
    QString icon;
    QString comment;
    QString categories;
};

class AppModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ExecRole,
        IconRole,
        CommentRole,
        CategoriesRole,
    };

    explicit AppModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void launch(int index);
    Q_INVOKABLE void refresh();

private:
    void loadDesktopFiles();
    QVector<AppEntry> m_apps;
};

class AppFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    explicit AppFilterModel(QObject *parent = nullptr);

    QString filterText() const { return m_filterText; }
    void setFilterText(const QString &text);

signals:
    void filterTextChanged();

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

private:
    QString m_filterText;
};

#endif // APPMODEL_H
