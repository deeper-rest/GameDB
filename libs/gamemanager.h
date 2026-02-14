#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include "gamedata.h"

class GameManager : public QObject {
    Q_OBJECT

public:
    static GameManager& instance();

    void addGame(const GameItem &item);
    void removeGame(int index);
    void removeGameByPath(const QString &path);
    QList<GameItem> getGames() const;
    GameItem getGameByPath(const QString &path) const;
    
    void saveGames();
    void loadGames();

public slots:
    void onTagRenamed(const QString &oldTag, const QString &newTag);
    void onTagRemoved(const QString &tag);

signals:
    void gameAdded(GameItem item);
    void gameRemoved(QString path);
    void libraryUpdated();

private:
    GameManager(QObject *parent = nullptr);
    ~GameManager();
    
    QList<GameItem> library;
    QString savePath;

    // Helper for JSON serialization
    QJsonObject gameToJson(const GameItem &item);
    GameItem jsonToGame(const QJsonObject &obj);
};

#endif // GAMEMANAGER_H
