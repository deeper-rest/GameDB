#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <QString>

enum class GameType {
    Folder,
    Zip,
    SevenZip,
    Rar,
    Iso,
    Unknown
};

struct GameItem {
    QString originalName;
    QString cleanName;
    QString filePath;
    GameType type;
};

#endif // GAMEDATA_H
