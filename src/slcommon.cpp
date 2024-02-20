#include "slcommon.h"
#include <math.h>

namespace SL
{

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

}
