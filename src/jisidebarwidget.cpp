#include "jisidebarwidget.h"
#include "ui_jisidebarwidget.h"
#include "jigraphicsview.h"
#include "jigraphicstextitem.h"
#include <QScopeGuard>
// TODO: add 4 rotation presets 0,90,180,270
// TODO: add bring to front, send to back
// TODO: add preserve aspect ratio
// TODO: add text option
JISideBarWidget::JISideBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JISideBarWidget)
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
    connect(ui->sl_rotation, &QSlider::valueChanged, this, &JISideBarWidget::onRotationChanged);
    connect(ui->chb_transparent, &QCheckBox::stateChanged, this, &JISideBarWidget::onTransparentChangedByUser);
#ifdef Q_OS_WASM
    // disable text selection on wasm
    ui->pt_text->setTextInteractionFlags(Qt::TextEditable);
#endif

    connect(ui->fcb_font, &QFontComboBox::currentFontChanged, [this](const QFont & /*font*/) {
        updateTextItem();
    });
    foreach(auto btn, m_textButtons) {
        if (m_textAlignButtons.contains(btn))
        {
            connect(btn, &QToolButton::clicked, this, &JISideBarWidget::onTextAlignButtonClicked);
        }
        else
        {
            connect(btn, &QToolButton::toggled, [this](bool /*checked*/) {
                updateTextItem();
            });
        }
    }
    connect(ui->pt_text, &QPlainTextEdit::textChanged, this, &JISideBarWidget::updateTextItem);
    updateUI();
    setMinimumWidth(300);
}

JISideBarWidget::~JISideBarWidget()
{
    delete ui;
}

JIGraphicsView *JISideBarWidget::graphicsView() const
{
    return m_graphicsView;
}

void JISideBarWidget::setGraphicsView(JIGraphicsView *newGraphicsView)
{
    if (m_graphicsView)
    {
        m_graphicsView->disconnect(this);
    }
    m_graphicsView = newGraphicsView;
    if (m_graphicsView)
    {
        connect(m_graphicsView, &JIGraphicsView::selectionChanged, this, &JISideBarWidget::updateUI);
        connect(m_graphicsView, &JIGraphicsView::itemRotationChangedByUser, this, &JISideBarWidget::onRotationChangedByUser);
    }
    updateUI();
}

Qt::Orientation JISideBarWidget::orientation() const
{
    if (!m_graphicsView)
    {
        return Qt::Vertical;
    }
    return m_graphicsView->orientation();
}

void JISideBarWidget::setOrientation(Qt::Orientation newOrientation)
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

void JISideBarWidget::updateUI()
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
    const JIGraphicsTextItem *textItem = dynamic_cast<JIGraphicsTextItem*>(selectedItem);
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

void JISideBarWidget::updateTextItem()
{
    qDebug()<<"Updating text item";
    if (m_updatingUI) return;
    auto selectedItem = m_graphicsView->selectedItem();
    if (!selectedItem) return;
    auto textItem = dynamic_cast<JIGraphicsTextItem*>(selectedItem);
    if (!textItem) return;
    JI::TextParams textParams;
    textParams.text = ui->pt_text->toPlainText();
    textParams.font = QFont(
        ui->fcb_font->currentFont().family(),
        JI::DefaultFontSize,
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

void JISideBarWidget::onRotationChanged(int rotation)
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

void JISideBarWidget::onRotationChangedByUser(JIGraphicsItem * /*item*/, double rotation)
{
    qDebug()<<"Rotation changed by user:"<<rotation;
    if (rotation<0)
    {
        rotation += 360;
    }
    QSignalBlocker _b(ui->sl_rotation);
    ui->sl_rotation->setValue(rotation);
}

void JISideBarWidget::onTransparentChangedByUser(bool checked)
{
    if (m_updatingUI) return;
    auto selectedItem = m_graphicsView->selectedItem();
    if (!selectedItem) return;
    selectedItem->setTransparentBackground(checked);
}

void JISideBarWidget::onTextAlignButtonClicked()
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

