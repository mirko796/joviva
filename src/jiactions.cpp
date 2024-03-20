#include "jiactions.h"
#include <QActionGroup>
namespace JIActions {
QHash<Action, QAction*> g_actions;
QActionGroup g_iconSizeGroup(nullptr);
QAction* getAction(Action action)
{
    if (g_actions.isEmpty()) {
        initActions();
    }
    return g_actions.value(action);
}

void initActions()
{
    auto createAction = [](Action act, const QString &text, const QKeySequence &shortcut=QKeySequence(), const QString& icon=QString()) {
        auto action = new QAction(text, nullptr);
        action->setToolTip(text);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::ApplicationShortcut);
        if (icon.size()) {
            action->setIcon(QIcon(icon));
        }
        g_actions[act] = action;
    };

    createAction(
        actNew,
        QObject::tr("New"),
        QKeySequence::New,
        ":/new-icon.png");

    createAction(
        actOpen,
        QObject::tr("Open"),
        QKeySequence::Open,
        ":/open-icon.png");

    createAction(
        actSave,
        QObject::tr("Save"),
        QKeySequence::Save,
        ":/save-icon.png");

    createAction(
        actSaveAs,
        QObject::tr("Save As"),
        QKeySequence::SaveAs,
        ":/saveas-icon.png");
    createAction(
        actPaste,
        QObject::tr("Paste"),
#ifdef Q_OS_WASM
        /* handled by JS part */
        QKeySequence(),
#else
        QKeySequence::Paste,
#endif
        ":/paste-icon.png");

    createAction(
        actAddImage,
        QObject::tr("Add Image"),
        QKeySequence(QObject::tr("Ctrl+I")),
        ":/add-image-icon.png");

    createAction(
        actAddText,
        QObject::tr("Add Text"),
        QKeySequence(QObject::tr("Ctrl+T")),
        ":/add-text-icon.png");

    createAction(
        actUndo,
        QObject::tr("Undo"),
        QKeySequence::Undo,
        ":/undo-icon.png");

    createAction(
        actRedo,
        QObject::tr("Redo"),
        QKeySequence::Redo,
        ":/redo-icon.png");

    createAction(
        actPortrait,
        QObject::tr("Portrait"),
        QKeySequence(),
        ":/portrait-icon.png");
    g_actions[actPortrait]->setCheckable(true);

    createAction(
        actLandscape,
        QObject::tr("Landscape"),
        QKeySequence(),
        ":/landscape-icon.png");
    g_actions[actLandscape]->setCheckable(true);

    createAction(
        actPrint,
        QObject::tr("Print"),
        QKeySequence(),
        ":/print-icon.png");

    createAction(
        actPrintPreview,
        QObject::tr("Print Preview"),
        QKeySequence(QObject::tr("Ctrl+P")),
        ":/print-preview-icon.png");

    createAction(
        actShowButtonText,
        QObject::tr("Show Buttons Text"));
    g_actions[actShowButtonText]->setCheckable(true);

    createAction(
        actAbout,
        QObject::tr("About"));

    createAction(
        actPaperSize,
        QObject::tr("Set Paper Size"),
        QKeySequence(),
        ":/paper-size-icon.png");

    createAction(
        actExportImage,
        QObject::tr("Export As Image"),
        QKeySequence(),
        ":/add-image-icon.png");

    createAction(
        actDuplicateObject,
        QObject::tr("Duplicate"));

    createAction(
        actBigIcons,
        QObject::tr("Big Icons"));
    g_actions[actBigIcons]->setCheckable(true);

    createAction(
        actSmallIcons,
        QObject::tr("Small Icons"));
    g_actions[actSmallIcons]->setCheckable(true);
    g_iconSizeGroup.addAction(g_actions[actSmallIcons]);
    g_iconSizeGroup.addAction(g_actions[actBigIcons]);

    createAction(
        actCreateDesktopIcon,
        QObject::tr("Create Desktop Icon"));

}
}
