#include "jimainwindow.h"
#include "ui_jimainwindow.h"
#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include "jiprintpreview.h"
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTimer>
#include <QBuffer>
#include <QPlainTextEdit>
#include "jiactions.h"
#include <QProcessEnvironment>
using namespace JIActions;

#ifdef Q_OS_WASM
#include <emscripten.h>
#include <emscripten/bind.h>
const char* byteArray = "Hello, World!";
static JIMainWindow* g_mainWindow = nullptr;
void pasteTextWasm(const std::string& text) {
    if (g_mainWindow) {
        g_mainWindow->pasteTextWasm(QString::fromStdString(text));
    }
}

void pasteImageWasm(const std::string& b64) {
    const QByteArray data = QByteArray::fromBase64(b64.c_str());
    g_mainWindow->pasteImageWasm(data);
}

std::string getImageAsBase64String() {
    const QByteArray bytes(g_mainWindow->exportAsImageToByteArray());
    std::string ret = bytes.toBase64().toStdString();
    return ret;
}

EMSCRIPTEN_BINDINGS(mylibrary) {
    emscripten::function("getImageAsBase64String", &getImageAsBase64String);
    emscripten::function("pasteTextWasm", &pasteTextWasm);
    emscripten::function("pasteImageWasm", &pasteImageWasm);
}
#endif
JIMainWindow::JIMainWindow(QSettings *settings, const Translators& translators, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JIMainWindow)
    , m_settings(settings)
    , m_translators(translators)
    , m_undoRedo(QJsonObject())
{
#ifdef Q_OS_WASM
    g_mainWindow = this;
#endif
    /* let's install translator before any ui actions */
    const QString lang = m_settings->value(JI::SettingsKeyLanguage, "").toString();
    if (m_translators.contains(lang)) {
        qApp->installTranslator(m_translators.value(lang).data());
    }
    ui->setupUi(this);
    initActions();
    initMainMenu();
    connect(ui->graphicsView, &JIGraphicsView::itemsChanged,
            this, &JIMainWindow::onItemsChanged);
    connect(ui->graphicsView, &JIGraphicsView::orientationChanged,
            this, &JIMainWindow::updateActions);
    connect(ui->graphicsView,&JIGraphicsView::filesDropped,
            this, &JIMainWindow::onFilesDropped);
    ui->mw_sidebar->setGraphicsView(ui->graphicsView);
    ui->splitter->setSizes({100000,20});

    loadFromSettings();
//    updateActions();
//    updateButtonsTextVisibility();
//    updateWindowTitle();
    startNewDocument();
}

JIMainWindow::~JIMainWindow()
{
    m_settings->sync();
#ifdef Q_OS_WASM
    g_mainWindow = nullptr;
#endif
    delete ui;
}

void JIMainWindow::loadFromSettings()
{
    getAction(actShowButtonText)->setChecked(
        m_settings->value(JI::SettingsKeyShowButtonTexts, false).toBool()
        );
    getAction(JIActions::actSmallIcons)->setChecked(
        m_settings->value(JI::SettingsKeyUseSmallIcons, false).toBool()
        );
    const QString lang = m_settings->value(JI::SettingsKeyLanguage, "").toString();
    foreach(QAction* action, m_languageMenu->actions()) {
        const bool checked = action->text()==lang;

        action->setChecked( checked );
        if (checked) {
            action->trigger();
        }
    }
    JIMainWindow::updateButtonsTextVisibility();
}

void JIMainWindow::updateWindowTitle()
{
    QString title = QString("JovIva %1 - Simple Image Editor").arg(APP_VERSION);
    if (m_fileName.size()) {
        const QString basename = QFileInfo(m_fileName).fileName();
        const QString modifiedMarker = isModified() ? "*" : "";
        title+=QString(" [ %1 %2]").arg(basename,modifiedMarker);
    }
    setWindowTitle(title);
}

void JIMainWindow::pasteTextWasm(const QString &text)
{
    auto focusedPlainTextControl = qobject_cast<QPlainTextEdit*>(qApp->focusWidget());
    if (focusedPlainTextControl) {
        focusedPlainTextControl->insertPlainText(text);
    } else {
        addText(text);
    }
}

void JIMainWindow::pasteImageWasm(const QByteArray &data)
{
    QPixmap pixmap;
    pixmap.loadFromData(data);
    if (pixmap.isNull()) {
        // show error using QMessageBox
        QMessageBox::warning(nullptr, qApp->applicationName(), QObject::tr("Failed to load image from clipboard"));
    } else {
        ui->graphicsView->addPixmapItem(pixmap);
    }

}

void JIMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    QTimer::singleShot(100, ui->graphicsView, &JIGraphicsView::fitToView);
}

void JIMainWindow::closeEvent(QCloseEvent *event)
{
    if (isModified()) {
        const auto btn =
            QMessageBox::question(this, qApp->applicationName(), tr("Do you want to save changes?"),
                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                  QMessageBox::Yes);
        if (btn==QMessageBox::Yes) {
            saveToFile();
            event->accept();
        } else if (btn==QMessageBox::Cancel) {
            event->ignore();
        } else {
            event->accept();
        }
    } else {
        event->accept();
    }
}

void JIMainWindow::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    updateWindowTitle();
    updateActions();
}

bool JIMainWindow::isModified() const
{
    if (ui->graphicsView->itemsCount()==0) {
        return false;
    }
    return m_savedContent!=ui->graphicsView->asJson(JIGraphicsView::jfItemsOnly);
}

void JIMainWindow::ensureAllSaved(std::function<void()> onProceed)
{
    if (isModified()) {
        auto onConfirmed = [this, onProceed](){
            saveToFile();
            onProceed();
        };
        JI::showQuestionAsync(
            tr("Save Changes"), tr("Do you want to save changes?"),
            {
                { QMessageBox::Yes, onConfirmed },
                { QMessageBox::No, onProceed },
                { QMessageBox::Cancel, nullptr }
            });
    } else {
        onProceed();
    }
}

void JIMainWindow::print()
{
#ifdef Q_OS_WASM
    emscripten_async_run_script("printImage();",100);
#else
    ui->graphicsView->clearSelection();
    JIPrintPreview preview(ui->graphicsView);
    preview.printDirect(ui->graphicsView->documentSize());
#endif
}

void JIMainWindow::printPreview()
{
#ifdef Q_OS_WASM
    emscripten_async_run_script("openImageInBrowser();",100);
#else
    ui->graphicsView->clearSelection();
    JIPrintPreview preview(ui->graphicsView);
    preview.printPreview(ui->graphicsView->documentSize());
#endif
}

void JIMainWindow::pasteContent()
{
#ifdef Q_OS_WASM
    emscripten_async_run_script("getClipboardData();",1000);
#else
    qDebug()<<"TEST2";
    auto clp = qApp->clipboard();
    qDebug()<<clp->image().size();
    qDebug()<<"TEXT:"<<clp->text();
    const QImage image = clp->image();
    if (image.size().width() && image.size().height()) {
        // Retrieve the image data from the clipboard as a QImage

        // Convert the QImage to a QPixmap
        const QPixmap pixmap = QPixmap::fromImage(image);
        if (pixmap.width()==0 || pixmap.height()==0)
        {
            // show error using QMessageBox
            QMessageBox::warning(nullptr, qApp->applicationName(), tr("Failed to load image from clipboard"));
        }
        else
        {
            ui->graphicsView->addPixmapItem(pixmap);
        }
    } else {
        const QString text = clp->text();
        if (!text.isEmpty()) {
            addText(text.trimmed());
        }
    }
#endif
}

void JIMainWindow::addImageFromLocalFile()
{
    // do the same using getOpenFileContent
    auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent)
    {
        if (fileContent.size())
        {
            QPixmap pixmap;
            pixmap.loadFromData(fileContent);
            if (pixmap.isNull())
            {
                // show error using QMessageBox
                QMessageBox::warning(nullptr, qApp->applicationName(), tr("Failed to load image from file %1").arg(fileName));
            }
            else
            {
                ui->graphicsView->addPixmapItem(pixmap);
            }
        }
    };
    JI::showOpenFileDialog(this, tr("Open Image File"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
             "Image Files (*.jpg *.jpeg *.png *.tiff *.bmp)", fileContentReady);
}

void JIMainWindow::startNewDocument()
{
    auto onConfirmed = [this](){
        ui->graphicsView->removeAllItems();
        setFileName("");
        m_savedContent = QJsonObject();
        m_undoRedo.reset(
            ui->graphicsView->asJson(JIGraphicsView::jfItemsOnly)
            );
        updateActions();
        ui->graphicsView->fitToView();
    };
    ensureAllSaved( onConfirmed );
}

void JIMainWindow::saveFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to open file %1 for writing").arg(filename));
        return;
    }
    const QByteArray ba( saveToByteArray() );
    file.write(ba);
    file.close();
    m_savedContent=ui->graphicsView->asJson(JIGraphicsView::jfItemsOnly);
    setFileName(filename);
}

QByteArray JIMainWindow::saveToByteArray() const
{
    QJsonObject obj = ui->graphicsView->asJson(JIGraphicsView::jfItemsAndImages);
    QJsonDocument doc(obj);
    const QString json = doc.toJson(QJsonDocument::Indented);
    return json.toUtf8();
}
void JIMainWindow::saveToFile()
{
    if (m_fileName.isEmpty()) {
        saveAsToFile();
    } else {
        saveFile(m_fileName);
    }
}

void JIMainWindow::saveAsToFile()
{
    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QByteArray data( saveToByteArray() );
    auto onSaved = [this](const QString &filename) {
        m_savedContent = ui->graphicsView->asJson(JIGraphicsView::jfItemsOnly);
        setFileName(filename);
    };
    JI::showSaveFileDialog(
        this, tr("Save File"),
        desktopPath,
        JI::defaultFileFilter(),
        m_fileName.isEmpty() ? tr("Untitled.ji") : QFileInfo(m_fileName).fileName(),
        JI::DefaultExtension,
        data,
        onSaved);
}

void JIMainWindow::loadFromFile()
{
    auto onProceed = [this]() {
        auto fileContentReady = [this](const QString &filename, const QByteArray &fileContent) {
            loadFromByteArray(fileContent, filename);
            ui->graphicsView->fitToView();
        };

        const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        JI::showOpenFileDialog(
            this, tr("Open File"),
            desktopPath,
            JI::defaultFileFilter(),
            fileContentReady);
    };
    ensureAllSaved(onProceed);
}

void JIMainWindow::onItemsChanged()
{
    if (m_restoring) {
        qDebug()<<"Restoring is true, ignoring items changed signal...";
        return;
    }
    auto view = ui->graphicsView;
    qDebug()<<"Main window items changed...";
    auto data = view->asJson(JIGraphicsView::jfItemsOnly);
    if (m_undoRedo.head()==data) {
        qDebug()<<"This state is already in head of undo redo stack";
    } else {
        qDebug()<<"Adding new state to undo redo stack, data keys"<<data.keys();
        m_undoRedo.push(data);
        updateActions();
    }
    updateActions();
    updateWindowTitle();
    ui->graphicsView->fitToView();
}

void JIMainWindow::updateActions()
{
    getAction(actUndo)->setEnabled(m_undoRedo.canUndo());
    getAction(actRedo)->setEnabled(m_undoRedo.canRedo());

    const auto orientation = ui->graphicsView->orientation();
    getAction(actLandscape)->setChecked(orientation==Qt::Horizontal);
    getAction(actPortrait)->setChecked(orientation==Qt::Vertical);

    bool modified = isModified() || m_fileName.isEmpty();

    getAction(actSave)->setEnabled(modified);
    getAction(actSaveAs)->setEnabled(m_fileName.isEmpty()==false);
}

void JIMainWindow::undo()
{
    if (m_undoRedo.undo()) {
        m_restoring = true;
        auto view = ui->graphicsView;
        auto data = m_undoRedo.head();
        // dump data to console
        qDebug()<<"UNDO DATA:"<<QJsonDocument(data).toJson(QJsonDocument::Indented);
        view->fromJson(data, JIGraphicsView::jfItemsOnly);
        m_restoring = false;
    }
    updateActions();
    updateWindowTitle();
    ui->graphicsView->fitToView();
}

void JIMainWindow::redo()
{
    if (m_undoRedo.redo()) {
        m_restoring = true;
        auto view = ui->graphicsView;
        auto data = m_undoRedo.head();
        view->fromJson(data, JIGraphicsView::jfItemsOnly);
        m_restoring = false;
    }
    updateActions();
    updateWindowTitle();
    ui->graphicsView->fitToView();
}

void JIMainWindow::updateButtonsTextVisibility()
{
    const auto buttons = findChildren<QToolButton*>();
    const bool textVisible = getAction(actShowButtonText)->isChecked();
    QSize iconSize(24,24);
    if (getAction(JIActions::actSmallIcons)->isChecked()) {
        iconSize = QSize(16,16);
    }
    auto actBig = getAction(JIActions::actBigIcons);
    QSignalBlocker _b1(actBig);
    actBig->setChecked(iconSize.width()==24);

    for (auto button: buttons) {
        button->setIconSize(iconSize);
        button->setToolButtonStyle(textVisible ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    }
    m_settings->setValue(JI::SettingsKeyShowButtonTexts, textVisible);
    m_settings->setValue(JI::SettingsKeyUseSmallIcons, iconSize.width()==16);
}

void JIMainWindow::onLanguageActionTriggered()
{
    auto act = qobject_cast<QAction*>(sender());
    if (act==nullptr) {
        return;
    }
    const QString newLang = act->text();
    foreach(const QString& lang, m_translators.keys()) {
        auto translator = m_translators.value(lang);
        if (lang==newLang) {
            qApp->installTranslator(translator.data());
        } else {
            qApp->removeTranslator(translator.data());
        }
    }
    foreach(QAction* action, m_languageMenu->actions()) {
        QSignalBlocker _b(action);
        action->setChecked(action==act);
    }
    m_settings->setValue(JI::SettingsKeyLanguage,newLang);
    ui->retranslateUi(this);
    updateWindowTitle();
}

void JIMainWindow::about()
{
    m_aboutDlg.setModal(true);
    m_aboutDlg.show();
}

void JIMainWindow::setPaperSize()
{
    m_paperSizeDlg.setModal(true);
    m_paperSizeDlg.setDocumentSize(ui->graphicsView->documentSize());
    m_paperSizeDlg.show();
    connect(&m_paperSizeDlg, &JIPaperSizeDlg::finished, this, &JIMainWindow::onPaperSizeDialogFinished, Qt::UniqueConnection);
}

void JIMainWindow::onPaperSizeDialogFinished(int result)
{
    if (result!=QDialog::Accepted) {
        return;
    }
    auto dlg = qobject_cast<JIPaperSizeDlg*>(sender());
    if (!dlg) {
        return;
    }
    ui->graphicsView->setDocumentSize( dlg->getDocumentSize() );
}

QByteArray JIMainWindow::exportAsImageToByteArray()
{
    ui->graphicsView->clearSelection();
    //save content of view to image
    const QSize paperSize = ui->graphicsView->paperSize();
    QImage img(paperSize,QImage::Format_ARGB32);
    QPainter painter(&img);
    ui->graphicsView->scene()->render(&painter, img.rect(), ui->graphicsView->sceneRect());
    // save image as png to buffer
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    buffer.close();
    return bytes;
}

void JIMainWindow::exportAsImage()
{
    const QByteArray bytes( exportAsImageToByteArray() );

    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    JI::showSaveFileDialog(
        this, tr("Export As Image"),
        desktopPath,
        "PNG Files (*.png)",
        "Untitled.png","png",bytes);
}

void JIMainWindow::onFilesDropped(const QStringList &files)
{
    // first check do we have ".ji" file, if so open it
    const QString extension = QString(".%1").arg(JI::DefaultExtension);
    foreach(const QString& fn, files) {
        if (fn.toLower().endsWith(extension)) {
            auto onProceed = [this,fn]() {
                loadFile(fn);
            };
            ensureAllSaved(onProceed);
            return;
        }
    }
    foreach(const QString& fn, files) {
        QPixmap pix(fn);
        if (pix.width()>0) {
            ui->graphicsView->addPixmapItem(pix);
        }
    }
}

#ifdef Q_OS_LINUX
void saveFileFromResourceToLocal(const QString &resourcePath, const QString &localFilePath) {
    QFile resourceFile(resourcePath);
    if (!resourceFile.exists()) {
        qDebug() << "Resource file does not exist.";
        return;
    }
    if (!resourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open resource file for reading.";
        return;
    }
    QFile localFile(localFilePath);
    if (!localFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open local file for writing.";
        return;
    }
    QByteArray data = resourceFile.readAll();
    localFile.write(data);
    resourceFile.close();
    localFile.close();
}
void JIMainWindow::createDesktopIconOnLinux()
{
    // when running from appimage container this should be set and used as target for shortcut
    const QString appImagePath = QProcessEnvironment::systemEnvironment().value("APPIMAGE");

    const QString appPath = appImagePath.isEmpty() ? qApp->applicationFilePath() : appImagePath;

    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d(appDataPath);
    if (d.exists()==false) {
        d.mkdir(appDataPath);
    }
    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QString appShortcutPath = desktopPath+"/JovIva.desktop";
    if (QFile::exists(appShortcutPath)) {
        const int btn = QMessageBox::question(
            this,
            tr("Confirm"),
            tr("The shortuct already exists on your desktop.\nDo you want to replace it?")
            );
        if (btn!=QMessageBox::Yes) {
            return;
        }
    }

    const QString appIconPath = appDataPath+"/joviva.png";
    const QStringList desktopEntry = {
        "[Desktop Entry]",
        "Version=1.0",
        "Type=Application",
        QString("Name=%1").arg(qApp->applicationName()),
        "Comment=Simple image manipulation program ( https://github.com/mirko796/joviva )",
        QString("Exec=%1").arg(appPath),
        QString("Icon=%1").arg(appIconPath),
        "Terminal=false",
        "StartupNotify=true"
    };
    qDebug()<<appPath<<" data:"<<appDataPath;
    /* copy icon to local file system under appData path */
    saveFileFromResourceToLocal(":/app-icon.png",appIconPath);
    const bool fileCopied = QFile::exists(appIconPath);
    if (!fileCopied) {
        QMessageBox::warning(this,
                             "Error",
                             "Failed to save app icon!\nFile name: "+appIconPath);
    }

    /* save desktop entry to appShortcutPath on desktop */
    QFile f(appShortcutPath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(desktopEntry.join("\n").toLatin1());
        f.close();
        f.setPermissions(f.permissions()|
                         QFile::ExeOwner|
                         QFile::ExeUser|
                         QFile::ExeGroup|
                         QFile::ExeOther);
        QMessageBox::information(this,
                                 qApp->applicationName(),
                                 "Shortcut created.\nFile name: "+appShortcutPath);
    } else {
        QMessageBox::warning(this,
                             "Error",
                             "Failed to create shortcut on your desktop!\nFile name: "+appShortcutPath);
    }
    qDebug()<<"File copied to:"<<appIconPath<<fileCopied;

}
#endif
void JIMainWindow::addText(const QString& text)
{
    JI::TextParams params;
    params.text = text.isEmpty() ? tr("Text Item") : text;
    params.font = QFont("Arial", JI::DefaultFontSize);
    params.color = Qt::black;
    params.alignment = Qt::AlignCenter;
    ui->graphicsView->addTextItem(params);
}


void JIMainWindow::loadFile(const QString &filename)
{
    qDebug()<<"Loading file:"<<filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to open file %1 for reading").arg(filename));
        return;
    }
    setFileName("");
    const QByteArray data = file.readAll();
    file.close();
    loadFromByteArray(data, filename);
}

void JIMainWindow::loadFromByteArray(const QByteArray &data, const QString& filename)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error!=QJsonParseError::NoError) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to parse json file %1: %2").arg(filename).arg(error.errorString()));
        return;
    }
    if (!ui->graphicsView->fromJson(doc.object(), JIGraphicsView::jfItemsAndImages)) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to load file %1").arg(filename));
        return;
    }
    m_savedContent = ui->graphicsView->asJson(JIGraphicsView::jfItemsOnly);
    m_undoRedo.reset(m_savedContent);
    setFileName(filename);
}

void JIMainWindow::initActions()
{
    auto setActionButton = [](Action act, QToolButton* btn) {
        auto action = getAction(act);
        if (btn) {
            btn->setDefaultAction(action);
            QString tooltip = action->text();
            const auto shortcut = action->shortcut();
            if (shortcut.isEmpty()==false) {
                tooltip+=QString(" (%1)").arg(shortcut.toString());
            }
            btn->setToolTip(tooltip);
        }
    };
    auto connectAction = [this](Action act, auto slot) {
        auto action = getAction(act);
        if (action) {
            connect(action, &QAction::triggered, this, slot);
        }
    };
    setActionButton(actNew, ui->btn_new);
    setActionButton(actOpen, ui->btn_load);
    setActionButton(actSave, ui->btn_save);
    setActionButton(actPaste, ui->btn_paste);
    setActionButton(actAddImage, ui->btn_addImage);
    setActionButton(actAddText, ui->btn_text);
    setActionButton(actUndo, ui->btn_undo);
    setActionButton(actRedo, ui->btn_redo);
    setActionButton(actPortrait, ui->btn_portrait);
    setActionButton(actLandscape, ui->btn_landscape);
    setActionButton(actPrint, ui->btn_print);
    setActionButton(actPrintPreview, ui->btn_preview);
    setActionButton(actPaperSize, ui->btn_paperSize);
    setActionButton(actExportImage, ui->btn_exportImage);

    connectAction(actNew, &JIMainWindow::startNewDocument);
    connectAction(actOpen, &JIMainWindow::loadFromFile);
    connectAction(actSave, &JIMainWindow::saveToFile);
    connectAction(actSaveAs, &JIMainWindow::saveAsToFile);
    connectAction(actPaste, &JIMainWindow::pasteContent);
    connectAction(actAddImage, &JIMainWindow::addImageFromLocalFile);
    connectAction(actAddText, [this]() {
        addText();
    });
    connectAction(actUndo, &JIMainWindow::undo);
    connectAction(actRedo, &JIMainWindow::redo);
    connectAction(actPortrait, [this](){
        ui->graphicsView->setOrientation(Qt::Vertical);
    });
    connectAction(actLandscape, [this](){
        ui->graphicsView->setOrientation(Qt::Horizontal);
    });
    connectAction(actPrint, &JIMainWindow::print);
    connectAction(actPrintPreview, &JIMainWindow::printPreview);
    connectAction(actShowButtonText, &JIMainWindow::updateButtonsTextVisibility);
    connectAction(actAbout, &JIMainWindow::about);
    connectAction(actPaperSize, &JIMainWindow::setPaperSize);
    connectAction(actExportImage, &JIMainWindow::exportAsImage);

    connectAction(actBigIcons, &JIMainWindow::updateButtonsTextVisibility);
    connectAction(actSmallIcons, &JIMainWindow::updateButtonsTextVisibility);
#ifdef Q_OS_LINUX
    connectAction(actCreateDesktopIcon, &JIMainWindow::createDesktopIconOnLinux);
#endif
}

void JIMainWindow::initMainMenu()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(getAction(actNew));
    fileMenu->addAction(getAction(actOpen));
    fileMenu->addAction(getAction(actSave));
    fileMenu->addAction(getAction(actSaveAs));
    fileMenu->addSeparator();
    fileMenu->addAction(getAction(actExportImage));
    fileMenu->addSeparator();
    fileMenu->addAction(getAction(actPrint));
    fileMenu->addAction(getAction(actPrintPreview));
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(getAction(actUndo));
    editMenu->addAction(getAction(actRedo));
    editMenu->addSeparator();
    editMenu->addAction(getAction(actPaste));
    editMenu->addAction(getAction(actAddImage));
    editMenu->addAction(getAction(actAddText));
    editMenu->addSeparator();
    editMenu->addAction(getAction(actPaperSize));
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(getAction(actPortrait));
    viewMenu->addAction(getAction(actLandscape));
    viewMenu->addSeparator();
    viewMenu->addAction(getAction(actShowButtonText));
    viewMenu->addSeparator();
    viewMenu->addAction(getAction(actSmallIcons));
    viewMenu->addAction(getAction(actBigIcons));
    viewMenu->addSeparator();
    m_languageMenu = viewMenu->addMenu(tr("Languages"));
    for (const auto& lang: m_translators.keys()) {
        auto action = new QAction(lang, this);
        action->setCheckable(true);
        m_languageMenu->addAction(action);
        connect( action, &QAction::triggered, this, &JIMainWindow::onLanguageActionTriggered);
    }
    viewMenu->addSeparator();
#ifdef Q_OS_LINUX
    viewMenu->addAction(getAction(actCreateDesktopIcon));
#endif
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(getAction(actAbout));
}

