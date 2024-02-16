#ifndef JIABOUTDLG_H
#define JIABOUTDLG_H

#include <QDialog>

namespace Ui {
class JIAboutDlg;
}

class JIAboutDlg : public QDialog
{
    Q_OBJECT

public:
    explicit JIAboutDlg(QWidget *parent = nullptr);
    ~JIAboutDlg();

private:
    Ui::JIAboutDlg *ui;
private slots:
    void aboutQt();
};

#endif // JIABOUTDLG_H
