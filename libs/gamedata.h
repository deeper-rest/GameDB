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
    
    // Extended Metadata
    bool koreanSupport = false;
    QString folderName; // Folder name or File name without path
    QStringList tags;
    QString source; // e.g. "DLsite", "Steam"
    QString gameCode; // e.g. "RJ123456"
    
    // Thumbnail & Launch
    QString thumbnailPath;
    QString exePath; // Specific executable to run (for Folder type games)
};

#endif // GAMEDATA_H
