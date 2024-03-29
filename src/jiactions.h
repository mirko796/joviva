#ifndef JIACTIONS_H
#define JIACTIONS_H
#include <QAction>

namespace JIActions
{
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
    actExportImage,
    actDuplicateObject,
    actBigIcons,
    actSmallIcons,
    actCreateDesktopIcon
};
void    initActions();
QAction* getAction(Action action);
};

#endif // JIACTIONS_H
