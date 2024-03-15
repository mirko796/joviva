#ifndef JIMAINWINDOW_H
#define JIMAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include "jiundoredo.h"
#include <QSettings>
#include <QTranslator>
#include "jiaboutdlg.h"
#include "jipapersizedlg.h"
QT_BEGIN_NAMESPACE
namespace Ui { class JIMainWindow; }
QT_END_NAMESPACE

class JIMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QHash<QString, QSharedPointer<QTranslator> > Translators;
    JIMainWindow(QSettings* settings, const Translators& translators, QWidget *parent = nullptr);
    ~JIMainWindow();

    void loadFile(const QString& fileName);
    void loadFromByteArray(const QByteArray& data, const QString &filename);
    void saveFile(const QString& filename);
    QByteArray saveToByteArray() const;

    void pasteTextWasm(const QString& text);
    void pasteImageWasm(const QByteArray& data);

    QByteArray exportAsImageToByteArray();
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
    Ui::JIMainWindow *ui;
    QSettings* m_settings;
    Translators m_translators;
    QMenu* m_languageMenu;
    JIUndoRedo<QJsonObject> m_undoRedo;
    bool    m_restoring = false;
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
    void    onFilesDropped(const QStringList& files);

};
#endif // JIMAINWINDOW_H
