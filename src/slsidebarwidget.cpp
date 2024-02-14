#include "slsidebarwidget.h"
#include "ui_slsidebarwidget.h"
#include "slgraphicsview.h"
#include "slgraphicstextitem.h"
#include <QScopeGuard>
// TODO: add 4 rotation presets 0,90,180,270
// TODO: add bring to front, send to back
// TODO: add preserve aspect ratio
// TODO: add text option
SLSideBarWidget::SLSideBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SLSideBarWidget)
{
    ui->setupUi(this);
    m_textButtons =
        {
        ui->btn_bold,
        ui->btn_italic,
        ui->btn_underline,
        ui->btn_hollow,
        ui->btn_left,
        ui->btn_center,
        ui->btn_right
        };
    m_textAlignButtons = {ui->btn_left, ui->btn_center, ui->btn_right};
    connect(ui->sl_rotation, &QSlider::valueChanged, this, &SLSideBarWidget::onRotationChanged);
    connect(ui->chb_transparent, &QCheckBox::stateChanged, this, &SLSideBarWidget::onTransparentChangedByUser);

    connect(ui->fcb_font, &QFontComboBox::currentFontChanged, [this](const QFont & /*font*/) {
        updateTextItem();
    });
    foreach(auto btn, m_textButtons) {
        if (m_textAlignButtons.contains(btn))
        {
            connect(btn, &QToolButton::clicked, this, &SLSideBarWidget::onTextAlignButtonClicked);
        }
        else
        {
            connect(btn, &QToolButton::toggled, [this](bool /*checked*/) {
                updateTextItem();
            });
        }
    }
    connect(ui->pt_text, &QPlainTextEdit::textChanged, this, &SLSideBarWidget::updateTextItem);
    updateUI();
    setMinimumWidth(300);
}

SLSideBarWidget::~SLSideBarWidget()
{
    delete ui;
}

SLGraphicsView *SLSideBarWidget::graphicsView() const
{
    return m_graphicsView;
}

void SLSideBarWidget::setGraphicsView(SLGraphicsView *newGraphicsView)
{
    if (m_graphicsView)
    {
        m_graphicsView->disconnect(this);
    }
    m_graphicsView = newGraphicsView;
    if (m_graphicsView)
    {
        connect(m_graphicsView, &SLGraphicsView::selectionChanged, this, &SLSideBarWidget::updateUI);
        connect(m_graphicsView, &SLGraphicsView::itemRotationChangedByUser, this, &SLSideBarWidget::onRotationChangedByUser);
//        connect(m_graphicsView, &SLGraphicsView::selectionChanged, this, &SLSideBarWidget::onSelectionChanged);
    }
    updateUI();
}

Qt::Orientation SLSideBarWidget::orientation() const
{
    if (!m_graphicsView)
    {
        return Qt::Vertical;
    }
    return m_graphicsView->orientation();
}

void SLSideBarWidget::setOrientation(Qt::Orientation newOrientation)
{
    if (!m_graphicsView)
    {
        return;
    }
    qDebug()<<"Orientation1:"<<newOrientation;
    m_graphicsView->setOrientation(newOrientation);
    qDebug()<<"Orientation2:"<<m_graphicsView->orientation();
    updateUI();
}

void SLSideBarWidget::updateUI()
{
    if (!m_graphicsView)
    {
        return;
    }
    m_updatingUI = true;
    QScopeGuard _g([this]() {m_updatingUI = false;});
    auto selectedItem = m_graphicsView->selectedItem();
    ui->lbl_rotation->setVisible(selectedItem);
    ui->sl_rotation->setVisible(selectedItem);
    ui->chb_transparent->setVisible(selectedItem);
    const SLGraphicsTextItem *textItem = dynamic_cast<SLGraphicsTextItem*>(selectedItem);
    if (selectedItem)
    {
        ui->sl_rotation->setValue(selectedItem->rotation());
        ui->chb_transparent->setChecked(selectedItem->transparentBackground());
    }
    ui->lbl_text->setVisible(textItem);
    ui->lbl_font->setVisible(textItem);
    ui->fcb_font->setVisible(textItem);
    ui->pt_text->setVisible(textItem);
    foreach(auto btn, m_textButtons)
    {
        btn->setVisible(textItem);
    }
    if (textItem)
    {
        ui->pt_text->setPlainText(textItem->textParams().text);
        ui->fcb_font->setCurrentFont(textItem->textParams().font);
        ui->btn_italic->setChecked(textItem->textParams().font.italic());
        ui->btn_underline->setChecked(textItem->textParams().font.underline());
        ui->btn_bold->setChecked(textItem->textParams().font.weight() > QFont::Normal);
        ui->btn_hollow->setChecked(textItem->textParams().hollow);
        const auto alignment = textItem->textParams().alignment;
        ui->btn_left->setChecked(alignment.testFlag(Qt::AlignLeft));
        ui->btn_center->setChecked(alignment.testFlag(Qt::AlignCenter));
        ui->btn_right->setChecked(alignment.testFlag(Qt::AlignRight));
        ui->pt_text->setFocus();
        ui->pt_text->selectAll();
    }

}

void SLSideBarWidget::updateTextItem()
{
    qDebug()<<"Updating text item";
    if (m_updatingUI) return;
    auto selectedItem = m_graphicsView->selectedItem();
    if (!selectedItem) return;
    auto textItem = dynamic_cast<SLGraphicsTextItem*>(selectedItem);
    if (!textItem) return;
    SL::TextParams textParams;
    textParams.text = ui->pt_text->toPlainText();
    textParams.font = QFont(
        ui->fcb_font->currentFont().family(),
        SL::DefaultFontSize,
        ui->btn_bold->isChecked()?QFont::Black:QFont::Normal,
        ui->btn_italic->isChecked()
        );
    textParams.font.setUnderline(
                       ui->btn_underline->isChecked()
        );
    textParams.hollow = ui->btn_hollow->isChecked();
    textParams.alignment.setFlag(Qt::AlignLeft, ui->btn_left->isChecked());
    textParams.alignment.setFlag(Qt::AlignCenter, ui->btn_center->isChecked());
    textParams.alignment.setFlag(Qt::AlignRight, ui->btn_right->isChecked());
    qDebug()<<"Flags:"<<textParams.alignment;
    textItem->setTextParams(textParams);
}

void SLSideBarWidget::onRotationChanged(int rotation)
{
    if (m_graphicsView)
    {
        auto selectedItem = m_graphicsView->selectedItem();
        if (selectedItem)
        {
            selectedItem->setRotation(rotation);
        }
    }
}

void SLSideBarWidget::onRotationChangedByUser(SLGraphicsItem * /*item*/, double rotation)
{
    qDebug()<<"Rotation changed by user:"<<rotation;
    if (rotation<0)
    {
        rotation += 360;
    }
    QSignalBlocker _b(ui->sl_rotation);
    ui->sl_rotation->setValue(rotation);
}

void SLSideBarWidget::onTransparentChangedByUser(bool checked)
{
    if (m_updatingUI) return;
    auto selectedItem = m_graphicsView->selectedItem();
    if (!selectedItem) return;
    selectedItem->setTransparentBackground(checked);
}

void SLSideBarWidget::onTextAlignButtonClicked()
{
    auto btn = dynamic_cast<QToolButton*>(sender());
    QSet<QToolButton*> buttons = {ui->btn_left, ui->btn_center, ui->btn_right};
    // keep only clicked button checked
    foreach(auto b, buttons)
    {
        b->setChecked(b == btn);
    }
    updateTextItem();
}

