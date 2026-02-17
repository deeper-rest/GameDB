#include "gamemanager.h"
#include "tagmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
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
    
    // Connect TagManager signals for consistency
    connect(&TagManager::instance(), &TagManager::tagRenamed, this, &GameManager::onTagRenamed);
    connect(&TagManager::instance(), &TagManager::tagRemoved, this, &GameManager::onTagRemoved);
    
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

void GameManager::updateGame(const GameItem &item) {
    for (int i = 0; i < this->library.size(); ++i) {
        if (this->library[i].filePath == item.filePath) {
            this->library[i] = item;
            emit libraryUpdated();
            saveGames();
            return;
        }
    }
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

GameItem GameManager::getGameByPath(const QString &path) const {
    for (const auto &game : this->library) {
        if (game.filePath == path) {
            return game;
        }
    }
    return GameItem(); // Return empty item if not found
}

void GameManager::updateLastPlayed(const QString &path) {
    for (int i = 0; i < library.size(); ++i) {
        if (library[i].filePath == path) {
            library[i].lastPlayed = QDateTime::currentDateTime();
            saveGames();
            emit libraryUpdated(); // Or emit a specific signal if needed
            return;
        }
    }
}

QJsonObject GameManager::gameToJson(const GameItem &item) {
    QJsonObject obj;
    obj["originalName"] = item.originalName;
    obj["cleanName"] = item.cleanName;
    obj["filePath"] = item.filePath;
    obj["type"] = static_cast<int>(item.type);
    obj["koreanSupport"] = item.koreanSupport;
    obj["folderName"] = item.folderName;
    
    QJsonArray tagArray;
    for (const QString &tag : item.tags) {
        tagArray.append(tag);
    }
    obj["tags"] = tagArray;
    
    obj["source"] = item.source;
    obj["gameCode"] = item.gameCode;
    obj["thumbnailPath"] = item.thumbnailPath;
    obj["exePath"] = item.exePath;
    
    if (item.lastPlayed.isValid()) {
        obj["lastPlayed"] = item.lastPlayed.toString(Qt::ISODate);
    }

    return obj;
}

GameItem GameManager::jsonToGame(const QJsonObject &obj) {
    GameItem item;
    item.originalName = obj["originalName"].toString();
    item.cleanName = obj["cleanName"].toString();
    item.filePath = obj["filePath"].toString();
    item.type = static_cast<GameType>(obj["type"].toInt());
    item.koreanSupport = obj["koreanSupport"].toBool();
    item.folderName = obj["folderName"].toString();
    
    QJsonArray tagArray = obj["tags"].toArray();
    for (const QJsonValue &val : tagArray) {
        item.tags.append(val.toString());
    }
    
    item.source = obj["source"].toString();
    item.gameCode = obj["gameCode"].toString();
    item.thumbnailPath = obj["thumbnailPath"].toString();
    item.exePath = obj["exePath"].toString();
    
    if (obj.contains("lastPlayed")) {
        item.lastPlayed = QDateTime::fromString(obj["lastPlayed"].toString(), Qt::ISODate);
    }

    return item;
}

void GameManager::onTagRenamed(const QString &oldTag, const QString &newTag) {
    bool changed = false;
    for (GameItem &game : this->library) {
        if (game.tags.contains(oldTag)) {
            int idx = game.tags.indexOf(oldTag);
            game.tags.replace(idx, newTag);
            changed = true;
        }
    }
    
    if (changed) {
        saveGames();
        emit libraryUpdated();
    }
}

void GameManager::onTagRemoved(const QString &tag) {
    bool changed = false;
    for (GameItem &game : this->library) {
        if (game.tags.contains(tag)) {
            game.tags.removeAll(tag);
            changed = true;
        }
    }
    
    if (changed) {
        saveGames();
        emit libraryUpdated();
    }
}
