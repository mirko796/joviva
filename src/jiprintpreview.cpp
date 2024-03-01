#include "jiprintpreview.h"
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>

QPageSize pageSizeFromDocumentSize(const JI::DocumentSize& docsize)
{
    switch (docsize.paperFormat()) {
    case JI::psA4:
        return QPageSize(QPageSize::A4);
    case JI::psB4:
        return QPageSize(QPageSize::B4);
    case JI::psLetter:
        return QPageSize(QPageSize::Letter);
    case JI::psLegal:
        return QPageSize(QPageSize::Legal);
    default:
        break;
    }

    return QPageSize(docsize.sizeInPixels());
}
JIPrintPreview::JIPrintPreview(JIGraphicsView *view) :
    m_view(view),
    m_orientation(Qt::Vertical)
{

}

void JIPrintPreview::printPreview(const JI::DocumentSize& docsize)
{
    const QPageLayout::Orientation orientation =
        docsize.orientation() == Qt::Vertical ? QPageLayout::Portrait : QPageLayout::Landscape;
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation( orientation );
    printer.setPageSize( pageSizeFromDocumentSize(docsize) );
//    printer.setFullPage(true);


    QPrintPreviewDialog preview(&printer, m_view->parentWidget());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &JIPrintPreview::paintPage);

    preview.exec();
}

void JIPrintPreview::printDirect(const JI::DocumentSize &docsize)
{
    const QPageLayout::Orientation orientation =
        docsize.orientation() == Qt::Vertical ? QPageLayout::Portrait : QPageLayout::Landscape;
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation( orientation );
    printer.setPageSize( pageSizeFromDocumentSize(docsize) );
    //    printer.setFullPage(true);
    QPrintDialog *dialog = new QPrintDialog(&printer);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted) {
        return;
    }
    paintPage(&printer);
}

void JIPrintPreview::paintPage(QPrinter *printer)
{
    auto view = m_view;
    QPainter painter(printer);

    const QRectF pageRect = printer->paperRect(QPrinter::DevicePixel);
//    painter.drawLine(pageRect.topLeft(),pageRect.bottomRight());
    view->scene()->render(&painter, pageRect, view->sceneRect());
}

