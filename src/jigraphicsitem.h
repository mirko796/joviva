#ifndef JIGRAPHICSITEM_H
#define JIGRAPHICSITEM_H
#include <QGraphicsObject>
#include <QFont>
#include "jicommon.h"
class JIGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
    JSONKEY JK_X = "x";
    JSONKEY JK_Y = "y";
    JSONKEY JK_WIDTH = "width";
    JSONKEY JK_HEIGHT = "height";
    JSONKEY JK_ROTATION = "rotation";
    JSONKEY JK_TRANSPARENT = "transparent";
    JSONKEY JK_ZVALUE = "z";
    JSONKEY JK_TYPE = "type";
    /**
     *  due to problem with transformOriginPoint we are storing
     *  both position of the item and position of the rect
     *  There must be a better way to avoid this redundancy but
     *  I failed to find it within limited time
     **/
    JSONKEY JK_POSX = "posx";
    JSONKEY JK_POSY = "posy";

    enum class ControlPoint
    {
        Invalid=0,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        TopMiddle,
        BottomMiddle,
        LeftMiddle,
        RightMiddle,
        Rotate,
        Move
    };

    static constexpr int MIN_ITEM_SIZE_PIX = 10;
    static const QSet<JIGraphicsItem::ControlPoint>& controlPoints();
    JIGraphicsItem(QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    // alignControlPoint is important when m_preserveAspectRatio is true
    void    setRect(const QRectF& rect, const ControlPoint alignControlPoint=ControlPoint::Invalid);
    QRectF rect() const;
    virtual QSizeF aspectRatio() const = 0;
    virtual void render(QPainter* painter) = 0;

    bool drawRotateControl() const;
    void setDrawRotateControl(bool newDrawRotateControl);

//    double backgroundThreshold() const;
//    void setBackgroundThreshold(double newBackgroundThreshold);

    bool transparentBackground() const;
    void setTransparentBackground(bool newTransparentBackground);

    virtual QJsonObject asJson() const;
    virtual bool fromJson(const QJsonObject& obj);

    void    setSortOrder(int newSortOrder);
    int     sortOrder() const;
signals:
    void    rotationChangedByUser(double newRotation);
    void    itemChanged();
private slots:
    void    onRotationChanged();
protected:
    void    makeOriginPointCentered();
    void    emitItemChanged();
    void    setRectDirect(const QRectF& rect);
    virtual void  transparentBackgroundChangedEvent() {};
    double controlPointSize() const;
    double rotateControlOffset() const;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    QRectF controlPointRect(const ControlPoint point) const;
    QCursor cursorShape(const ControlPoint point) const;
    ControlPoint controlPointAt(const QPointF& pos) const;
    // zValue might change when item is selected but this value is always the "intended" zValue
    int m_sortOrder;
    QRectF m_rect;
    QRectF m_boundingRect;
    double m_controlPointSizeFactor;
    double m_rotateControlOffsetFactor;
    ControlPoint m_captureControlPoint = ControlPoint::Invalid;
    QPointF m_captureMousePosLocal;
    QPointF m_captureMousePosScene;
    QPointF m_captureItemPosScene;
    QRectF  m_captureItemRect;
    double  m_captureRotation;
    double  m_backgroundThreshold = 0;
    bool    m_drawRotateControl = true;
    bool    m_preserveAspectRatio = true;
    bool    m_transparentBackground = true;
};

QDebug operator<<(QDebug dbg, const JIGraphicsItem::ControlPoint &point);
// define qhash operator for ControlPoint
uint qHash(const JIGraphicsItem::ControlPoint &point, uint seed = 0);
#endif // JIGRAPHICSITEM_H
