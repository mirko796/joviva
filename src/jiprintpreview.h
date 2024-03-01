#ifndef JIPRINTPREVIEW_H
#define JIPRINTPREVIEW_H

#include <QObject>
#include <QPrinter>
#include "jigraphicsview.h"
class JIPrintPreview : public QObject
{
    Q_OBJECT
public:
    JIPrintPreview(JIGraphicsView* view);
public slots:
    void    printPreview(const JI::DocumentSize &docsize);
    void    printDirect(const JI::DocumentSize &docsize);
private slots:
    void paintPage(QPrinter* printer);

private:
    JIGraphicsView* m_view = nullptr;
    Qt::Orientation m_orientation;
};

#endif // JIPRINTPREVIEW_H
