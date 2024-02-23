#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "utest.h"
#pragma GCC diagnostic pop
#include "jipapersizedlg.h"
#include <QJsonDocument>
#include <QEventLoop>
UTEST(PaperSizeDlg,basics)
{
    JIPaperSizeDlg dlg;
    SL::DocumentSize ds;
    ds.paperFormat = SL::psA4;
    ds.orientation = Qt::Orientation::Vertical;
    ds.sizeInPixels = QSize(2100,2970);
    dlg.setDocumentSize(ds);
    ASSERT_TRUE( dlg.getDocumentSize() == ds);
    ds.orientation = Qt::Orientation::Horizontal;
    dlg.setDocumentSize(ds);
    qDebug()<<ds.toString()<<dlg.getDocumentSize().toString();
    ASSERT_TRUE( dlg.getDocumentSize() == ds);
    dlg.setModal(true);
    dlg.show();
    // wait until dlg is closed
    QEventLoop loop;
    QObject::connect(&dlg, SIGNAL(finished(int)), &loop, SLOT(quit()));
    loop.exec();
    qDebug()<<"Finished with result:"<<dlg.result();
}
