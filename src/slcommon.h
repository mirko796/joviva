#ifndef SLCOMMON_H
#define SLCOMMON_H
#include <QString>
#include <QFont>
#include <QColor>
#include <QJsonObject>

#define JSONKEY static constexpr char const* const
namespace SL
{
constexpr int DefaultFontSize = 128;
constexpr char DefaultExtension[] = "ji";
constexpr char SettingsKeyShowButtonTexts[] = "showbuttontexts";
constexpr char SettingsKeyLanguage[] = "language";
struct TextParams
{
    JSONKEY JK_TEXT = "text";
    JSONKEY JK_FONT = "font";
    JSONKEY JK_COLOR = "color";
    JSONKEY JK_HOLLOW = "hollow";
    JSONKEY JK_ALIGNMENT = "alignment";

    QString text;
    QFont   font;
    QColor  color = Qt::black;
    bool    hollow = false;
    Qt::Alignment alignment;
    TextParams() = default;
    TextParams(const QString& text, const QFont& font, const QColor& color = Qt::black, Qt::Alignment alignment = Qt::AlignCenter)
        : text(text), font(font), color(color), alignment(alignment)
    {

    }
    QJsonObject asJson() const;
    bool    fromJson(const QJsonObject& obj);

    bool operator==(const TextParams& other) const;
};

double normalizedAngle(const double angle);
QString defaultFileFilter();
}
#endif // SLCOMMON_H
