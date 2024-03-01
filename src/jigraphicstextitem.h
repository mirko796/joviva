#ifndef JIGRAPHICSTEXTITEM_H
#define JIGRAPHICSTEXTITEM_H
#include "jigraphicsitem.h"
#include "jicommon.h"
class JIGraphicsTextItem : public JIGraphicsItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 2 };
    static constexpr char const* const JK_TEXTPARAMS = "textparams";

    JIGraphicsTextItem();

    void    setTextParams(const JI::TextParams& textParams);
    JI::TextParams textParams() const;

    QSizeF aspectRatio() const override;
    void render(QPainter* painter) override;

    QJsonObject asJson() const override;
    bool fromJson(const QJsonObject &obj) override;

    int type() const override { return Type; }

protected:
    void refreshPath();
    void transparentBackgroundChangedEvent() override;
private:
    JI::TextParams m_textParams;

    QPainterPath m_path;

};

#endif // JIGRAPHICSTEXTITEM_H
