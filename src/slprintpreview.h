#ifndef SLPRINTPREVIEW_H
#define SLPRINTPREVIEW_H

#include <QObject>
#include <QPrinter>
#include "slgraphicsview.h"
class SLPrintPreview : public QObject
{
    Q_OBJECT
public:
    SLPrintPreview(SLGraphicsView* view);
    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation newOrientation);

public slots:
    void    printPreview();
    void    printDirect();
private slots:
    void paintPage(QPrinter* printer);

private:
    SLGraphicsView* m_view = nullptr;
    Qt::Orientation m_orientation;
};

#endif // SLPRINTPREVIEW_H
