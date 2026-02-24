#include "imageprovider.h"
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QPixmap>
#include <QColor>
#include <QPainter>

ImageProvider& ImageProvider::instance() {
    static ImageProvider _instance;
    return _instance;
}

ImageProvider::ImageProvider(QObject *parent) : QObject(parent) {
    // Generate a default placeholder
    QPixmap placeholder(320, 200);
    placeholder.fill(QColor(200, 200, 200)); 
    placeholderIcon = QIcon(placeholder);
    
    // Increase cache size if needed (default usually 10MB, set to 50MB for thumbnails)
    QPixmapCache::setCacheLimit(50 * 1024);
}

ImageProvider::~ImageProvider() {}

QIcon ImageProvider::getIcon(const QString &thumbnailPath) {
    if (thumbnailPath.isEmpty()) {
        return placeholderIcon;
    }
    
    QPixmap pixmap;
    if (QPixmapCache::find(thumbnailPath, &pixmap)) {
        return QIcon(pixmap);
    }
    
    // Check if it's already being loaded to avoid redundant background tasks
    QMutexLocker locker(&mutex);
    if (!pendingLoads.contains(thumbnailPath)) {
        pendingLoads.append(thumbnailPath);
        
        // Asynchronously load the image
        QFutureWatcher<QPixmap> *watcher = new QFutureWatcher<QPixmap>(this);
        connect(watcher, &QFutureWatcher<QPixmap>::finished, this, [this, watcher, thumbnailPath]() {
            QPixmap result = watcher->result();
            if (!result.isNull()) {
                QPixmapCache::insert(thumbnailPath, result);
            }
            
            QMutexLocker lock(&mutex);
            pendingLoads.removeAll(thumbnailPath);
            lock.unlock(); // Unlock before emitting to prevent deadlock if emit hooks back
            
            emit imageLoaded(thumbnailPath);
            watcher->deleteLater();
        });
        
        // Execute background read
        QFuture<QPixmap> future = QtConcurrent::run([thumbnailPath]() {
            QPixmap img(thumbnailPath);
            return img;
        });
        
        watcher->setFuture(future);
    }
    
    return placeholderIcon;
}
