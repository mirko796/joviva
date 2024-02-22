#ifndef SLCOMMON_H
#define SLCOMMON_H
#include <QString>
#include <QFont>
#include <QColor>
#include <QJsonObject>
#include <QMessageBox>

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
enum PaperFormat {
    psFreeform,
    psA4,
    psB4,
    psLetter,
    psLegal,
    psSquare
};

struct PaperFormatInfo
{
    PaperFormat paperFormat;
    QSize sizeInMM;
    QString name;
    PaperFormatInfo() = default;
    PaperFormatInfo(const PaperFormat pf, const QSize& sizeInMM, const QString& name)
        : paperFormat(pf), sizeInMM(sizeInMM), name(name)
    {

    }
};
QList<PaperFormat> getPaperFormats();
PaperFormatInfo getPaperFormatInfo(const PaperFormat paperFormat);

struct DocumentSize
{
    PaperFormat paperFormat;
    QSize   sizeInPixels;
    Qt::Orientation orientation;
    DocumentSize() = default;
    DocumentSize(const PaperFormat pf, const QSize& sizeInPixels, const Qt::Orientation orientation)
        : paperFormat(pf), sizeInPixels(sizeInPixels), orientation(orientation)
    {

    }
};

double normalizedAngle(const double angle);
QString defaultFileFilter();

void showQuestionAsync(const QString& title, const QString& text, const QHash<QMessageBox::Button, std::function<void()> > buttonsAndCallbacks, const QMessageBox::Button defaultButton=QMessageBox::NoButton);
}
#endif // SLCOMMON_H
