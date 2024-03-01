#ifndef SLIMAGEPROVIDER_H
#define SLIMAGEPROVIDER_H
#include <QHash>
#include <QPixmap>
#include <QSet>
#include "jicommon.h"
class JIImageProvider
{
public:
    typedef QString PixmapId;
    JSONKEY JK_IMAGES = "images";

    JIImageProvider();
    ~JIImageProvider();

    PixmapId addPixmap(const QPixmap& pixmap);
    const QPixmap *pixmap(const PixmapId& id, const bool transparentBackground = false) const;
    QSet<PixmapId> pixmapIds() const;
    int count() const { return m_pixmaps.count(); }
    void clear();

    QJsonObject asJson() const;
    bool fromJson(const QJsonObject& obj);
private:
    QHash<QString, QPixmap*> m_pixmaps;
    mutable QHash<QString, QPixmap*> m_transparentPixmaps;
};

#endif // SLIMAGEPROVIDER_H
