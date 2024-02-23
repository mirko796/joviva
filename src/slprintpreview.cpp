#include "slprintpreview.h"
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>

QPageSize pageSizeFromDocumentSize(const SL::DocumentSize& docsize)
{
    switch (docsize.paperFormat()) {
    case SL::psA4:
        return QPageSize(QPageSize::A4);
    case SL::psB4:
        return QPageSize(QPageSize::B4);
    case SL::psLetter:
        return QPageSize(QPageSize::Letter);
    case SL::psLegal:
        return QPageSize(QPageSize::Legal);
    default:
        break;
    }

    return QPageSize(docsize.sizeInPixels());
}
SLPrintPreview::SLPrintPreview(SLGraphicsView *view) :
    m_view(view),
    m_orientation(Qt::Vertical)
{

}

void SLPrintPreview::printPreview(const SL::DocumentSize& docsize)
{
    const QPageLayout::Orientation orientation =
        docsize.orientation() == Qt::Vertical ? QPageLayout::Portrait : QPageLayout::Landscape;
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation( orientation );
    printer.setPageSize( pageSizeFromDocumentSize(docsize) );
//    printer.setFullPage(true);


    QPrintPreviewDialog preview(&printer, m_view->parentWidget());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &SLPrintPreview::paintPage);

    preview.exec();
}

void SLPrintPreview::printDirect(const SL::DocumentSize &docsize)
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

void SLPrintPreview::paintPage(QPrinter *printer)
{
    auto view = m_view;
    QPainter painter(printer);

    const QRectF pageRect = printer->paperRect(QPrinter::DevicePixel);
//    painter.drawLine(pageRect.topLeft(),pageRect.bottomRight());
    view->scene()->render(&painter, pageRect, view->sceneRect());
}

