#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QObject>
#include <QIcon>
#include <QString>
#include <QPixmapCache>
#include <QMutex>

class ImageProvider : public QObject {
    Q_OBJECT

public:
    static ImageProvider& instance();
    
    // Returns the cached icon instantly if available, otherwise returns a null/placeholder icon
    // and begins loading it asynchronously.
    QIcon getIcon(const QString &thumbnailPath);

signals:
    // Emitted when an image finishes loading in the background
    void imageLoaded(const QString &thumbnailPath);

private:
    explicit ImageProvider(QObject *parent = nullptr);
    ~ImageProvider();

    QIcon placeholderIcon;
    QList<QString> pendingLoads;
    QMutex mutex;
};

#endif // IMAGEPROVIDER_H
