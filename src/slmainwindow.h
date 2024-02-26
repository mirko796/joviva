#ifndef SLMAINWINDOW_H
#define SLMAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include "slundoredo.h"
#include <QSettings>
#include <QTranslator>
#include "jiaboutdlg.h"
#include "jipapersizedlg.h"
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
        actSaveAs,
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
        actShowButtonText,
        actAbout,
        actPaperSize,
        actExportImage
    };

public:
    typedef QHash<QString, QSharedPointer<QTranslator> > Translators;
    SLMainWindow(QSettings* settings, const Translators& translators, QWidget *parent = nullptr);
    ~SLMainWindow();

    void loadFile(const QString& fileName);
    void loadFromByteArray(const QByteArray& data, const QString &filename);
    void saveFile(const QString& filename);
    QByteArray saveToByteArray() const;

    void pasteTextWasm(const QString& text);
    void pasteImageWasm(const QByteArray& data);
private:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void initActions();
    void initMainMenu();
    void loadFromSettings();
    void updateWindowTitle();
    void setFileName(const QString& fileName);
    bool isModified() const;
    /**
     * @brief ensureAllSaved
     * @param onProceed function to be called if all data is saved
     * @return
     */
    void ensureAllSaved(std::function<void()> onProceed);
    Ui::SLMainWindow *ui;
    QSettings* m_settings;
    Translators m_translators;
    QMenu* m_languageMenu;
    SLUndoRedo<QJsonObject> m_undoRedo;
    bool    m_restoring = false;
    QHash<Action, QAction*> m_actions;
    QString m_fileName;
    QJsonObject m_savedContent;
    JIAboutDlg  m_aboutDlg;
    JIPaperSizeDlg m_paperSizeDlg;
private slots:
    void    print();
    void    printPreview();
    void    pasteContent();
    void    addImageFromLocalFile();
    void    addText(const QString& text=QString());
    void    startNewDocument();
    void    saveToFile();
    void    saveAsToFile();
    void    loadFromFile();
    // important for undo/redo
    void    onItemsChanged();
    void    updateActions();
    void    undo();
    void    redo();
    void    updateButtonsTextVisibility();
    void    onLanguageActionTriggered();
    void    about();
    void    setPaperSize();
    void    onPaperSizeDialogFinished(int result);
    void    exportAsImage();
};
#endif // SLMAINWINDOW_H
