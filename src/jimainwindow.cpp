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
#ifdef Q_OS_WASM
    g_mainWindow = nullptr;
#endif
    delete ui;
}

void JIMainWindow::loadFromSettings()
{
    m_actions[actShowButtonText]->setChecked(
        m_settings->value(JI::SettingsKeyShowButtonTexts, false).toBool()
        );
    const QString lang = m_settings->value(JI::SettingsKeyLanguage, "").toString();
    foreach(QAction* action, m_languageMenu->actions()) {
        const bool checked = action->text()==lang;

        action->setChecked( checked );
        if (checked) {
            action->trigger();
        }
    }
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
    m_actions[actUndo]->setEnabled(m_undoRedo.canUndo());
    m_actions[actRedo]->setEnabled(m_undoRedo.canRedo());

    const auto orientation = ui->graphicsView->orientation();
    m_actions[actLandscape]->setChecked(orientation==Qt::Horizontal);
    m_actions[actPortrait]->setChecked(orientation==Qt::Vertical);

    bool modified = isModified() || m_fileName.isEmpty();

    m_actions[actSave]->setEnabled(modified);
    m_actions[actSaveAs]->setEnabled(m_fileName.isEmpty()==false);
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
        // TODO: use scope quard for this
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
    const bool textVisible = m_actions[actShowButtonText]->isChecked();
    for (auto button: buttons) {
        button->setIconSize(QSize(24,24));
        button->setToolButtonStyle(textVisible ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    }
    m_settings->setValue(JI::SettingsKeyShowButtonTexts, textVisible);
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
    auto createAction = [this](Action act, const QString &text, const QKeySequence &shortcut=QKeySequence(), const QString& icon=QString(), QToolButton* btn=nullptr) {
        auto action = new QAction(text, this);
        action->setToolTip(text);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::ApplicationShortcut);
        if (icon.size()) {
            action->setIcon(QIcon(icon));
        }
        if (btn) {
            btn->setDefaultAction(action);
            QString tooltip = text;
            if (shortcut.isEmpty()==false) {
                tooltip+=QString(" (%1)").arg(shortcut.toString());
            }
            btn->setToolTip(tooltip);
        }
        m_actions[act] = action;
    };
    createAction(
        actNew,
        tr("New"),
        QKeySequence::New,
        ":/new-icon.png",
        ui->btn_new);

    createAction(
        actOpen,
        tr("Open"),
        QKeySequence::Open,
        ":/open-icon.png",
        ui->btn_load);

    createAction(
        actSave,
        tr("Save"),
        QKeySequence::Save,
        ":/save-icon.png",
        ui->btn_save);

    createAction(
        actSaveAs,
        tr("Save As"),
        QKeySequence::SaveAs,
        ":/saveas-icon.png");
    createAction(
        actPaste,
        tr("Paste"),
#ifdef Q_OS_WASM
        /* handled by JS part */
        QKeySequence(),
#else
        QKeySequence::Paste,
#endif
        ":/paste-icon.png",
        ui->btn_paste);

    createAction(
        actAddImage,
        tr("Add Image"),
        QKeySequence(tr("Ctrl+I")),
        ":/add-image-icon.png",
        ui->btn_addImage);

    createAction(
        actAddText,
        tr("Add Text"),
        QKeySequence(tr("Ctrl+T")),
        ":/add-text-icon.png",
        ui->btn_text);

    createAction(
        actUndo,
        tr("Undo"),
        QKeySequence::Undo,
        ":/undo-icon.png",
        ui->btn_undo);

    createAction(
        actRedo,
        tr("Redo"),
        QKeySequence::Redo,
        ":/redo-icon.png",
        ui->btn_redo);

    createAction(
        actPortrait,
        tr("Portrait"),
        QKeySequence(),
        ":/portrait-icon.png",
        ui->btn_portrait);
    m_actions[actPortrait]->setCheckable(true);

    createAction(
        actLandscape,
        tr("Landscape"),
        QKeySequence(),
        ":/landscape-icon.png",
        ui->btn_landscape);
    m_actions[actLandscape]->setCheckable(true);

    createAction(
        actPrint,
        tr("Print"),
        QKeySequence(),
        ":/print-icon.png",
        ui->btn_print);

    createAction(
        actPrintPreview,
        tr("Print Preview"),
        QKeySequence(tr("Ctrl+P")),
        ":/print-preview-icon.png",
        ui->btn_preview);

    createAction(
        actShowButtonText,
        tr("Show Buttons Text"));
    m_actions[actShowButtonText]->setCheckable(true);

    createAction(
        actAbout,
        tr("About"));

    createAction(
        actPaperSize,
        tr("Set Paper Size"),
        QKeySequence(),
        ":/paper-size-icon.png",
        ui->btn_paperSize);

    createAction(
        actExportImage,
        tr("Export As Image"),
        QKeySequence(),
        ":/add-image-icon.png",
        ui->btn_exportImage);

    connect(m_actions[actNew], &QAction::triggered, this, &JIMainWindow::startNewDocument);
    connect(m_actions[actOpen], &QAction::triggered, this, &JIMainWindow::loadFromFile);
    connect(m_actions[actSave], &QAction::triggered, this, &JIMainWindow::saveToFile);
    connect(m_actions[actSaveAs], &QAction::triggered, this, &JIMainWindow::saveAsToFile);
    connect(m_actions[actPaste], &QAction::triggered, this, &JIMainWindow::pasteContent);
    connect(m_actions[actAddImage], &QAction::triggered, this, &JIMainWindow::addImageFromLocalFile);
    connect(m_actions[actAddText], &QAction::triggered, this, [this](){
        addText();
    });
    connect(m_actions[actUndo], &QAction::triggered, this, &JIMainWindow::undo);
    connect(m_actions[actRedo], &QAction::triggered, this, &JIMainWindow::redo);
    connect(m_actions[actPortrait], &QAction::triggered, [this](){
        ui->graphicsView->setOrientation(Qt::Vertical);
    });
    connect(m_actions[actLandscape], &QAction::triggered, [this](){
        ui->graphicsView->setOrientation(Qt::Horizontal);
    });
    connect(m_actions[actPrint], &QAction::triggered, this, &JIMainWindow::print);
    connect(m_actions[actPrintPreview], &QAction::triggered, this, &JIMainWindow::printPreview);
    connect(m_actions[actShowButtonText], &QAction::triggered, this, &JIMainWindow::updateButtonsTextVisibility);
    connect(m_actions[actAbout], &QAction::triggered, this, &JIMainWindow::about);
    connect(m_actions[actPaperSize], &QAction::triggered, this, &JIMainWindow::setPaperSize);
    connect(m_actions[actExportImage], &QAction::triggered, this, &JIMainWindow::exportAsImage);
    // add all actions to main window
    addActions(m_actions.values());

}

void JIMainWindow::initMainMenu()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[actNew]);
    fileMenu->addAction(m_actions[actOpen]);
    fileMenu->addAction(m_actions[actSave]);
    fileMenu->addAction(m_actions[actSaveAs]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[actExportImage]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[actPrint]);
    fileMenu->addAction(m_actions[actPrintPreview]);
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_actions[actUndo]);
    editMenu->addAction(m_actions[actRedo]);
    editMenu->addSeparator();
    editMenu->addAction(m_actions[actPaste]);
    editMenu->addAction(m_actions[actAddImage]);
    editMenu->addAction(m_actions[actAddText]);
    editMenu->addSeparator();
    editMenu->addAction(m_actions[actPaperSize]);
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_actions[actPortrait]);
    viewMenu->addAction(m_actions[actLandscape]);
    viewMenu->addSeparator();
    viewMenu->addAction(m_actions[actShowButtonText]);
    viewMenu->addSeparator();
    m_languageMenu = viewMenu->addMenu(tr("Languages"));
    for (const auto& lang: m_translators.keys()) {
        auto action = new QAction(lang, this);
        action->setCheckable(true);
        m_languageMenu->addAction(action);
        connect( action, &QAction::triggered, this, &JIMainWindow::onLanguageActionTriggered);
    }

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_actions[actAbout]);
}

