#ifndef SLGRAPHICSVIEW_H
#define SLGRAPHICSVIEW_H

#include <QObject>
#include <QGraphicsView>
#include "slcommon.h"
#include "slgraphicsscene.h"
#include "slgraphicsitem.h"
#include "slimageprovider.h"
#include <QTimer>
class SLGraphicsView : public QGraphicsView
{
    Q_OBJECT

    typedef quint32 ItemId;
    enum ItemType {
        itInvalid,
        itPixmap,
        itText
    };

public:
    JSONKEY JK_ITEMS = "items";
    JSONKEY JK_PAPER_WIDTH = "paper_width";
    JSONKEY JK_PAPER_HEIGHT = "paper_height";
    JSONKEY JK_ORIENTATION = "orientation";
    JSONKEY JK_PROVIDER = "provider";

    enum JsonFlags {
        jfItemsOnly,
        jfItemsAndImages
    };
    SLGraphicsView(QWidget *parent = nullptr);
    ~SLGraphicsView();

    void    addPixmapItem(const QPixmap& pixmap);
    void    addTextItem(const SL::TextParams &textParams);
    QSize paperSize() const;
    void setPaperSize(const QSize &newPaperSize);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation newOrientation);

    SLGraphicsItem* selectedItem() const;

    int     itemsCount() const;

    QJsonObject asJson(const JsonFlags flags) const;
    bool fromJson(const QJsonObject& obj, const JsonFlags flags);
public slots:
    void    clearSelection();
    void    removeAllItems();
    void    removeItem(SLGraphicsItem* item);
signals:
    void    selectionChanged();
    void    itemRotationChangedByUser(SLGraphicsItem* item, double rotation);
    void    itemsChanged();
    void    orientationChanged();
private slots:
    void    onItemRotationChangedByUser(double rotation);
    void    onItemChanged();
    void    sendItemsChangedIfAny();
protected:
    void startItemsChangedTimer(const int msInterval);
    void addItemWithId(SLGraphicsItem* item, const ItemId id);
    void addItem(SLGraphicsItem* item);
    void resizeEvent(QResizeEvent *event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void updateSceneSize();
    bool eventFilter(QObject *obj, QEvent *event) override;
    ItemId m_lastItemId = 0;
    QHash<ItemId,SLGraphicsItem*> m_items;
    SLGraphicsScene m_scene;
    QSize   m_paperSize = QSize(210,297);
    Qt::Orientation m_orientation = Qt::Vertical;
    SLImageProvider m_provider;
    QSet<SLGraphicsItem*> m_changedItems;
    QTimer m_itemsChangedTimer;
    bool    m_mousePressed = false;

};

#endif // SLGRAPHICSVIEW_H
