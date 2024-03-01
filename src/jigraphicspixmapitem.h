#ifndef JIGRAPHICSPIXMAPITEM_H
#define JIGRAPHICSPIXMAPITEM_H

#include "jigraphicsitem.h"
#include "jicommon.h"
#include "jiimageprovider.h"
class JIGraphicsPixmapItem : public JIGraphicsItem
{
    Q_OBJECT
public:
    static constexpr char const* const JK_PIXMAPID = "pixmapId";

    enum { Type = UserType + 1 };
    JIGraphicsPixmapItem(const JIImageProvider* provider);
    
    void setPixmapId(const JIImageProvider::PixmapId& pixmapId);
    JIImageProvider::PixmapId pixmapId() const { return m_pixmapId; }

    QSizeF aspectRatio() const override;
    void render(QPainter* painter) override;

    QPixmap pixmap() const;

    QJsonObject asJson() const override;
    bool    fromJson(const QJsonObject& obj) override;
    int type() const override { return Type; }
protected:
    void transparentBackgroundChangedEvent() override;
private:
    const JIImageProvider* m_provider = nullptr;
    JIImageProvider::PixmapId m_pixmapId;
};

#endif // JIGRAPHICSPIXMAPITEM_H
