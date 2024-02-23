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
public slots:
    void    printPreview(const SL::DocumentSize &docsize);
    void    printDirect(const SL::DocumentSize &docsize);
private slots:
    void paintPage(QPrinter* printer);

private:
    SLGraphicsView* m_view = nullptr;
    Qt::Orientation m_orientation;
};

#endif // SLPRINTPREVIEW_H
