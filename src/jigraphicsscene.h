#ifndef JIGRAPHICSSCENE_H
#define JIGRAPHICSSCENE_H
#include <QGraphicsScene>
#include <jigraphicsitem.h>
class JIGraphicsScene: public QGraphicsScene
{
public:
    JIGraphicsScene(QObject *parent = nullptr);
    ~JIGraphicsScene() override;
public slots:
    void bringSelectedItemToTop();
    void bringSelectedItemToBottom();
private slots:
    void onSelectionChanged();
private:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    JIGraphicsItem* m_lastSelectedItem = nullptr;
    double m_selectedItemZValue = 0;

};

#endif // JIGRAPHICSSCENE_H
