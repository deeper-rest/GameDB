#ifndef GAMESCANNER_H
#define GAMESCANNER_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QRegularExpression>
#include "gamedata.h"

class GameScanner : public QObject {
    Q_OBJECT

public:
    explicit GameScanner(QObject *parent = nullptr);

public slots:
    void scanDirectory(const QString &path);

signals:
    void gameFound(GameItem item);
    void scanFinished();

private:
    void processEntry(const QFileInfo &info);
    QString cleanGameName(const QString &name);
    GameType determineType(const QFileInfo &info);
};

#endif // GAMESCANNER_H
