#include "tagmanager.h"
#include <QDebug>

TagManager& TagManager::instance() {
    static TagManager _instance;
    return _instance;
}

TagManager::TagManager(QObject *parent) : QObject(parent) {
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataLocation);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    this->savePath = dir.filePath("tags.json");
    
    loadTags();
}

TagManager::~TagManager() {
    saveTags();
}

QStringList TagManager::getTags() const {
    return this->tags;
}

void TagManager::addTag(const QString &tag) {
    if (!this->tags.contains(tag)) {
        this->tags.append(tag);
        this->tags.sort();
        saveTags();
        emit tagAdded(tag);
    }
}

void TagManager::removeTag(const QString &tag) {
    if (this->tags.removeOne(tag)) {
        saveTags();
        emit tagRemoved(tag);
    }
}

void TagManager::renameTag(const QString &oldTag, const QString &newTag) {
    int index = this->tags.indexOf(oldTag);
    if (index != -1 && !this->tags.contains(newTag)) {
        this->tags[index] = newTag;
        this->tags.sort();
        saveTags();
        emit tagRenamed(oldTag, newTag);
    }
}

void TagManager::saveTags() {
    QJsonArray array;
    for (const QString &tag : this->tags) {
        array.append(tag);
    }
    
    QJsonDocument doc(array);
    QFile file(this->savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void TagManager::loadTags() {
    QFile file(this->savePath);
    if (!file.open(QIODevice::ReadOnly)) {
        // Default tags
        this->tags << "Action" << "Adventure" << "RPG" << "Simulation" << "Strategy" << "Sports" << "FPS" << "Puzzle";
        this->tags.sort();
        saveTags();
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        this->tags.clear();
        QJsonArray array = doc.array();
        for (const auto &val : array) {
            this->tags.append(val.toString());
        }
        this->tags.sort();
    }
}
