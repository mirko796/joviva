#ifndef JISIDEBARWIDGET_H
#define JISIDEBARWIDGET_H

#include <QWidget>
#include <QToolButton>
class JIGraphicsView;
class JIGraphicsItem;
namespace Ui {
class JISideBarWidget;
}

class JISideBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JISideBarWidget(QWidget *parent = nullptr);
    ~JISideBarWidget();
    
    JIGraphicsView *graphicsView() const;
    void setGraphicsView(JIGraphicsView *newGraphicsView);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation newOrientation);
private slots:
    void    updateUI();
    void    updateTextItem();
    void    rotateLeft();
    void    rotateRight();
    void    mirror();
    void    onRotationChangedViaSlider(int rotation);
    void    onRotationChangedByUser(JIGraphicsItem* item, double rotation);
    void    onTransparentChangedByUser(bool checked);
    void    onTextAlignButtonClicked();
private:
    Ui::JISideBarWidget *ui;
    QList<QToolButton*> m_textButtons;
    QList<QToolButton*> m_textAlignButtons;
    bool   m_updatingUI = false;
    JIGraphicsView *m_graphicsView = nullptr;
};

#endif // JISIDEBARWIDGET_H
