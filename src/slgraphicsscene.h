#ifndef SLGRAPHICSSCENE_H
#define SLGRAPHICSSCENE_H
#include <QGraphicsScene>
#include <slgraphicsitem.h>
class SLGraphicsScene: public QGraphicsScene
{
public:
    SLGraphicsScene(QObject *parent = nullptr);
    ~SLGraphicsScene() override;
public slots:
    void bringSelectedItemToTop();
    void bringSelectedItemToBottom();
private slots:
    void onSelectionChanged();
private:    
    SLGraphicsItem* m_lastSelectedItem = nullptr;
    double m_selectedItemZValue = 0;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

};

#endif // SLGRAPHICSSCENE_H
