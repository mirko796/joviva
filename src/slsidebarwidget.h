#ifndef SLSIDEBARWIDGET_H
#define SLSIDEBARWIDGET_H

#include <QWidget>
#include <QToolButton>
class SLGraphicsView;
class SLGraphicsItem;
namespace Ui {
class SLSideBarWidget;
}

class SLSideBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SLSideBarWidget(QWidget *parent = nullptr);
    ~SLSideBarWidget();

    SLGraphicsView *graphicsView() const;
    void setGraphicsView(SLGraphicsView *newGraphicsView);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation newOrientation);
private slots:
    void    updateUI();
    void    updateTextItem();
    void    onRotationChanged(int rotation);
    void    onRotationChangedByUser(SLGraphicsItem* item, double rotation);
    void    onTransparentChangedByUser(bool checked);
    void    onTextAlignButtonClicked();
private:
    Ui::SLSideBarWidget *ui;
    QList<QToolButton*> m_textButtons;
    QList<QToolButton*> m_textAlignButtons;
    bool   m_updatingUI = false;
    SLGraphicsView *m_graphicsView = nullptr;
};

#endif // SLSIDEBARWIDGET_H
