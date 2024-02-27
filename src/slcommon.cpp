#include "slcommon.h"
#include <math.h>
#include <QFile>
#include <QFileDialog>
#include <QCoreApplication>
namespace SL
{

static QHash<PaperFormat, PaperFormatInfo> g_paperFormatInfos;

static QHash<PaperFormat, PaperFormatInfo>& getPaperFormatInfos()
{
    if (g_paperFormatInfos.isEmpty()) {
        auto add = [](const PaperFormat pf, const QSize& size, const QString& name) {
            g_paperFormatInfos[pf] = PaperFormatInfo(pf, size, name);
        };
        add(psFreeform, QSize(0, 0), "Freeform");
        add(psA4, QSize(210, 297), "A4");
        add(psB4, QSize(250, 353), "B4");
        add(psLetter, QSize(216, 279), "Letter");
        add(psLegal, QSize(216, 356), "Legal");
        add(psSquare, QSize(210, 210), "Square");
    }
    return g_paperFormatInfos;
}
QList<PaperFormat> getPaperFormats()
{
    return getPaperFormatInfos().keys();
}

PaperFormatInfo getPaperFormatInfo(const PaperFormat paperFormat)
{
    return getPaperFormatInfos().value(paperFormat);
};

QJsonObject TextParams::asJson() const
{
    QJsonObject ret;
    ret[JK_TEXT] = text;
    ret[JK_FONT] = font.toString();
    ret[JK_COLOR] = color.name();
    ret[JK_HOLLOW] = hollow;
    ret[JK_ALIGNMENT] = static_cast<int>(alignment);
    return ret;
}

bool TextParams::fromJson(const QJsonObject &obj)
{
    static QList<QString> requiredKeys({
                                           JK_TEXT,
                                           JK_FONT,
                                           JK_COLOR,
                                           JK_HOLLOW,
                                           JK_ALIGNMENT
                                       });
    for (const QString& key : requiredKeys)
    {
        if (!obj.contains(key))
            return false;
    }
    text = obj[JK_TEXT].toString();
    font.fromString(obj[JK_FONT].toString());
    color = QColor(obj[JK_COLOR].toString());
    hollow = obj[JK_HOLLOW].toBool();
    alignment = static_cast<Qt::Alignment>(obj[JK_ALIGNMENT].toInt());
    return true;
}

bool TextParams::operator==(const TextParams &other) const
{
    return text == other.text &&
           font == other.font &&
           color == other.color &&
           hollow == other.hollow &&
           alignment == other.alignment;
}

double normalizedAngle(const double angle)
{
    double ret=fmod(angle, 360.0);

    // Make sure the result is non-negative
    if (ret < 0.0) {
        ret += 360.0;
    }
    return ret;
}

QString defaultFileFilter()
{
    return QString("JovIva files (*.%1)").arg(DefaultExtension);
}

void showQuestionAsync(const QString& title, const QString& text, const QHash<QMessageBox::Button, std::function<void()> > buttonsAndCallbacks, const QMessageBox::Button defaultButton) {
    QMessageBox* msgBox = new QMessageBox();
    msgBox->setText(text);
    msgBox->setWindowTitle(title);
    QMessageBox::StandardButtons buttons;
    for (auto button: buttonsAndCallbacks.keys()) {
        buttons|=button;
    }
    msgBox->setStandardButtons(buttons);
    msgBox->setDefaultButton(defaultButton);
    msgBox->setIcon(QMessageBox::Question);
    QObject::connect(msgBox, &QMessageBox::finished, [msgBox, buttonsAndCallbacks](int result) {
        const auto btn = QMessageBox::StandardButton(result);
        auto callback = buttonsAndCallbacks.value(btn);
        if (callback) {
            callback();
        }
        msgBox->deleteLater();
    });
    msgBox->setModal(true);
    msgBox->show();
    msgBox->adjustSize();
}

QSize normalizeSize(const QSize s)
{
    return QSize(qMin(s.width(), s.height()), qMax(s.width(), s.height()));
}

void showOpenFileDialog(QWidget *parent,
              const QString &caption,
              const QString &dir,
              const QString &filter,
              const std::function<void(const QString&, const QByteArray&)>& fileContentReady)
{
#ifdef Q_OS_WASM
    QFileDialog::getOpenFileContent(filter, fileContentReady);
#else
    const QString filename = QFileDialog::getOpenFileName(parent, caption,
                                                          dir,
                                                          filter);
    if (filename.isEmpty())
        return;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(parent, qApp->applicationName(), QObject::tr("Failed to open file %1 for reading").arg(filename));
        return;
    }
    fileContentReady(filename, file.readAll());
#endif
}

void showSaveFileDialog(QWidget *parent,
                        const QString &caption,
                        const QString &dir,
                        const QString &filter,
                        const QString &fileHint,
                        const QString &defaultExtension,
                        const QByteArray &fileContent,
                        std::function<void(const QString& filename)> onSaved)
{
#ifdef Q_OS_WASM
    Q_UNUSED(onSaved);
    QFileDialog::saveFileContent(fileContent, fileHint);
#else
    Q_UNUSED(fileHint);
    QString filename = QFileDialog::getSaveFileName(parent, caption,
                                                    dir,
                                                    filter);
    if (filename.isEmpty())
        return;
    if (!filename.endsWith(defaultExtension)) {
        filename+=QString(".%1").arg(SL::DefaultExtension);
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(parent, qApp->applicationName(), QObject::tr("Failed to open file %1 for writing").arg(filename));
        return;
    }
    file.write(fileContent);
    file.close();
    if (onSaved) {
        onSaved(filename);
    }
#endif
}

}
