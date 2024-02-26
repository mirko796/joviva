#include "slmainwindow.h"
#include "ui_slmainwindow.h"
#include <QUrl>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include "slprintpreview.h"
#include <QJsonDocument>
#include <QStandardPaths>
#include <QTimer>
#include <QBuffer>
#include <QPlainTextEdit>
#ifdef Q_OS_WASM
#include <emscripten.h>
#include <emscripten/bind.h>
const char* byteArray = "Hello, World!";
static SLMainWindow* g_mainWindow = nullptr;
void myFunction(const int v) {
    qDebug()<<"Value received:"<<v;
    QTimer::singleShot(1000,[v]() {
        qDebug()<<"Delayed:"<<v;
    });
}

void myFunctionChar(std::string text) {
    qDebug()<<"Text received:"<<text;
}

void pasteTextWasm(const std::string& text) {
    if (g_mainWindow) {
        g_mainWindow->pasteTextWasm(QString::fromStdString(text));
    }
}

void pasteImageWasm(const std::string& b64) {
    const QByteArray data = QByteArray::fromBase64(b64.c_str());
    g_mainWindow->pasteImageWasm(data);
}

std::string getByteArray() {
    const int width=4000;
    const int height=5000;
    // create qimage 128*128 draw diagonal line and save to QByteArray as PNG
    QImage img(width,height,QImage::Format_ARGB32);
    QPainter painter(&img);
    // draw vertical gradient green, yellow, red
    QLinearGradient gradient(0,0,0,height);
    gradient.setColorAt(0, Qt::green);
    gradient.setColorAt(0.5, Qt::yellow);
    gradient.setColorAt(1, Qt::red);
    painter.setBrush(gradient);
    painter.drawRect(0,0,width,height);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(0,0,width,height);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    std::string ret = byteArray.toBase64().toStdString();
    return ret;
}
EMSCRIPTEN_BINDINGS(mylibrary) {
    emscripten::function("myFunction", &myFunction);
    emscripten::function("myFunctionChar", &myFunctionChar);
    emscripten::function("getByteArray", &getByteArray, emscripten::allow_raw_pointers());
    emscripten::function("pasteTextWasm", &pasteTextWasm);
    emscripten::function("pasteImageWasm", &pasteImageWasm);
}
#endif
SLMainWindow::SLMainWindow(QSettings *settings, const Translators& translators, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SLMainWindow)
    , m_settings(settings)
    , m_translators(translators)
    , m_undoRedo(QJsonObject())
{
#ifdef Q_OS_WASM
    g_mainWindow = this;
#endif
    /* let's install translator before any ui actions */
    const QString lang = m_settings->value(SL::SettingsKeyLanguage, "").toString();
    if (m_translators.contains(lang)) {
        qApp->installTranslator(m_translators.value(lang).data());
    }
    ui->setupUi(this);
    initActions();
    initMainMenu();
    connect(ui->graphicsView, &SLGraphicsView::itemsChanged,
            this, &SLMainWindow::onItemsChanged);
    connect(ui->graphicsView, &SLGraphicsView::orientationChanged,
            this, &SLMainWindow::updateActions);
    ui->mw_sidebar->setGraphicsView(ui->graphicsView);
    ui->splitter->setSizes({100000,20});

    loadFromSettings();
//    updateActions();
//    updateButtonsTextVisibility();
//    updateWindowTitle();
    startNewDocument();
}

SLMainWindow::~SLMainWindow()
{
#ifdef Q_OS_WASM
    g_mainWindow = nullptr;
#endif
    delete ui;
}

void SLMainWindow::loadFromSettings()
{
    m_actions[actShowButtonText]->setChecked(
        m_settings->value(SL::SettingsKeyShowButtonTexts, false).toBool()
        );
    const QString lang = m_settings->value(SL::SettingsKeyLanguage, "").toString();
    foreach(QAction* action, m_languageMenu->actions()) {
        const bool checked = action->text()==lang;

        action->setChecked( checked );
        if (checked) {
            action->trigger();
        }
    }
}

void SLMainWindow::updateWindowTitle()
{
    QString title = QString("JovIva %1 - Simple Image Editor").arg(APP_VERSION);
    if (m_fileName.size()) {
        const QString basename = QFileInfo(m_fileName).fileName();
        const QString modifiedMarker = isModified() ? "*" : "";
        title+=QString(" [ %1 %2]").arg(basename,modifiedMarker);
    }
    setWindowTitle(title);
}

void SLMainWindow::pasteTextWasm(const QString &text)
{
    auto focusedPlainTextControl = qobject_cast<QPlainTextEdit*>(qApp->focusWidget());
    if (focusedPlainTextControl) {
        focusedPlainTextControl->insertPlainText(text);
    } else {
        addText(text);
    }
}

void SLMainWindow::pasteImageWasm(const QByteArray &data)
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

void SLMainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    QTimer::singleShot(100, ui->graphicsView, &SLGraphicsView::fitToView);
}

void SLMainWindow::closeEvent(QCloseEvent *event)
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

void SLMainWindow::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    updateWindowTitle();
    updateActions();
}

bool SLMainWindow::isModified() const
{
    if (ui->graphicsView->itemsCount()==0) {
        return false;
    }
    return m_savedContent!=ui->graphicsView->asJson(SLGraphicsView::jfItemsOnly);
}

void SLMainWindow::ensureAllSaved(std::function<void()> onProceed)
{
    if (isModified()) {
        auto onConfirmed = [this, onProceed](){
            saveToFile();
            onProceed();
        };
        SL::showQuestionAsync(
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

void SLMainWindow::print()
{
    ui->graphicsView->clearSelection();
    SLPrintPreview preview(ui->graphicsView);
    preview.printDirect(ui->graphicsView->documentSize());
#if 0
    //save content of view to image
    QImage img(2100,2970,QImage::Format_ARGB32);
    QPainter painter(&img);
    ui->graphicsView->scene()->render(&painter, img.rect(), ui->graphicsView->sceneRect());
    QString filePath = "/tmp/output.png"; // Replace with the path to your document
    img.save(filePath);
    // open url in browser
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
#endif
}

void SLMainWindow::printPreview()
{
    ui->graphicsView->clearSelection();
    SLPrintPreview preview(ui->graphicsView);
    preview.printPreview(ui->graphicsView->documentSize());
}

void SLMainWindow::pasteContent()
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

void SLMainWindow::addImageFromLocalFile()
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
    SL::showOpenFileDialog(this, tr("Open Image File"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
             "Image Files (*.jpg *.jpeg *.png *.tiff *.bmp)", fileContentReady);
}

void SLMainWindow::startNewDocument()
{
    auto onConfirmed = [this](){
        ui->graphicsView->removeAllItems();
        setFileName("");
        m_savedContent = QJsonObject();
        m_undoRedo.reset(
            ui->graphicsView->asJson(SLGraphicsView::jfItemsOnly)
            );
        updateActions();
        ui->graphicsView->fitToView();
    };
    ensureAllSaved( onConfirmed );
}

void SLMainWindow::saveFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to open file %1 for writing").arg(filename));
        return;
    }
    const QByteArray ba( saveToByteArray() );
    file.write(ba);
    file.close();
    m_savedContent=ui->graphicsView->asJson(SLGraphicsView::jfItemsOnly);
    setFileName(filename);
}

QByteArray SLMainWindow::saveToByteArray() const
{
    QJsonObject obj = ui->graphicsView->asJson(SLGraphicsView::jfItemsAndImages);
    QJsonDocument doc(obj);
    const QString json = doc.toJson(QJsonDocument::Indented);
    return json.toUtf8();
}
void SLMainWindow::saveToFile()
{
    if (m_fileName.isEmpty()) {
        saveAsToFile();
    } else {
        saveFile(m_fileName);
    }
}

void SLMainWindow::saveAsToFile()
{
    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QByteArray data( saveToByteArray() );
    SL::showSaveFileDialog(
        this, tr("Save File"),
        desktopPath,
        SL::defaultFileFilter(),
        m_fileName.isEmpty() ? tr("Untitled.ji") : QFileInfo(m_fileName).fileName(),
        SL::DefaultExtension,
        data);
}

void SLMainWindow::loadFromFile()
{
    auto onProceed = [this]() {
        auto fileContentReady = [this](const QString &filename, const QByteArray &fileContent) {
            loadFromByteArray(fileContent, filename);
            ui->graphicsView->fitToView();
        };

        const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        SL::showOpenFileDialog(
            this, tr("Open File"),
            desktopPath,
            SL::defaultFileFilter(),
            fileContentReady);
    };
    ensureAllSaved(onProceed);
}

void SLMainWindow::onItemsChanged()
{
    if (m_restoring) {
        qDebug()<<"Restoring is true, ignoring items changed signal...";
        return;
    }
    auto view = ui->graphicsView;
    qDebug()<<"Main window items changed...";
    auto data = view->asJson(SLGraphicsView::jfItemsOnly);
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

void SLMainWindow::updateActions()
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

void SLMainWindow::undo()
{
    if (m_undoRedo.undo()) {
        m_restoring = true;
        auto view = ui->graphicsView;
        auto data = m_undoRedo.head();
        // dump data to console
        qDebug()<<"UNDO DATA:"<<QJsonDocument(data).toJson(QJsonDocument::Indented);
        view->fromJson(data, SLGraphicsView::jfItemsOnly);
        m_restoring = false;
    }
    updateActions();
    updateWindowTitle();
    ui->graphicsView->fitToView();
}

void SLMainWindow::redo()
{
    if (m_undoRedo.redo()) {
        // TODO: use scope quard for this
        m_restoring = true;
        auto view = ui->graphicsView;
        auto data = m_undoRedo.head();
        view->fromJson(data, SLGraphicsView::jfItemsOnly);
        m_restoring = false;
    }
    updateActions();
    updateWindowTitle();
    ui->graphicsView->fitToView();
}

void SLMainWindow::updateButtonsTextVisibility()
{
    const auto buttons = findChildren<QToolButton*>();
    const bool textVisible = m_actions[actShowButtonText]->isChecked();
    for (auto button: buttons) {
        button->setIconSize(QSize(24,24));
        button->setToolButtonStyle(textVisible ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    }
    m_settings->setValue(SL::SettingsKeyShowButtonTexts, textVisible);
}

void SLMainWindow::onLanguageActionTriggered()
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
    m_settings->setValue(SL::SettingsKeyLanguage,newLang);
    ui->retranslateUi(this);
}

void SLMainWindow::about()
{
    m_aboutDlg.setModal(true);
    m_aboutDlg.show();
}

void SLMainWindow::setPaperSize()
{
    m_paperSizeDlg.setModal(true);
    m_paperSizeDlg.setDocumentSize(ui->graphicsView->documentSize());
    m_paperSizeDlg.show();
    connect(&m_paperSizeDlg, &JIPaperSizeDlg::finished, this, &SLMainWindow::onPaperSizeDialogFinished, Qt::UniqueConnection);
}

void SLMainWindow::onPaperSizeDialogFinished(int result)
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

void SLMainWindow::exportAsImage()
{
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

    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    SL::showSaveFileDialog(
        this, tr("Export As Image"),
        desktopPath,
        "PNG Files (*.png)",
        "Untitled.png","png",bytes);
}

void SLMainWindow::addText(const QString& text)
{
    SL::TextParams params;
    params.text = text.isEmpty() ? tr("Text Item") : text;
    params.font = QFont("Arial", SL::DefaultFontSize);
    params.color = Qt::black;
    params.alignment = Qt::AlignCenter;
    ui->graphicsView->addTextItem(params);
}


void SLMainWindow::loadFile(const QString &filename)
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

void SLMainWindow::loadFromByteArray(const QByteArray &data, const QString& filename)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error!=QJsonParseError::NoError) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to parse json file %1: %2").arg(filename).arg(error.errorString()));
        return;
    }
    if (!ui->graphicsView->fromJson(doc.object(), SLGraphicsView::jfItemsAndImages)) {
        QMessageBox::warning(this, qApp->applicationName(), tr("Failed to load file %1").arg(filename));
        return;
    }
    m_savedContent = ui->graphicsView->asJson(SLGraphicsView::jfItemsOnly);
    m_undoRedo.reset(m_savedContent);
    setFileName(filename);
}

void SLMainWindow::initActions()
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

    connect(m_actions[actNew], &QAction::triggered, this, &SLMainWindow::startNewDocument);
    connect(m_actions[actOpen], &QAction::triggered, this, &SLMainWindow::loadFromFile);
    connect(m_actions[actSave], &QAction::triggered, this, &SLMainWindow::saveToFile);
    connect(m_actions[actSaveAs], &QAction::triggered, this, &SLMainWindow::saveAsToFile);
    connect(m_actions[actPaste], &QAction::triggered, this, &SLMainWindow::pasteContent);
    connect(m_actions[actAddImage], &QAction::triggered, this, &SLMainWindow::addImageFromLocalFile);
    connect(m_actions[actAddText], &QAction::triggered, this, [this](){
        addText();
    });
    connect(m_actions[actUndo], &QAction::triggered, this, &SLMainWindow::undo);
    connect(m_actions[actRedo], &QAction::triggered, this, &SLMainWindow::redo);
    connect(m_actions[actPortrait], &QAction::triggered, [this](){
        ui->graphicsView->setOrientation(Qt::Vertical);
    });
    connect(m_actions[actLandscape], &QAction::triggered, [this](){
        ui->graphicsView->setOrientation(Qt::Horizontal);
    });
    connect(m_actions[actPrint], &QAction::triggered, this, &SLMainWindow::print);
    connect(m_actions[actPrintPreview], &QAction::triggered, this, &SLMainWindow::printPreview);
    connect(m_actions[actShowButtonText], &QAction::triggered, this, &SLMainWindow::updateButtonsTextVisibility);
    connect(m_actions[actAbout], &QAction::triggered, this, &SLMainWindow::about);
    connect(m_actions[actPaperSize], &QAction::triggered, this, &SLMainWindow::setPaperSize);
    connect(m_actions[actExportImage], &QAction::triggered, this, &SLMainWindow::exportAsImage);
    // add all actions to main window
    addActions(m_actions.values());

}

void SLMainWindow::initMainMenu()
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
        connect( action, &QAction::triggered, this, &SLMainWindow::onLanguageActionTriggered);
    }

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_actions[actAbout]);
}

