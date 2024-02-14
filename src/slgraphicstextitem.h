#ifndef SLGRAPHICSTEXTITEM_H
#define SLGRAPHICSTEXTITEM_H
#include "slgraphicsitem.h"
#include "slcommon.h"
class SLGraphicsTextItem : public SLGraphicsItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 2 };
    static constexpr char const* const JK_TEXTPARAMS = "textparams";

    SLGraphicsTextItem();

    void    setTextParams(const SL::TextParams& textParams);
    SL::TextParams textParams() const;

    QSizeF aspectRatio() const override;
    void render(QPainter* painter) override;

    QJsonObject asJson() const override;
    bool fromJson(const QJsonObject &obj) override;

    int type() const override { return Type; }

protected:
    void refreshPath();
    void transparentBackgroundChangedEvent() override;
private:
    SL::TextParams m_textParams;

    QPainterPath m_path;

};

#endif // SLGRAPHICSTEXTITEM_H
