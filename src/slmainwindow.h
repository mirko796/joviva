#ifndef SLMAINWINDOW_H
#define SLMAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include "slundoredo.h"
#include <QSettings>
#include <QTranslator>
QT_BEGIN_NAMESPACE
namespace Ui { class SLMainWindow; }
QT_END_NAMESPACE

class SLMainWindow : public QMainWindow
{
    Q_OBJECT

    enum Action {
        actNew,
        actOpen,
        actSave,
        actPaste,
        actDelete,
        actUndo,
        actRedo,
        actPrint,
        actPrintPreview,
        actAddImage,
        actAddText,
        actPortrait,
        actLandscape,
        actShowButtonText
    };

public:
    typedef QHash<QString, QSharedPointer<QTranslator> > Translators;
    SLMainWindow(QSettings* settings, const Translators& translators, QWidget *parent = nullptr);
    ~SLMainWindow();

    void loadFile(const QString& fileName);
private:
    void initActions();
    void initMainMenu();
    void loadFromSettings();
    Ui::SLMainWindow *ui;
    QSettings* m_settings;
    Translators m_translators;
    QMenu* m_languageMenu;
    SLUndoRedo<QJsonObject> m_undoRedo;
    bool    m_restoring = false;
    QHash<Action, QAction*> m_actions;
private slots:
    void    print();
    void    printPreview();
    void    pasteContent();
    void    addImageFromLocalFile();
    void    addText(const QString& text=QString());
    void    removeAll();
    void    saveToFile();
    void    loadFromFile();
    // important for undo/redo
    void    onItemsChanged();
    void    updateActions();
    void    undo();
    void    redo();
    void    updateButtonsTextVisibility();
    void    onLanguageActionTriggered();
};
#endif // SLMAINWINDOW_H