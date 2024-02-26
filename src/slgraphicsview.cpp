#include "slgraphicsview.h"
#include <QGraphicsTextItem>
#include "slgraphicspixmapitem.h"
#include "slgraphicstextitem.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QTimer>
SLGraphicsView::SLGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setScene(&m_scene);
    m_documentSize.setSizeInPixels(SL::getPaperFormatInfo(SL::psA4).sizeInMM*10);
    m_documentSize.setPaperFormat(SL::psA4);
    m_documentSize.setOrientation(Qt::Vertical);

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    connect(&m_scene, &QGraphicsScene::selectionChanged,
            this, &SLGraphicsView::selectionChanged);

    updateSceneSize();
    m_itemsChangedTimer.setSingleShot(true);
    connect(&m_itemsChangedTimer, &QTimer::timeout,
            this,&SLGraphicsView::sendItemsChangedIfAny);
    qApp->installEventFilter(this);
}

SLGraphicsView::~SLGraphicsView()
{
    qApp->removeEventFilter(this);
}

void SLGraphicsView::addPixmapItem(const QPixmap &pixmap)
{
    const auto id = m_provider.addPixmap(pixmap);
    auto item = new SLGraphicsPixmapItem(&m_provider);
    item->setPixmapId(id);

    const QSize ps = paperSizeWithOrientation();
    item->setRect(QRectF(
        ps.width()*0.3, ps.height()*0.3,
        ps.width()*0.4, ps.height()*0.4
        ));
    addItem(item);    
}

void SLGraphicsView::addTextItem(const SL::TextParams &textParams)
{
    auto item = new SLGraphicsTextItem;
    const QSize ps = paperSizeWithOrientation();
    item->setRect(QRectF(
        ps.width()*0.3, ps.height()*0.3,
        ps.width()*0.4, ps.height()*0.4
        ));
    item->setTextParams(textParams);
    addItem(item);
}

void SLGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitToView();
}

void SLGraphicsView::drawBackground(QPainter *painter, const QRectF &rect) {
    // Fill the background with the custom color
    painter->fillRect(rect, Qt::gray);

    // Call the base class implementation to paint the scene
    QGraphicsView::drawBackground(painter, rect);
}

void SLGraphicsView::contextMenuEvent(QContextMenuEvent *event) {
    // get item at pos
    // if item is null, show default menu
    // else show item menu
    SLGraphicsItem* item=dynamic_cast<SLGraphicsItem*>(this->itemAt(event->pos()));
    qDebug()<<"Item at:"<<event->pos()<<item;
    if (item)
    {
        item->setSelected(true);
        QMenu* contextMenu=new QMenu(this);
        auto actDelete = contextMenu->addAction(tr("Delete"));
        connect(actDelete, &QAction::triggered, item, [this,item](bool checked) {
            Q_UNUSED(checked);
            removeItem(item);
        });
        auto actRaise = contextMenu->addAction(tr("Bring to front"));
        connect(actRaise, &QAction::triggered, item, [this,item](bool checked) {
            Q_UNUSED(checked);
            qDebug()<<"Raise item:"<<item;
            m_scene.bringSelectedItemToTop();
            emit itemsChanged();
        });
        auto actLower = contextMenu->addAction(tr("Move to back"));
        connect(actLower, &QAction::triggered, item, [this,item](bool checked) {
            Q_UNUSED(checked);
            qDebug()<<"Lower item:"<<item;
            m_scene.bringSelectedItemToBottom();
            emit itemsChanged();
        });
//        auto actPreserveAspectRatio = contextMenu.addAction(tr("Preserve aspect ratio"));
//        actPreserveAspectRatio->setCheckable(true);
//        actPreserveAspectRatio->setChecked(item->preserveAspectRatio());
//        connect(actPreserveAspectRatio, &QAction::triggered, [&](bool checked) {
//            qDebug()<<"Preserve aspect ratio:"<<checked;
//            item->setPreserveAspectRatio(checked);
//        });
        connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QObject::deleteLater);
        contextMenu->move(event->globalPos());
        contextMenu->show();

    }
    // Add more actions as needed

}

void SLGraphicsView::updateSceneSize()
{
    QSize s(m_documentSize.sizeInPixels());
    if (m_documentSize.orientation() == Qt::Horizontal)
    {
        s.transpose();
    }
    m_scene.setSceneRect(QRectF(QPointF(0,0), s));
}

bool SLGraphicsView::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
//        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        m_mousePressed = true;
    } else if (event->type() == QEvent::MouseButtonRelease) {
//        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
//        if (mouseEvent->button() == Qt::LeftButton) {
//            // Left mouse button released
//            qDebug() << "Left mouse button released";
//            // Your custom code here
//        }
        m_mousePressed = false;
        startItemsChangedTimer(100);
    }

    // Standard event processing
    return QObject::eventFilter(obj, event);
}

SL::DocumentSize SLGraphicsView::documentSize() const
{
    return m_documentSize;
}

void SLGraphicsView::setDocumentSize(const SL::DocumentSize &newDocumentSize)
{
    m_documentSize = newDocumentSize;
    updateSceneSize();
    fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
    m_paperSizeChanged = true;
    onItemChanged();
    emit orientationChanged();
}

Qt::Orientation SLGraphicsView::orientation() const
{
    return m_documentSize.orientation();
}

void SLGraphicsView::setOrientation(Qt::Orientation newOrientation)
{
    m_documentSize.setOrientation(newOrientation);
    updateSceneSize();
    fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
    m_paperSizeChanged = true;
    onItemChanged();
    emit orientationChanged();
}

SLGraphicsItem *SLGraphicsView::selectedItem() const
{
    auto items = m_scene.selectedItems();
    if (items.isEmpty())
    {
        return nullptr;
    }
    return dynamic_cast<SLGraphicsItem*>(items.first());
}

int SLGraphicsView::itemsCount() const
{
    return m_items.count();
}

QJsonObject SLGraphicsView::asJson(const JsonFlags flags) const
{
    QJsonObject ret;
    QJsonObject items;
    auto it = m_items.constBegin();
    while (it != m_items.constEnd()) {
        items[QString::number(it.key())] = it.value()->asJson();
        ++it;
    }
    ret[JK_ITEMS] = items;
    ret[JK_PAPER_WIDTH] = m_documentSize.sizeInPixels().width();
    ret[JK_PAPER_HEIGHT] = m_documentSize.sizeInPixels().height();
    ret[JK_ORIENTATION] = m_documentSize.orientation();
    ret[JK_PAPERFORMAT] = m_documentSize.paperFormat();
    if (flags == jfItemsAndImages)
    {
        ret[JK_PROVIDER] = m_provider.asJson();
    }
    return ret;
}

bool SLGraphicsView::fromJson(const QJsonObject &obj, const JsonFlags flags)
{
    if (flags == jfItemsAndImages) {
        m_provider.clear();
    }
    updateSceneSize();
    removeAllItems();

    const QStringList requiredKeys({
        JK_ITEMS,
        JK_PAPER_WIDTH,
        JK_PAPER_HEIGHT,
        JK_ORIENTATION
    });
    foreach(const auto key, requiredKeys) {
        if (!obj.contains(key)) {
            qWarning()<<"Missing key:"<<key<<" Keys:"<<obj.keys();
            return false;
        }
    }
    const QJsonObject itemsObj = obj[JK_ITEMS].toObject();
    const int paperWidth = obj[JK_PAPER_WIDTH].toInt();
    const int paperHeight = obj[JK_PAPER_HEIGHT].toInt();
    const Qt::Orientation orientation = static_cast<Qt::Orientation>(obj[JK_ORIENTATION].toInt());
    if (flags == jfItemsAndImages)
    {
        if (!obj.contains(JK_PROVIDER))
        {
            return false;
        }
        const QJsonObject providerObj = obj[JK_PROVIDER].toObject();
        if (!m_provider.fromJson(providerObj))
        {
            return false;
        }
    }
    if (obj.contains(JK_PAPERFORMAT))
    {
        m_documentSize.setPaperFormat(static_cast<SL::PaperFormat>(obj[JK_PAPERFORMAT].toInt()));
    }
    else
    {
        m_documentSize.setPaperFormat(SL::psA4);
    }
    m_documentSize.setSizeInPixels(QSize(paperWidth, paperHeight));
    m_documentSize.setOrientation(orientation);
    auto it = itemsObj.constBegin();
    while (it != itemsObj.constEnd()) {
        const ItemId id = it.key().toUInt();
        const QJsonObject itemObj = it.value().toObject();
        const ItemType type = static_cast<ItemType>(itemObj[SLGraphicsItem::JK_TYPE].toInt());
        qDebug()<<"Item type:"<<type<< SLGraphicsPixmapItem::Type << SLGraphicsTextItem::Type<<m_lastItemId;
        SLGraphicsItem* item = nullptr;
        switch (static_cast<int>(type)) {
        case SLGraphicsPixmapItem::Type:
            item = new SLGraphicsPixmapItem(&m_provider);
            break;
        case SLGraphicsTextItem::Type:
            item = new SLGraphicsTextItem;
            break;
        default:
            qWarning()<<"Unknown item type:"<<type;
            break;
        }
        if (item) {
            item->fromJson(itemObj);
            addItemWithId(item,id);
            qDebug()<<"Item with id:"<<id<<"added.  Last id:"<<m_lastItemId;
        }
        ++it;
    }
    clearSelection();
    updateSceneSize();
    return true;
}

void SLGraphicsView::onItemRotationChangedByUser(double rotation)
{
    qDebug()<<"View item rotation changed by user:"<<rotation;
    auto ptr = dynamic_cast<SLGraphicsItem*>(QObject::sender());
    if (ptr)
    {
        emit itemRotationChangedByUser(ptr, rotation);
    }
}

void SLGraphicsView::onItemChanged()
{
    auto ptr = dynamic_cast<SLGraphicsItem*>(QObject::sender());
    m_changedItems.insert(ptr);
    if (m_mousePressed)
    {
        return;
    }
    startItemsChangedTimer(100);
}

void SLGraphicsView::sendItemsChangedIfAny()
{
    if ( (m_changedItems.size() == 0) && !m_paperSizeChanged)
    {
        return;
    }
    qDebug()<<"Clearing changed items, had:"<<m_changedItems.count()<<" paper size changed:"<<m_paperSizeChanged;
    m_changedItems.clear();
    m_paperSizeChanged = false;
    emit itemsChanged();
}

void SLGraphicsView::startItemsChangedTimer(const int msInterval)
{
    if (m_itemsChangedTimer.isActive()) {
        m_itemsChangedTimer.stop();
    }
    m_itemsChangedTimer.start(msInterval);
}

void SLGraphicsView::addItemWithId(SLGraphicsItem *item, const ItemId id)
{
    m_items.insert(id, item);
    m_scene.addItem(item);
    clearSelection();
    item->setSelected(true);
    if (id > m_lastItemId)
    {
        m_lastItemId = id;
    }
    connect(item, &SLGraphicsItem::itemChanged,
            this, &SLGraphicsView::onItemChanged);
    connect( item, &SLGraphicsItem::rotationChangedByUser,
            this, &SLGraphicsView::onItemRotationChangedByUser);
}

void SLGraphicsView::addItem(SLGraphicsItem *item)
{
    const ItemId id = ++m_lastItemId;
    // get max sort order
    int maxSortOrder = 0;
    for (auto _item : m_scene.items())
    {
        auto item = dynamic_cast<SLGraphicsItem*>(_item);
        maxSortOrder = qMax(maxSortOrder, item->sortOrder());
    }
    item->setSortOrder(maxSortOrder+1);

    addItemWithId(item, id);
    emit itemsChanged();
}

void SLGraphicsView::clearSelection()
{
    m_scene.clearSelection();
}

void SLGraphicsView::removeAllItems()
{
    m_scene.clear();
    m_lastItemId = 0;
    m_items.clear();
}

void SLGraphicsView::removeItem(SLGraphicsItem *item)
{
    const ItemId id = m_items.key(item);
    qDebug()<<"Delete item:"<<item<<id;
    m_scene.removeItem(item);
    m_items.remove(id);
    item->deleteLater();
    emit itemsChanged();
}

void SLGraphicsView::fitToView()
{
    fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
}

QSize SLGraphicsView::paperSize() const
{
    return m_documentSize.sizeInPixels();
}

void SLGraphicsView::setPaperSize(const QSize &newPaperSize)
{
    m_documentSize.setSizeInPixels(newPaperSize);
    m_paperSizeChanged = true;
    updateSceneSize();
    onItemChanged();
}

QSize SLGraphicsView::paperSizeWithOrientation() const
{
    QSize ret = paperSize();
    if (orientation() == Qt::Horizontal)
    {
        ret.transpose();
    }
    return ret;
}
