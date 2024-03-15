#include "jigraphicstextitem.h"
#include <QPainter>
#include <QDebug>
#include <QFontMetrics>
JIGraphicsTextItem::JIGraphicsTextItem() :
    JIGraphicsItem()
{
    m_preserveAspectRatio = false;
}

JI::TextParams JIGraphicsTextItem::textParams() const
{
    return m_textParams;
}

QSizeF JIGraphicsTextItem::aspectRatio() const
{
    return m_path.boundingRect().size();
}

void JIGraphicsTextItem::render(QPainter *painter)
{
    painter->save();
    const auto &path = m_path;
    auto br = path.boundingRect();
    painter->setPen(Qt::black);
    const double scaleX = rect().width()/br.width();
    const double scaleY = rect().height()/br.height();
    // set render hints to high quality antialiasing
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    if (m_transparentBackground==false) {
        painter->fillRect(m_rect, Qt::white);
    }
    const int mirrorOffset = m_mirrored ? -m_rect.width() : 0;
    painter->translate(m_rect.topLeft()-QPointF(mirrorOffset+br.x()*scaleX,br.y()*scaleY));
    painter->scale(scaleX, scaleY);
    if (m_mirrored) {
        auto transform = painter->transform();
        transform.scale(-1,1);
        painter->setTransform(transform);
    }
    if (m_textParams.hollow) {
        painter->drawPath(path);
    } else {
        painter->fillPath(path, Qt::black);
    }
//    painter->drawPixmap(rect(), m_pixmap, m_pixmap.rect());
    painter->restore();
}

void JIGraphicsTextItem::transparentBackgroundChangedEvent()
{
    refreshPath();
}

void JIGraphicsTextItem::refreshPath()
{
    if (m_rect.isEmpty()) return;
    const QString text = m_textParams.text.isEmpty() ? QString("          ") : m_textParams.text;

    QPainterPath path;
    qDebug()<<"Render text, font:"<<m_textParams.font<<m_textParams.text;
    const QStringList lines = text.split("\n");
    QList<QRectF> boundingRects;
    QFontMetrics fm(m_textParams.font);
    int maxWidth=0;
    foreach(const QString& line, lines)
    {
        const auto br = fm.boundingRect(line);
        boundingRects.append(br);
        maxWidth = qMax(maxWidth, br.width());
    }
    for (int i = 0; i < lines.size(); ++i)
    {
        const QString& line = lines.at(i);
        const QRectF br = boundingRects.at(i);
        int x = 0;
        if (m_textParams.alignment & Qt::AlignHCenter) {
            x = (maxWidth - br.width())/2;
        } else if (m_textParams.alignment & Qt::AlignRight) {
            x = maxWidth - br.width();
        }
        path.addText(x, br.height()*i, m_textParams.font, line);
    }
    //    path.addText(0,0,m_textParams.font,m_textParams.text);
    const auto br = path.boundingRect().isEmpty() ? QRectF(0,0,m_rect.width(),m_rect.height()) : path.boundingRect();
    qDebug()<<"Bounding rect:"<<br<<rect();

    m_path = path;

    const double scaleX = m_rect.width()/br.width();
    QSizeF newSize = br.size()*scaleX;
    QRectF newRect(m_rect.topLeft(),newSize);
    qDebug()<<"New rect:"<<newRect;
    setRect(newRect);
    update();

#if 0
    const int hollowOffset = 10;
    const int margin = hollowOffset; //let's leave few px around the text to avoid cropping (when having hollow text)
    const QString text = m_textParams.text.isEmpty() ? QString("          ") : m_textParams.text;
    const auto bbox = QFontMetrics(m_textParams.font).boundingRect(QRect(0,0,1000,1000), Qt::AlignCenter, text).adjusted(0,0,margin*2, margin*2);
    QPixmap pix(bbox.size());
    if (m_transparentBackground)
        pix.fill(Qt::transparent);
    else
        pix.fill(Qt::white);
    QPainter painter(&pix);
    // set all painter parameters to high quality
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    painter.setFont(m_textParams.font);
    painter.setPen(m_textParams.color);
    painter.setBrush(m_textParams.color);
    QRect pixRect = pix.rect();
    pixRect.adjust(margin,margin,-margin,-margin);
    if (m_textParams.hollow)
    {
        painter.drawText(pixRect.adjusted(0,hollowOffset,0,0), m_textParams.alignment, text);
        painter.drawText(pixRect.adjusted(0,-hollowOffset,0,0), m_textParams.alignment, text);
        painter.drawText(pixRect.adjusted(hollowOffset,0,0,0), m_textParams.alignment, text);
        painter.drawText(pixRect.adjusted(-hollowOffset,0,0,0), m_textParams.alignment, text);

        painter.drawText(pixRect.adjusted(hollowOffset,hollowOffset,0,0), m_textParams.alignment, text);
        painter.drawText(pixRect.adjusted(hollowOffset,-hollowOffset,0,0), m_textParams.alignment, text);
        painter.drawText(pixRect.adjusted(-hollowOffset,hollowOffset,0,0), m_textParams.alignment, text);
        painter.drawText(pixRect.adjusted(-hollowOffset,-hollowOffset,0,0), m_textParams.alignment, text);

        painter.setPen(Qt::white);
        painter.setBrush(Qt::white);
    }
    painter.drawText(pixRect, m_textParams.alignment, text);
    qDebug()<<"Pixmap size:"<<pix.size()<<bbox.size()<<m_textParams.text<<m_textParams.font<<m_textParams.color;
    const QRectF oldRect = rect();
    m_pixmap = pix;
    QRectF newRect(oldRect);
    const double k = pix.width()/double(pix.height());
    newRect.setHeight(newRect.width()/k);
    setRect(newRect);
    update();
#endif
}

void JIGraphicsTextItem::setTextParams(const JI::TextParams& textParams)
{
    m_textParams = textParams;
    refreshPath();
    makeOriginPointCentered();
    emitItemChanged();
}
QJsonObject JIGraphicsTextItem::asJson() const
{
    QJsonObject obj = JIGraphicsItem::asJson();
    obj[JK_TEXTPARAMS] = m_textParams.asJson();
    return obj;
}

bool JIGraphicsTextItem::fromJson(const QJsonObject &obj)
{
    if (!JIGraphicsItem::fromJson(obj))
        return false;
    const QRectF jsonRect(
        obj[JK_X].toDouble(),
        obj[JK_Y].toDouble(),
        obj[JK_WIDTH].toDouble(),
        obj[JK_HEIGHT].toDouble()
        );
    JI::TextParams params;
    if (!params.fromJson(obj[JK_TEXTPARAMS].toObject())) {
        return false;
    }
    setTextParams(params);
    // restore original rect since it could be modified by setTextParams and refresh pixmap
    setRectDirect(jsonRect);
    setPos(obj[JK_POSX].toDouble(),obj[JK_POSY].toDouble());
    return true;
}
