#ifndef JIGRAPHICSVIEW_H
#define JIGRAPHICSVIEW_H

#include <QObject>
#include <QGraphicsView>
#include "jicommon.h"
#include "jigraphicsscene.h"
#include "jigraphicsitem.h"
#include "jiimageprovider.h"
#include <QTimer>
class JIGraphicsView : public QGraphicsView
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
    JSONKEY JK_PAPERFORMAT = "paper_format";

    enum JsonFlags {
        jfItemsOnly,
        jfItemsAndImages
    };
    JIGraphicsView(QWidget *parent = nullptr);
    ~JIGraphicsView();

    void    addPixmapItem(const QPixmap& pixmap);
    void    addTextItem(const JI::TextParams &textParams);
    QSize paperSize() const;
    void setPaperSize(const QSize &newPaperSize);
    QSize paperSizeWithOrientation() const;
    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation newOrientation);
    
    JIGraphicsItem* selectedItem() const;

    int     itemsCount() const;

    QJsonObject asJson(const JsonFlags flags) const;
    bool fromJson(const QJsonObject& obj, const JsonFlags flags);
    JI::DocumentSize documentSize() const;
    void setDocumentSize(const JI::DocumentSize &newDocumentSize);

public slots:
    void    clearSelection();
    void    removeAllItems();
    void    removeItem(JIGraphicsItem* item);
    void    fitToView();
signals:
    void    selectionChanged();
    void    itemRotationChangedByUser(JIGraphicsItem* item, double rotation);
    void    itemsChanged();
    void    orientationChanged();
private slots:
    void    onItemRotationChangedByUser(double rotation);
    void    onItemChanged();
    void    sendItemsChangedIfAny();
protected:
    void startItemsChangedTimer(const int msInterval);
    void addItemWithId(JIGraphicsItem* item, const ItemId id);
    void addItem(JIGraphicsItem* item);
    void resizeEvent(QResizeEvent *event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void updateSceneSize();
    bool eventFilter(QObject *obj, QEvent *event) override;
    ItemId m_lastItemId = 0;
    QHash<ItemId,JIGraphicsItem*> m_items;
    JIGraphicsScene m_scene;
    JI::DocumentSize m_documentSize;
    JIImageProvider m_provider;
    QSet<JIGraphicsItem*> m_changedItems;
    bool    m_paperSizeChanged = false;
    QTimer m_itemsChangedTimer;
    bool    m_mousePressed = false;

};

#endif // JIGRAPHICSVIEW_H
