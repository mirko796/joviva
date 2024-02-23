#include "slgraphicsitem.h"
#include <QPainter>
#include <QDebug>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#ifdef Q_OS_WIN
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include <cmath>

uint qHash(const SLGraphicsItem::ControlPoint &point, uint seed)
{
    return qHash(int(point),seed);
}
// DONE: fix rotate
// DONE: fix resize when rotated
// TODO: right click resets rotation
// TODO: preserve aspect ratio when resize
const QSet<SLGraphicsItem::ControlPoint>& SLGraphicsItem::controlPoints()
{
    static QSet<SLGraphicsItem::ControlPoint> ret;
    if (ret.isEmpty()) {
        ret.insert(ControlPoint::TopLeft);
        ret.insert(ControlPoint::TopRight);
        ret.insert(ControlPoint::BottomLeft);
        ret.insert(ControlPoint::BottomRight);
        ret.insert(ControlPoint::TopMiddle);
        ret.insert(ControlPoint::BottomMiddle);
        ret.insert(ControlPoint::LeftMiddle);
        ret.insert(ControlPoint::RightMiddle);
        ret.insert(ControlPoint::Rotate);
        ret.insert(ControlPoint::Move);
    }
    return ret;
}

SLGraphicsItem::SLGraphicsItem(QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_controlPointSizeFactor(8),
    m_rotateControlOffsetFactor(16)
{
    // make selectable
    setFlag(QGraphicsItem::ItemIsSelectable);
    // accept hover events
    setAcceptHoverEvents(true);

    connect(this, &QGraphicsObject::rotationChanged,
            this, &SLGraphicsItem::onRotationChanged);
}

void SLGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    render(painter);
    if (option->state & QStyle::State_Selected) {
        // draw control points
        painter->setPen(QPen(Qt::white,1));
        // iterate over all control points
        foreach(const auto cp, controlPoints())
        {
            if (!m_drawRotateControl && cp==ControlPoint::Rotate) {
                continue;
            }
            if (cp!=ControlPoint::Move)
            {                
                const QRectF r(controlPointRect(cp));
                painter->fillRect(r,Qt::black);
                painter->drawRect(r);
            }
        }
    }
#if 0
    QRectF r2(0,0,4,4);
    r2.moveCenter(transformOriginPoint());
    painter->fillRect(r2,Qt::blue);
#endif
}

void SLGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    //set cursors depending on pos
    const ControlPoint cp = controlPointAt(event->pos());
    setCursor( cursorShape(cp) );
}

void SLGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    const ControlPoint cp = controlPointAt(event->pos());
    qDebug()<<"Mouse press"<<int(cp)<<rect()<<event->pos();
    m_captureControlPoint = cp;
    m_captureMousePosLocal = event->pos();
    m_captureMousePosScene = event->scenePos();
    m_captureItemPosScene = scenePos();
    m_captureRotation = rotation();
    m_captureItemRect = rect();
    if (m_captureControlPoint!=ControlPoint::Invalid) {
        m_preserveAspectRatio = false;
        setRect(rect());
    }
    event->accept();
    QGraphicsItem::mousePressEvent(event);
}

/**
 * Make sure that transform origin point is in center
 * and that the position is adjusted so that the item
 * is in the same place as before
 */
void SLGraphicsItem::makeOriginPointCentered()
{
    const auto before = mapToParent(m_rect.center());
    setTransformOriginPoint(m_rect.center());
    const auto after = mapToParent(m_rect.center());
    setPos(pos()+before-after);
}


void SLGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    const ControlPoint cp = controlPointAt(event->pos());
    qDebug()<<"Mouse release"<<int(cp);
    const auto prev = m_captureControlPoint;
    m_captureControlPoint = ControlPoint::Invalid;
    event->accept();
    if (prev!=ControlPoint::Invalid) {
        setRect(rect());
    }
    makeOriginPointCentered();
    QGraphicsItem::mouseReleaseEvent(event);
}

void SLGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }
    if (m_captureControlPoint == ControlPoint::Invalid) {
        return;
    }
    if (event->modifiers().testFlag(Qt::ShiftModifier) && (m_captureControlPoint != ControlPoint::Move)) {
        m_preserveAspectRatio = true;
    } else {
        m_preserveAspectRatio = false;
    }
//    qDebug()<<"Move"<<m_captureControlPoint;
    if (m_captureControlPoint == ControlPoint::Move) {
        const auto delta = event->scenePos() - m_captureMousePosScene;
//        setPos(m_captureItemPosScene+delta);
        QRectF r(m_captureItemRect);
        r.moveCenter(r.center()+delta);
        setTransformOriginPoint(r.center());
        setRect(r);
    } else if (m_captureControlPoint == ControlPoint::Rotate) {
        const QPointF origin = m_captureItemRect.center();
        setTransformOriginPoint(origin);
        const auto delta = event->scenePos() - origin;
        double angle = -atan(delta.x()/delta.y())*180/M_PI;
        if (delta.y()>0) {
            angle += 180;
        }
        angle = SL::normalizedAngle(angle);
        setRotation(angle);
        qDebug()<<"Emit rotation changed angle:"<<angle;
        emit rotationChangedByUser(angle);
    } else {
        const auto delta = event->pos() - m_captureMousePosLocal;
        QRectF r(m_captureItemRect);
        switch(m_captureControlPoint) {
        case ControlPoint::BottomMiddle:
            r.adjust(0,0,0,delta.y());
            break;
        case ControlPoint::TopMiddle:
            r.adjust(0,delta.y(),0,0);
            break;
        case ControlPoint::LeftMiddle:
            r.adjust(delta.x(),0,0,0);
            break;
        case ControlPoint::RightMiddle:
            r.adjust(0,0,delta.x(),0);
            break;
        case ControlPoint::TopLeft:
            r.adjust(delta.x(),delta.y(),0,0);
            break;
        case ControlPoint::TopRight:
            r.adjust(0,delta.y(),delta.x(),0);
            break;
        case ControlPoint::BottomLeft:
            r.adjust(delta.x(),0,0,delta.y());
            break;
        case ControlPoint::BottomRight:
            r.adjust(0,0,delta.x(),delta.y());
            break;
        default:
                break;
        }
        if (r.width()<MIN_ITEM_SIZE_PIX) {
            r.setWidth(MIN_ITEM_SIZE_PIX);
        }
        if (r.height()<MIN_ITEM_SIZE_PIX) {
            r.setHeight(MIN_ITEM_SIZE_PIX);
        }

        setRect(r,m_captureControlPoint);
        update();
    }
}

QRectF SLGraphicsItem::controlPointRect(const ControlPoint point) const
{
    const QRectF rect = this->rect();
    if (point == ControlPoint::Move) {
        return rect;
    }
    QPoint center;
    switch (point) {
    case ControlPoint::TopLeft:
        center = rect.topLeft().toPoint();
        break;
    case ControlPoint::TopRight:
        center = rect.topRight().toPoint();
        break;
    case ControlPoint::BottomLeft:
        center = rect.bottomLeft().toPoint();
        break;
    case ControlPoint::BottomRight:
        center = rect.bottomRight().toPoint();
        break;
    case ControlPoint::TopMiddle:
        center = rect.topLeft().toPoint() + QPoint(rect.width()/2,0);
        break;
    case ControlPoint::BottomMiddle:
        center = rect.bottomLeft().toPoint() + QPoint(rect.width()/2,0);
        break;
    case ControlPoint::LeftMiddle:
        center = rect.topLeft().toPoint() + QPoint(0,rect.height()/2);
        break;
    case ControlPoint::RightMiddle:
        center = rect.topRight().toPoint() + QPoint(0,rect.height()/2);
        break;
    case ControlPoint::Rotate:
        center = rect.topLeft().toPoint() + QPoint(rect.width()/2,-rotateControlOffset());
        break;
    case ControlPoint::Invalid:
    case ControlPoint::Move:
        break;
    }
    const double cps = controlPointSize();
    return QRectF(center.x() - cps/2, center.y() - cps/2, cps, cps);
}

QCursor SLGraphicsItem::cursorShape(const ControlPoint point) const
{
    QCursor ret(Qt::ArrowCursor);
    switch(point) {
    case ControlPoint::TopLeft:
    case ControlPoint::BottomRight:
        ret = QCursor(Qt::SizeFDiagCursor);
        break;
    case ControlPoint::BottomLeft:
    case ControlPoint::TopRight:
        ret = QCursor(Qt::SizeBDiagCursor);
        break;
    case ControlPoint::TopMiddle:
    case ControlPoint::BottomMiddle:
        ret = QCursor(Qt::SizeVerCursor);
        break;
    case ControlPoint::LeftMiddle:
    case ControlPoint::RightMiddle:
        ret = QCursor(Qt::SizeHorCursor);
        break;
    case ControlPoint::Rotate:
        ret = QCursor(QPixmap(":/rotate-icon.png").scaledToHeight(16,Qt::SmoothTransformation));
        break;
    case ControlPoint::Move:
        ret = QCursor(Qt::DragMoveCursor);
        break;
    case ControlPoint::Invalid:
        break;
    }
    return ret;
}

SLGraphicsItem::ControlPoint SLGraphicsItem::controlPointAt(const QPointF &pos) const
{
    const auto cp = controlPoints();
    foreach(const auto point, cp) {
        if (controlPointRect(point).contains(pos)) {
            if (!m_drawRotateControl && point==ControlPoint::Rotate) {
                break;
            }
            return point;
        }
    }
    return ControlPoint::Invalid;
}

bool SLGraphicsItem::transparentBackground() const
{
    return m_transparentBackground;
}

void SLGraphicsItem::setTransparentBackground(bool newTransparentBackground)
{
    m_transparentBackground = newTransparentBackground;
    transparentBackgroundChangedEvent();
    emitItemChanged();
}

QJsonObject SLGraphicsItem::asJson() const
{
    QJsonObject ret;
    ret[JK_X] = m_rect.x();
    ret[JK_Y] = m_rect.y();
    ret[JK_WIDTH] = m_rect.width();
    ret[JK_HEIGHT] = m_rect.height();
    ret[JK_ROTATION] = rotation();
    ret[JK_TRANSPARENT] = m_transparentBackground;
    ret[JK_ZVALUE] = sortOrder();
    ret[JK_TYPE] = type();
    ret[JK_POSX] = pos().x();
    ret[JK_POSY] = pos().y();
    return ret;
}

bool SLGraphicsItem::fromJson(const QJsonObject &obj)
{
    static QList<QString> requiredKeys({
        JK_X,
        JK_Y,
        JK_WIDTH,
        JK_HEIGHT,
        JK_ROTATION,
        JK_TRANSPARENT,
        JK_ZVALUE,
        JK_TYPE
    });
    const int typeVal = obj[JK_TYPE].toInt();
    if (typeVal!=type()) {
        qWarning()<<"Invalid type:"<<typeVal<<"Expected:"<<type();
        return false;
    }
    foreach(const auto key, requiredKeys) {
        if (!obj.contains(key)) {
            qWarning()<<"Missing key:"<<key<<" Keys:"<<obj.keys();
            return false;
        }
    }
    const QRectF rect(
        obj[JK_X].toDouble(),
        obj[JK_Y].toDouble(),
        obj[JK_WIDTH].toDouble(),
        obj[JK_HEIGHT].toDouble()
        );
    const double rotation = SL::normalizedAngle(obj[JK_ROTATION].toDouble());
    setRotation(rotation);
    setTransparentBackground(obj[JK_TRANSPARENT].toBool());
    setSortOrder(obj[JK_ZVALUE].toInt());
    setRectDirect(rect);
    qDebug()<<"Loaded from rect:"<<rect;
    setPos(obj[JK_POSX].toDouble(),obj[JK_POSY].toDouble());
    return true;
}

void SLGraphicsItem::setSortOrder(int newSortOrder)
{
    if (m_sortOrder != newSortOrder) {
        m_sortOrder = newSortOrder;
        setZValue(newSortOrder);
        emitItemChanged();
    }
}

int SLGraphicsItem::sortOrder() const
{
    return m_sortOrder;
}

void SLGraphicsItem::onRotationChanged()
{
    emitItemChanged();
}

void SLGraphicsItem::emitItemChanged()
{
    qDebug()<<"Item changed, emiting signal...";
    emit itemChanged();
}


#if 0
double SLGraphicsItem::backgroundThreshold() const
{
    return m_backgroundThreshold;
}

void SLGraphicsItem::setBackgroundThreshold(double newBackgroundThreshold)
{
    m_backgroundThreshold = newBackgroundThreshold;
    if (m_backgroundThreshold<0) {
        m_backgroundThreshold = 0;
    }
    if (m_backgroundThreshold>1) {
        m_backgroundThreshold = 1;
    }
    if (m_backgroundThreshold==0) {
        m_pixmap.fill(Qt::white);
        QPainter p(&m_pixmap);
        p.drawPixmap(m_pixmap.rect(),m_originalPixmap);
    } else {
        QImage img = m_originalPixmap.toImage();
        // convert to grayscale
        img = img.convertToFormat(QImage::Format_ARGB32);
        // preserve only pixels with gray > threshold
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                const QRgb pixel = img.pixel(x,y);
                const int gray = qGray(pixel);
                if (gray>m_backgroundThreshold*255) {
                    img.setPixel(x,y,qRgba(qRed(pixel),qGreen(pixel),qBlue(pixel),0));
                }
            }
        }
        // convert to alpha
        m_pixmap = QPixmap::fromImage(img);

    }

}
#endif
bool SLGraphicsItem::drawRotateControl() const
{
    return m_drawRotateControl;
}

void SLGraphicsItem::setDrawRotateControl(bool newDrawRotateControl)
{
    m_drawRotateControl = newDrawRotateControl;
}

double SLGraphicsItem::controlPointSize() const
{
    double ret = m_controlPointSizeFactor;
    // view scale
    if (scene()) {
        ret /= scene()->views().first()->transform().m11();
    }
    return ret;
}

double SLGraphicsItem::rotateControlOffset() const
{
    double ret = m_rotateControlOffsetFactor;
    // view scale
    if (scene()) {
        ret /= scene()->views().first()->transform().m11();
    }
    return ret;
}

QVariant SLGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change==ItemSelectedChange) {
        setTransformOriginPoint(rect().center());
    }
    return QGraphicsItem::itemChange(change, value);
}

QRectF SLGraphicsItem::boundingRect() const
{
    return m_boundingRect;
}

void SLGraphicsItem::setRectDirect(const QRectF &rect)
{
    prepareGeometryChange();
    m_rect = rect;
    const double halfSize = controlPointSize()/2;
    const double margin = (m_captureControlPoint==ControlPoint::Invalid)?0:50;
    const double rotOffset = (m_drawRotateControl)?rotateControlOffset():0;
    m_boundingRect = m_rect.adjusted(
        -margin-halfSize,
        -margin-rotOffset- halfSize,
        margin+halfSize,
        margin+halfSize);
    qDebug()<<"Item rect:"<<m_rect<<m_boundingRect;
}

void SLGraphicsItem::setRect(const QRectF &inputRect, const ControlPoint alignControlPoint)
{
    qDebug()<<"Set rect:"<<inputRect<<alignControlPoint;
    const QSizeF ar = this->aspectRatio();
    //    setTransformOriginPoint(m_rect.center());
    QRectF rect(inputRect);
    if (m_preserveAspectRatio)
    {
        QSizeF newSize;
        switch(alignControlPoint) {
        case ControlPoint::BottomMiddle:
        case ControlPoint::TopMiddle:
            if (m_captureItemRect.height()<rect.height())
                newSize = ar.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding);
            else
                newSize = ar.scaled(rect.size().toSize(),Qt::KeepAspectRatio);
            break;
        case ControlPoint::LeftMiddle:
        case ControlPoint::RightMiddle:
            if (m_captureItemRect.width()<rect.width())
                newSize = ar.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding);
            else
                newSize = ar.scaled(rect.size().toSize(),Qt::KeepAspectRatio);
            break;
        default:
            newSize = ar.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding);
            break;
        }

        qDebug()<<"New size:"<<newSize<<"Rect size:"<<rect.size()<<ar;
        qDebug()<<"Capture size:"<<m_captureItemRect.size();
        QRectF newRect(QPointF(0,0),newSize);
        qDebug()<<newRect<<rect;
        newRect.moveCenter(rect.center());
        switch(alignControlPoint) {
        case ControlPoint::TopMiddle:
            newRect.moveBottom(rect.bottom());
            break;
        case ControlPoint::BottomMiddle:
            newRect.moveTop(rect.top());
            break;
        case ControlPoint::RightMiddle:
            newRect.moveLeft(rect.left());
            break;
        case ControlPoint::LeftMiddle:
            newRect.moveRight(rect.right());
            break;
        case ControlPoint::BottomRight:
            newRect.moveTopLeft(rect.topLeft());
            break;
        case ControlPoint::BottomLeft:
            newRect.moveTopRight(rect.topRight());
            break;
        case ControlPoint::TopRight:
            newRect.moveBottomLeft(rect.bottomLeft());
            break;
        case ControlPoint::TopLeft:
            newRect.moveBottomRight(rect.bottomRight());
            break;
        default:
            break;
        }
        rect = newRect;
    }
    setRectDirect(rect);
    emitItemChanged();
}

QRectF SLGraphicsItem::rect() const
{
    return m_rect;
}

QDebug operator<<(QDebug dbg, const SLGraphicsItem::ControlPoint &point) {
    dbg<<int(point);
    return dbg;
}
