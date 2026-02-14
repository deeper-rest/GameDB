#include "gamemanager.h"
#include <QDebug>

GameManager& GameManager::instance() {
    static GameManager _instance;
    return _instance;
}

GameManager::GameManager(QObject *parent) : QObject(parent) {
    // Determine save path
    // For simplicity, let's save in the executable directory or AppData.
    // AppData is better for permissions.
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    this->savePath = dir.filePath("games.json");
    
    loadGames();
}

GameManager::~GameManager() {
    saveGames();
}

void GameManager::addGame(const GameItem &item) {
    // Check for duplicates by path
    for (const auto &g : library) {
        if (g.filePath == item.filePath) {
            // Already exists
            return;
        }
    }
    
    this->library.append(item);
    emit gameAdded(item);
    emit libraryUpdated();
    saveGames();
}

void GameManager::removeGame(int index) {
    if (index >= 0 && index < this->library.size()) {
        QString path = this->library[index].filePath;
        this->library.removeAt(index);
        emit gameRemoved(path);
        emit libraryUpdated();
        saveGames();
    }
}

void GameManager::removeGameByPath(const QString &path) {
    for (int i = 0; i < this->library.size(); ++i) {
        if (this->library[i].filePath == path) {
            removeGame(i);
            return;
        }
    }
}

QList<GameItem> GameManager::getGames() const {
    return this->library;
}

void GameManager::saveGames() {
    QJsonArray array;
    for (const auto &game : this->library) {
        array.append(gameToJson(game));
    }
    
    QJsonObject root;
    root["games"] = array;
    
    QFile file(this->savePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
    } else {
        qDebug() << "Failed to save games to" << this->savePath;
    }
}

void GameManager::loadGames() {
    QFile file(this->savePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return; // File doesn't exist yet
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    
    this->library.clear();
    if (root.contains("games") && root["games"].isArray()) {
        QJsonArray array = root["games"].toArray();
        for (const auto &val : array) {
            this->library.append(jsonToGame(val.toObject()));
        }
    }
    
    emit libraryUpdated();
}

QJsonObject GameManager::gameToJson(const GameItem &item) {
    QJsonObject obj;
    obj["cleanName"] = item.cleanName;
    obj["originalName"] = item.originalName;
    obj["filePath"] = item.filePath;
    obj["type"] = static_cast<int>(item.type);
    
    obj["koreanSupport"] = item.koreanSupport;
    obj["folderName"] = item.folderName;
    
    QJsonArray tagsArr;
    for (const QString &t : item.tags) tagsArr.append(t);
    obj["tags"] = tagsArr;
    
    return obj;
}

GameItem GameManager::jsonToGame(const QJsonObject &obj) {
    GameItem item;
    item.cleanName = obj["cleanName"].toString();
    item.originalName = obj["originalName"].toString();
    item.filePath = obj["filePath"].toString();
    item.type = static_cast<GameType>(obj["type"].toInt());
    
    item.koreanSupport = obj["koreanSupport"].toBool();
    item.folderName = obj["folderName"].toString();
    
    if (obj.contains("tags") && obj["tags"].isArray()) {
        QJsonArray tagsArr = obj["tags"].toArray();
        for (const auto &val : tagsArr) {
            item.tags.append(val.toString());
        }
    }
    
    return item;
}
