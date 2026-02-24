#ifndef GAMELIBRARYMODEL_H
#define GAMELIBRARYMODEL_H

#include <QAbstractTableModel>
#include "gamedata.h"

// Define custom roles for our model to use in Card View and Proxy Filter
enum GameRoles {
    GameItemRole = Qt::UserRole + 1,
    FilePathRole,
    CleanNameRole,
    FolderNameRole,
    TypeRole,
    TagsRole,
    KoreanSupportRole,
    LastPlayedRole
};

class GameLibraryModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit GameLibraryModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Custom functions to interact with the underlying data
    void refreshData();
    GameItem getGame(int row) const;

public slots:
    void onLibraryUpdated();

private:
    const QList<GameItem> *libraryRef;
};

#endif // GAMELIBRARYMODEL_H
