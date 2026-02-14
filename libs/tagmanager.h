#ifndef TAGMANAGER_H
#define TAGMANAGER_H

#include <QObject>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

class TagManager : public QObject {
    Q_OBJECT

public:
    static TagManager& instance();

    QStringList getTags() const;
    void addTag(const QString &tag);
    void removeTag(const QString &tag);
    void renameTag(const QString &oldTag, const QString &newTag);
    void saveTags();

signals:
    void tagAdded(const QString &tag);
    void tagRemoved(const QString &tag);
    void tagRenamed(const QString &oldTag, const QString &newTag);

private:
    TagManager(QObject *parent = nullptr);
    ~TagManager();

    void loadTags();

    QStringList tags;
    QString savePath;
};

#endif // TAGMANAGER_H
