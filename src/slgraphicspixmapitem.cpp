#include "slgraphicspixmapitem.h"
#include <QPainter>

SLGraphicsPixmapItem::SLGraphicsPixmapItem(const SLImageProvider *provider) :
    SLGraphicsItem(),
    m_provider(provider)
{    
    m_transparentBackground = false;
}

void SLGraphicsPixmapItem::setPixmapId(const SLImageProvider::PixmapId &pixmapId)
{
    m_pixmapId = pixmapId;
    update();
}

QPixmap SLGraphicsPixmapItem::pixmap() const
{
    const QPixmap* pixmap = m_provider->pixmap(m_pixmapId, transparentBackground());
    if (pixmap == nullptr)
        return QPixmap();
    return *pixmap;
}

QJsonObject SLGraphicsPixmapItem::asJson() const
{
    QJsonObject obj = SLGraphicsItem::asJson();
    obj[JK_PIXMAPID] = m_pixmapId;
    return obj;
}

bool SLGraphicsPixmapItem::fromJson(const QJsonObject &obj)
{
    if (!SLGraphicsItem::fromJson(obj))
        return false;
    if (!obj.contains(JK_PIXMAPID))
        return false;
    setPixmapId(obj[JK_PIXMAPID].toString());
    return true;
}

void SLGraphicsPixmapItem::transparentBackgroundChangedEvent()
{
    update();
}

QSizeF SLGraphicsPixmapItem::aspectRatio() const
{
    return pixmap().size();
}

void SLGraphicsPixmapItem::render(QPainter *painter)
{
    const auto pix = pixmap();
    painter->drawPixmap(rect(), pix, pix.rect());
}
