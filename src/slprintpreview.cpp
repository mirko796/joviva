#include "slprintpreview.h"
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>
SLPrintPreview::SLPrintPreview(SLGraphicsView *view) :
    m_view(view),
    m_orientation(Qt::Vertical)
{

}

void SLPrintPreview::printPreview()
{
    const QPageLayout::Orientation orientation =
            m_orientation == Qt::Vertical ? QPageLayout::Portrait : QPageLayout::Landscape;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation( orientation );
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setFullPage(true);


    QPrintPreviewDialog preview(&printer, m_view->parentWidget());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &SLPrintPreview::paintPage);

    preview.exec();
}

void SLPrintPreview::printDirect()
{
    const QPageLayout::Orientation orientation =
        m_orientation == Qt::Vertical ? QPageLayout::Portrait : QPageLayout::Landscape;
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation( orientation );
    printer.setPageSize(QPageSize(QPageSize::A4));
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

Qt::Orientation SLPrintPreview::orientation() const
{
    return m_orientation;
}

void SLPrintPreview::setOrientation(Qt::Orientation newOrientation)
{
    m_orientation = newOrientation;
}
