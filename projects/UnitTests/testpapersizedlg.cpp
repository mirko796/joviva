#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "utest.h"
#pragma GCC diagnostic pop
#include "jipapersizedlg.h"
#include <QJsonDocument>
UTEST(PaperSizeDlg,basics)
{
    JIPaperSizeDlg dlg;
    SL::DocumentSize ds;
    ds.paperFormat = SL::psA4;
    ds.orientation = Qt::Orientation::Horizontal;
    ds.sizeInPixels = QSize(2100,2970);
    dlg.setDocumentSize(ds);
    dlg.exec();
}
