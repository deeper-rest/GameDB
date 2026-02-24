#include "gamelibrarymodel.h"
#include "gamemanager.h"
#include "imageprovider.h"
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>

GameLibraryModel::GameLibraryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // Connect to GameManager to receive updates
    connect(&GameManager::instance(), &GameManager::libraryUpdated, this, &GameLibraryModel::onLibraryUpdated);
    libraryRef = &GameManager::instance().getGames();
    
    // Connect to ImageProvider to repaint cells when images load via background thread
    connect(&ImageProvider::instance(), &ImageProvider::imageLoaded, this, [this](const QString &path) {
        if (!libraryRef) return;
        for (int i = 0; i < libraryRef->size(); ++i) {
            if (libraryRef->at(i).thumbnailPath == path) {
                QModelIndex idx = index(i, 0);
                emit dataChanged(idx, idx, {Qt::DecorationRole});
            }
        }
    });
}

void GameLibraryModel::onLibraryUpdated()
{
    // For now, the safest and simplest approach is a full reset since the underlying list can change entirely.
    beginResetModel();
    libraryRef = &GameManager::instance().getGames();
    endResetModel();
}

int GameLibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !libraryRef)
        return 0;
    return libraryRef->size();
}

int GameLibraryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 7; // Name, Folder Name, Type, Korean, Tags, Last Played, Path
}

QVariant GameLibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= libraryRef->size())
        return QVariant();

    const GameItem &game = libraryRef->at(index.row());

    // Provide custom roles for filtering and card view rendering
    if (role == GameRoles::GameItemRole) {
        return QVariant::fromValue(game);
    } else if (role == GameRoles::FilePathRole) {
        return game.filePath;
    } else if (role == GameRoles::CleanNameRole) {
        return game.cleanName;
    } else if (role == GameRoles::FolderNameRole) {
        return game.folderName;
    } else if (role == GameRoles::TypeRole) {
        return QVariant::fromValue(static_cast<int>(game.type));
    } else if (role == GameRoles::TagsRole) {
        return game.tags;
    } else if (role == GameRoles::LastPlayedRole) {
        return game.lastPlayed;
    }

    // Default Display Roles for TableView
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return game.cleanName;
            case 1: return game.folderName;
            case 2:
                switch (game.type) {
                    case GameType::Folder: return "Folder";
                    case GameType::Zip: return "Zip";
                    case GameType::SevenZip: return "7z";
                    case GameType::Rar: return "Rar";
                    case GameType::Iso: return "Iso";
                    default: return "Unknown";
                }
            case 3: return game.koreanSupport ? "Yes" : "No";
            case 4: return game.tags.join(", ");
            case 5: return game.lastPlayed.isValid() ? game.lastPlayed.toString("yyyy-MM-dd HH:mm") : "Never";
            case 6: return game.filePath;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 3) { // Korean Support 
            return game.koreanSupport ? QVariant(QColor(Qt::darkGreen)) : QVariant(QColor(Qt::transparent));
        }
    } else if (role == Qt::DecorationRole && index.column() == 0) {
        return ImageProvider::instance().getIcon(game.thumbnailPath);
    }

    return QVariant();
}

QVariant GameLibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "Name";
            case 1: return "Folder Name";
            case 2: return "Type";
            case 3: return "Korean";
            case 4: return "Tags";
            case 5: return "Last Played";
            case 6: return "Path";
        }
    }
    return QVariant();
}

GameItem GameLibraryModel::getGame(int row) const
{
    if (row >= 0 && row < libraryRef->size()) {
        return libraryRef->at(row);
    }
    return GameItem();
}
