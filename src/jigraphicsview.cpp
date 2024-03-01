#include "jigraphicsview.h"
#include <QGraphicsTextItem>
#include "jigraphicspixmapitem.h"
#include "jigraphicstextitem.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QTimer>
JIGraphicsView::JIGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
    setScene(&m_scene);
    m_documentSize.setSizeInPixels(JI::getPaperFormatInfo(JI::psA4).sizeInMM*10);
    m_documentSize.setPaperFormat(JI::psA4);
    m_documentSize.setOrientation(Qt::Vertical);

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    connect(&m_scene, &QGraphicsScene::selectionChanged,
            this, &JIGraphicsView::selectionChanged);

    updateSceneSize();
    m_itemsChangedTimer.setSingleShot(true);
    connect(&m_itemsChangedTimer, &QTimer::timeout,
            this,&JIGraphicsView::sendItemsChangedIfAny);
    qApp->installEventFilter(this);
}

JIGraphicsView::~JIGraphicsView()
{
    qApp->removeEventFilter(this);
}

void JIGraphicsView::addPixmapItem(const QPixmap &pixmap)
{
    const auto id = m_provider.addPixmap(pixmap);
    auto item = new JIGraphicsPixmapItem(&m_provider);
    item->setPixmapId(id);

    const QSize ps = paperSizeWithOrientation();
    item->setRect(QRectF(
        ps.width()*0.3, ps.height()*0.3,
        ps.width()*0.4, ps.height()*0.4
        ));
    addItem(item);    
}

void JIGraphicsView::addTextItem(const JI::TextParams &textParams)
{
    auto item = new JIGraphicsTextItem;
    const QSize ps = paperSizeWithOrientation();
    item->setRect(QRectF(
        ps.width()*0.3, ps.height()*0.3,
        ps.width()*0.4, ps.height()*0.4
        ));
    item->setTextParams(textParams);
    addItem(item);
}

void JIGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitToView();
}

void JIGraphicsView::drawBackground(QPainter *painter, const QRectF &rect) {
    // Fill the background with the custom color
    painter->fillRect(rect, Qt::gray);

    // Call the base class implementation to paint the scene
    QGraphicsView::drawBackground(painter, rect);
}

void JIGraphicsView::contextMenuEvent(QContextMenuEvent *event) {
    // get item at pos
    // if item is null, show default menu
    // else show item menu
    JIGraphicsItem* item=dynamic_cast<JIGraphicsItem*>(this->itemAt(event->pos()));
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

void JIGraphicsView::updateSceneSize()
{
    QSize s(m_documentSize.sizeInPixels());
    if (m_documentSize.orientation() == Qt::Horizontal)
    {
        s.transpose();
    }
    m_scene.setSceneRect(QRectF(QPointF(0,0), s));
}

bool JIGraphicsView::eventFilter(QObject *obj, QEvent *event) {
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

JI::DocumentSize JIGraphicsView::documentSize() const
{
    return m_documentSize;
}

void JIGraphicsView::setDocumentSize(const JI::DocumentSize &newDocumentSize)
{
    m_documentSize = newDocumentSize;
    updateSceneSize();
    fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
    m_paperSizeChanged = true;
    onItemChanged();
    emit orientationChanged();
}

Qt::Orientation JIGraphicsView::orientation() const
{
    return m_documentSize.orientation();
}

void JIGraphicsView::setOrientation(Qt::Orientation newOrientation)
{
    m_documentSize.setOrientation(newOrientation);
    updateSceneSize();
    fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
    m_paperSizeChanged = true;
    onItemChanged();
    emit orientationChanged();
}

JIGraphicsItem *JIGraphicsView::selectedItem() const
{
    auto items = m_scene.selectedItems();
    if (items.isEmpty())
    {
        return nullptr;
    }
    return dynamic_cast<JIGraphicsItem*>(items.first());
}

int JIGraphicsView::itemsCount() const
{
    return m_items.count();
}

QJsonObject JIGraphicsView::asJson(const JsonFlags flags) const
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

bool JIGraphicsView::fromJson(const QJsonObject &obj, const JsonFlags flags)
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
        m_documentSize.setPaperFormat(static_cast<JI::PaperFormat>(obj[JK_PAPERFORMAT].toInt()));
    }
    else
    {
        m_documentSize.setPaperFormat(JI::psA4);
    }
    m_documentSize.setSizeInPixels(QSize(paperWidth, paperHeight));
    m_documentSize.setOrientation(orientation);
    auto it = itemsObj.constBegin();
    while (it != itemsObj.constEnd()) {
        const ItemId id = it.key().toUInt();
        const QJsonObject itemObj = it.value().toObject();
        const ItemType type = static_cast<ItemType>(itemObj[JIGraphicsItem::JK_TYPE].toInt());
        qDebug()<<"Item type:"<<type<< JIGraphicsPixmapItem::Type << JIGraphicsTextItem::Type<<m_lastItemId;
        JIGraphicsItem* item = nullptr;
        switch (static_cast<int>(type)) {
        case JIGraphicsPixmapItem::Type:
            item = new JIGraphicsPixmapItem(&m_provider);
            break;
        case JIGraphicsTextItem::Type:
            item = new JIGraphicsTextItem;
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

void JIGraphicsView::onItemRotationChangedByUser(double rotation)
{
    qDebug()<<"View item rotation changed by user:"<<rotation;
    auto ptr = dynamic_cast<JIGraphicsItem*>(QObject::sender());
    if (ptr)
    {
        emit itemRotationChangedByUser(ptr, rotation);
    }
}

void JIGraphicsView::onItemChanged()
{
    auto ptr = dynamic_cast<JIGraphicsItem*>(QObject::sender());
    m_changedItems.insert(ptr);
    if (m_mousePressed)
    {
        return;
    }
    startItemsChangedTimer(100);
}

void JIGraphicsView::sendItemsChangedIfAny()
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

void JIGraphicsView::startItemsChangedTimer(const int msInterval)
{
    if (m_itemsChangedTimer.isActive()) {
        m_itemsChangedTimer.stop();
    }
    m_itemsChangedTimer.start(msInterval);
}

void JIGraphicsView::addItemWithId(JIGraphicsItem *item, const ItemId id)
{
    m_items.insert(id, item);
    m_scene.addItem(item);
    clearSelection();
    item->setSelected(true);
    if (id > m_lastItemId)
    {
        m_lastItemId = id;
    }
    connect(item, &JIGraphicsItem::itemChanged,
            this, &JIGraphicsView::onItemChanged);
    connect( item, &JIGraphicsItem::rotationChangedByUser,
            this, &JIGraphicsView::onItemRotationChangedByUser);
}

void JIGraphicsView::addItem(JIGraphicsItem *item)
{
    const ItemId id = ++m_lastItemId;
    // get max sort order
    int maxSortOrder = 0;
    for (auto _item : m_scene.items())
    {
        auto item = dynamic_cast<JIGraphicsItem*>(_item);
        maxSortOrder = qMax(maxSortOrder, item->sortOrder());
    }
    item->setSortOrder(maxSortOrder+1);

    addItemWithId(item, id);
    emit itemsChanged();
}

void JIGraphicsView::clearSelection()
{
    m_scene.clearSelection();
}

void JIGraphicsView::removeAllItems()
{
    m_scene.clear();
    m_lastItemId = 0;
    m_items.clear();
}

void JIGraphicsView::removeItem(JIGraphicsItem *item)
{
    const ItemId id = m_items.key(item);
    qDebug()<<"Delete item:"<<item<<id;
    m_scene.removeItem(item);
    m_items.remove(id);
    item->deleteLater();
    emit itemsChanged();
}

void JIGraphicsView::fitToView()
{
    fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
}

QSize JIGraphicsView::paperSize() const
{
    return m_documentSize.sizeInPixels();
}

void JIGraphicsView::setPaperSize(const QSize &newPaperSize)
{
    m_documentSize.setSizeInPixels(newPaperSize);
    m_paperSizeChanged = true;
    updateSceneSize();
    onItemChanged();
}

QSize JIGraphicsView::paperSizeWithOrientation() const
{
    QSize ret = paperSize();
    if (orientation() == Qt::Horizontal)
    {
        ret.transpose();
    }
    return ret;
}
