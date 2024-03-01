#ifndef JIPAPERSIZEDLG_H
#define JIPAPERSIZEDLG_H

#include <QDialog>
#include "jicommon.h"

namespace Ui {
class JIPaperSizeDlg;
}

class JIPaperSizeDlg : public QDialog
{
    Q_OBJECT

public:
    explicit JIPaperSizeDlg(QWidget *parent = nullptr);
    ~JIPaperSizeDlg();

    void    setDocumentSize(const JI::DocumentSize& size);
    JI::DocumentSize    getDocumentSize() const;
private:
    Ui::JIPaperSizeDlg *ui;
    bool    m_updatingSize = false;

    void    populatePaperFormats();
    void    populateOrientation();
    void    updateSizeInPixels(const QWidget* pivotWidget);
    void    onSizeChangedManually();
};

#endif // JIPAPERSIZEDLG_H
