#include "jipapersizedlg.h"
#include "ui_jipapersizedlg.h"
#include <QScopedValueRollback>

JIPaperSizeDlg::JIPaperSizeDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JIPaperSizeDlg)
{
    ui->setupUi(this);
    populatePaperFormats();
    populateOrientation();
    ui->sb_dpi->setValue(100);
    updateSizeInPixels(ui->sb_dpi);

    connect(ui->sb_dpi, &QSpinBox::valueChanged, this, &JIPaperSizeDlg::onSizeChangedManually);
    connect(ui->sb_width, &QSpinBox::valueChanged, this, &JIPaperSizeDlg::onSizeChangedManually);
    connect(ui->sb_height, &QSpinBox::valueChanged, this, &JIPaperSizeDlg::onSizeChangedManually);
    connect(ui->cmb_paperFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) { updateSizeInPixels(nullptr); });
    connect(ui->cmb_orientation, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) { updateSizeInPixels(ui->cmb_orientation); });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    setWindowTitle(tr("Set Paper Size"));
}

JIPaperSizeDlg::~JIPaperSizeDlg()
{
    delete ui;
}

void JIPaperSizeDlg::setDocumentSize(const JI::DocumentSize &size)
{
    m_updatingSize = true;
    ui->sb_width->setValue(size.sizeInPixels().width());
    ui->sb_height->setValue(size.sizeInPixels().height());
    ui->cmb_orientation->setCurrentIndex(ui->cmb_orientation->findData(size.orientation()));
    ui->cmb_paperFormat->setCurrentIndex(ui->cmb_paperFormat->findData(size.paperFormat()));
    const JI::PaperFormatInfo info = JI::getPaperFormatInfo(size.paperFormat());
    const int dpi = 25.4 * size.sizeInPixels().width() / info.sizeInMM.width();
    ui->sb_dpi->setValue(dpi);
    m_updatingSize = false;
    updateSizeInPixels(ui->sb_width);
}

JI::DocumentSize JIPaperSizeDlg::getDocumentSize() const
{
    JI::DocumentSize ret;
    QSize s(ui->sb_width->value(), ui->sb_height->value());
    ret.setSizeInPixels(s);
    ret.setOrientation(static_cast<Qt::Orientation>(ui->cmb_orientation->currentData().toInt()));
    ret.setPaperFormat(static_cast<JI::PaperFormat>(ui->cmb_paperFormat->currentData().toInt()));
    return ret;
}

void JIPaperSizeDlg::populatePaperFormats()
{
    QHash<QString, JI::PaperFormatInfo> paperFormats;
    const auto formats = JI::getPaperFormats();
    for (JI::PaperFormat format : JI::getPaperFormats())
    {
        auto info = JI::getPaperFormatInfo(format);
        const QString name = QString("%1 (%2 x %3mm)").arg(info.name).arg(info.sizeInMM.width()).arg(info.sizeInMM.height());
        paperFormats[name] = info;
    }

    auto cmb = ui->cmb_paperFormat;
    QStringList names = paperFormats.keys();
    names.sort();
    foreach(const QString& name, names)
    {
        cmb->addItem(name, paperFormats[name].paperFormat);
    }
}

void JIPaperSizeDlg::populateOrientation()
{
    auto cmb = ui->cmb_orientation;
    cmb->addItem(tr("Portrait"), Qt::Vertical);
    cmb->addItem(tr("Landscape"), Qt::Horizontal);
}

void JIPaperSizeDlg::updateSizeInPixels(const QWidget *pivotWidget)
{
    if (m_updatingSize) {
        return;
    }
    QScopedValueRollback<bool> _rb(m_updatingSize, true);
    const JI::PaperFormatInfo info = JI::getPaperFormatInfo(static_cast<JI::PaperFormat>(ui->cmb_paperFormat->currentData().toInt()));
    ui->sb_dpi->setEnabled(!info.sizeInMM.isEmpty());
    int wPix = 0;
    int hPix = 0;

    if (info.sizeInMM.isEmpty())
    {
        wPix = ui->sb_width->value();
        hPix = ui->sb_height->value();
        if (pivotWidget==ui->cmb_orientation)
        {
            auto tmp = wPix;
            wPix = qMin(wPix, hPix);
            hPix = qMax(tmp, hPix);
            if (ui->cmb_orientation->currentData().toInt() == Qt::Horizontal)
            {
                std::swap(wPix, hPix);
            }
        }
    } else {
        wPix = info.sizeInMM.width() * ui->sb_dpi->value() / 25.4;
        hPix = info.sizeInMM.height() * ui->sb_dpi->value() / 25.4;
        if ( (pivotWidget==ui->sb_width) || (pivotWidget==ui->sb_height) ){
            if (pivotWidget == ui->sb_width)
            {
                hPix = info.sizeInMM.height() * ui->sb_width->value() / info.sizeInMM.width();
            } else {
                wPix = info.sizeInMM.width() * ui->sb_height->value() / info.sizeInMM.height();
            }
            ui->sb_dpi->setValue(25.4 * wPix / info.sizeInMM.width());
        }
        auto tmp = wPix;
        wPix = qMin(wPix, hPix);
        hPix = qMax(tmp, hPix);
        if (ui->cmb_orientation->currentData().toInt() == Qt::Horizontal)
        {
            std::swap(wPix, hPix);
        }
    }
    ui->sb_width->setValue(wPix);
    ui->sb_height->setValue(hPix);
    qDebug()<<"wPix: "<<wPix<<" hPix: "<<hPix<<" dpi: "<<ui->sb_dpi->value();
}

void JIPaperSizeDlg::onSizeChangedManually()
{
    const QWidget* pivotWidget = qobject_cast<QWidget*>(sender());
    updateSizeInPixels(pivotWidget);
}
