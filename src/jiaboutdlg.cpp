#include "jiaboutdlg.h"
#include "ui_jiaboutdlg.h"

JIAboutDlg::JIAboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JIAboutDlg)
{
    ui->setupUi(this);
    // preserve only close button in title bar
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setWindowTitle(qApp->applicationName());
    ui->lbl_title->setText(qApp->applicationName());
    ui->lbl_version->setText(APP_VERSION);
    ui->lbl_hash->setText(GIT_COMMIT);
    ui->lbl_date->setText(QString("%1 %2").arg(__DATE__).arg(__TIME__));
    connect(ui->btn_aboutQt, &QPushButton::clicked, this, &JIAboutDlg::aboutQt);
    connect(ui->btn_close, &QPushButton::clicked, this, &JIAboutDlg::close);
}

JIAboutDlg::~JIAboutDlg()
{
    delete ui;
}

void JIAboutDlg::aboutQt()
{
    qApp->aboutQt();
}
