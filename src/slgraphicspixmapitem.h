#ifndef SLGRAPHICSPIXMAPITEM_H
#define SLGRAPHICSPIXMAPITEM_H

#include "slgraphicsitem.h"
#include "slcommon.h"
#include "slimageprovider.h"
class SLGraphicsPixmapItem : public SLGraphicsItem
{
    Q_OBJECT
public:
    static constexpr char const* const JK_PIXMAPID = "pixmapId";

    enum { Type = UserType + 1 };
    SLGraphicsPixmapItem(const SLImageProvider* provider);

    void setPixmapId(const SLImageProvider::PixmapId& pixmapId);
    SLImageProvider::PixmapId pixmapId() const { return m_pixmapId; }

    QSizeF aspectRatio() const override;
    void render(QPainter* painter) override;

    QPixmap pixmap() const;

    QJsonObject asJson() const override;
    bool    fromJson(const QJsonObject& obj) override;
    int type() const override { return Type; }
protected:
    void transparentBackgroundChangedEvent() override;
private:
    const SLImageProvider* m_provider = nullptr;
    SLImageProvider::PixmapId m_pixmapId;
};

#endif // SLGRAPHICSPIXMAPITEM_H
