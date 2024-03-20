#include "jimainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName("JovIva");
    a.setApplicationName("JovIva");
    a.setApplicationVersion(APP_VERSION);
    qDebug()<<"==== Starting application ===";
    qDebug()<<"Version:"<<APP_VERSION<<" ["<<GIT_COMMIT<<"]  Built on:"<<__DATE__<<__TIME__;
    a.setWindowIcon(QIcon(":/app-icon.png"));
    JIMainWindow::Translators translators;
    auto serbian = QSharedPointer<QTranslator>(new QTranslator());
    if (serbian->load(":/translations/srpski.qm"))
    {
        translators["Srpski"] = serbian;
    }
    translators["English"].reset(new QTranslator());
//    const QStringList uiLanguages = QLocale::system().uiLanguages();
//    for (const QString &locale : uiLanguages) {
//        const QString baseName = "SimpleLayout_" + QLocale(locale).name();
//        if (translator.load(":/i18n/" + baseName)) {
//            a.installTranslator(&translator);
//            break;
//        }
//    }
    QSettings settings("JovIva");
    JIMainWindow w(&settings,translators);
    w.show();
    // load from first param if provided
    if (argc > 1)
    {
        w.loadFile(argv[1]);
    }
    return a.exec();
}
