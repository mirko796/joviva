#include "jigraphicspixmapitem.h"
#include <QPainter>

JIGraphicsPixmapItem::JIGraphicsPixmapItem(const JIImageProvider *provider) :
    JIGraphicsItem(),
    m_provider(provider)
{    
    m_transparentBackground = false;
}

void JIGraphicsPixmapItem::setPixmapId(const JIImageProvider::PixmapId &pixmapId)
{
    m_pixmapId = pixmapId;
    update();
}

QPixmap JIGraphicsPixmapItem::pixmap() const
{
    const QPixmap* pixmap = m_provider->pixmap(m_pixmapId, transparentBackground());
    if (pixmap == nullptr)
        return QPixmap();
    return *pixmap;
}

QJsonObject JIGraphicsPixmapItem::asJson() const
{
    QJsonObject obj = JIGraphicsItem::asJson();
    obj[JK_PIXMAPID] = m_pixmapId;
    return obj;
}

bool JIGraphicsPixmapItem::fromJson(const QJsonObject &obj)
{
    if (!JIGraphicsItem::fromJson(obj))
        return false;
    if (!obj.contains(JK_PIXMAPID))
        return false;
    setPixmapId(obj[JK_PIXMAPID].toString());
    return true;
}

void JIGraphicsPixmapItem::transparentBackgroundChangedEvent()
{
    update();
}

QSizeF JIGraphicsPixmapItem::aspectRatio() const
{
    return pixmap().size();
}

void JIGraphicsPixmapItem::render(QPainter *painter)
{
    const auto pix = pixmap();
    if (m_mirrored) {
        painter->save();
        painter->translate(m_rect.topRight());
        auto transform = painter->transform();
        transform.scale(-1, 1);
        painter->setTransform(transform);
        painter->drawPixmap(QRectF(QPoint(0,0),m_rect.size()), pix, pix.rect());
        painter->restore();
    } else {
        painter->drawPixmap(rect(), pix, pix.rect());
    }
}
