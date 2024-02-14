#include "slgraphicsscene.h"
#include <QPainter>
#include <QGraphicsItem>

SLGraphicsScene::SLGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
    connect(this, &SLGraphicsScene::selectionChanged,
            this, &SLGraphicsScene::onSelectionChanged);
}

SLGraphicsScene::~SLGraphicsScene()
{
    disconnect(this);
}

void SLGraphicsScene::onSelectionChanged()
{
    const QList<QGraphicsItem*> selectedItems = this->selectedItems();
    QGraphicsItem* si = selectedItems.size() ? selectedItems.constFirst() : nullptr;
    SLGraphicsItem* selectedItem = dynamic_cast<SLGraphicsItem*>(si);
    if (m_lastSelectedItem!=selectedItem)
    {
        if (m_lastSelectedItem) {
            m_lastSelectedItem->setZValue(m_lastSelectedItem->sortOrder());
        }
        m_lastSelectedItem = selectedItem;
        if (selectedItem)
        {
            m_selectedItemZValue = selectedItem->zValue();
        }
    }
    if (selectedItem)
    {
        // loop through all items and get max Z value
        qreal maxZ = 0;
        for (auto item : this->items())
        {
            maxZ = qMax(maxZ, item->zValue());
        }
        selectedItem->setZValue(maxZ+1);
    }

}

void SLGraphicsScene::bringSelectedItemToTop()
{
    if (m_lastSelectedItem)
    {
        // loop through all items and get max sort order
        int maxSortOrder = 0;
        for (auto _item : this->items())
        {
            auto item = dynamic_cast<SLGraphicsItem*>(_item);
            if (item!=m_lastSelectedItem) {
                maxSortOrder = qMax(maxSortOrder, item->sortOrder());
            }
        }
        m_lastSelectedItem->setSortOrder(maxSortOrder+1);
    }
    this->clearSelection();
}

void SLGraphicsScene::bringSelectedItemToBottom()
{
    if (m_lastSelectedItem)
    {
        // loop through all items and get max Z value
        int minSortOrder = std::numeric_limits<int>::max();
        for (auto _item : this->items())
        {
            auto item = dynamic_cast<SLGraphicsItem*>(_item);
            if (item!=m_lastSelectedItem) {
                minSortOrder = qMin(minSortOrder, item->sortOrder());
            }
        }
        m_lastSelectedItem->setSortOrder(minSortOrder-1);
    }
    this->clearSelection();
}

void SLGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->fillRect(sceneRect(), Qt::white);
}
