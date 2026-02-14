#include "gamescanner.h"
#include <QDebug>

GameScanner::GameScanner(QObject *parent) : QObject(parent) {

}

void GameScanner::scanDirectory(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) return;

    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::NoSymLinks);

    for (const QFileInfo &info : list) {
        if (info.isDir()) {
            // Check if this directory itself is a game
            // For now, if it contains an executable or is a leaf node, we might consider it.
            // But let's stick to the user's request: "find games within subdirectories".
            // Implementation detail: Deep scan.
            
            // Simple heuristic: If it looks like a game folder, add it.
            // For now, let's treat every folder as a potential game container and recurse,
            // BUT also check if the folder ITSELF is the game.
            // A simple strategy: Treat all leaf folders or folders with archives/exes as games?
            // "compressed files AND sub directory games"
            
            // Current Strategy:
            // 1. Recurse.
            // 2. Also process the folder itself if it matches criteria (TODO: finer criteria).
            // For the first version, let's just recurse and find COMPRESSED files, 
            // and treating immediate sub-folders of the Root as games? 
            // Or just List all folders?
            
            // Non-recursive scan.
            // Just add the folder itself if it's a directory.
            processEntry(info);

        } else {
            processEntry(info);
        }
    }
    emit scanFinished();
}

void GameScanner::processEntry(const QFileInfo &info) {
    GameType type = determineType(info);
    if (type == GameType::Unknown) return;

    GameItem item;
    item.filePath = info.absoluteFilePath();
    item.originalName = info.fileName();
    item.type = type;
    item.cleanName = cleanGameName(item.originalName);

    emit gameFound(item);
}

GameType GameScanner::determineType(const QFileInfo &info) {
    if (info.isDir()) return GameType::Folder;
    
    QString suffix = info.suffix().toLower();
    if (suffix == "zip") return GameType::Zip;
    if (suffix == "7z") return GameType::SevenZip;
    if (suffix == "rar") return GameType::Rar;
    if (suffix == "iso") return GameType::Iso;
    
    return GameType::Unknown;
}

QString GameScanner::cleanGameName(const QString &name) {
    QString clean = name;
    
    // Remove extension if file
    int lastDot = clean.lastIndexOf('.');
    if (lastDot > 0) clean = clean.left(lastDot);

    // Common scene release patterns removal
    // e.g. (USA), [JP], v1.0, etc.
    
    // Remove brackets [] and () and content within
    static QRegularExpression re("\\[.*?\\]|\\(.*?\\)");
    clean.replace(re, "");
    
    // Remove underscores
    clean = clean.replace("_", " ");
    
    // Trim extra spaces
    clean = clean.simplified();
    
    return clean;
}
