#ifndef JICOMMON_H
#define JICOMMON_H
#include <QString>
#include <QFont>
#include <QColor>
#include <QJsonObject>
#include <QMessageBox>

#define JSONKEY static constexpr char const* const
namespace JI
{
constexpr int DefaultFontSize = 128;
constexpr char DefaultExtension[] = "ji";
constexpr char SettingsKeyShowButtonTexts[] = "showbuttontexts";
constexpr char SettingsKeyUseSmallIcons[] = "usesmallicons";
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
/**
 * @brief normalizeSize - make sure that width is smaller/equal to height
 * @param s
 * @return
 */
QSize normalizeSize(const QSize s);

class DocumentSize
{
    PaperFormat m_paperFormat;
    QSize   m_sizeInPixels;
    Qt::Orientation m_orientation;
public:
    DocumentSize() = default;
    DocumentSize(const PaperFormat pf, const QSize& sizeInPixels, const Qt::Orientation orientation)
        : m_paperFormat(pf), m_sizeInPixels(sizeInPixels), m_orientation(orientation)
    {

    }
    bool operator == (const DocumentSize& other) const
    {
        return m_paperFormat == other.m_paperFormat &&
               m_sizeInPixels == other.m_sizeInPixels &&
               m_orientation == other.m_orientation;
    }

    QString toString() const
    {
        return QString("%1 %2x%3 %4")
            .arg(getPaperFormatInfo(m_paperFormat).name)
            .arg(m_sizeInPixels.width())
            .arg(m_sizeInPixels.height())
            .arg(m_orientation == Qt::Orientation::Vertical ? "Portrait" : "Landscape");
    }
    PaperFormat paperFormat() const
    {
        return m_paperFormat;
    }
    void setPaperFormat(PaperFormat newPaperFormat)
    {
        m_paperFormat = newPaperFormat;
    }
    QSize sizeInPixels() const
    {
        return m_sizeInPixels;
    }
    void setSizeInPixels(const QSize &newSizeInPixels)
    {
        m_sizeInPixels = JI::normalizeSize(newSizeInPixels);
    }
    Qt::Orientation orientation() const
    {
        return m_orientation;
    }
    void setOrientation(Qt::Orientation newOrientation)
    {
        m_orientation = newOrientation;
    }
};


double normalizedAngle(const double angle);
QString defaultFileFilter();

void showQuestionAsync(const QString& title, const QString& text, const QHash<QMessageBox::Button, std::function<void()> > buttonsAndCallbacks, const QMessageBox::Button defaultButton=QMessageBox::NoButton);
void showOpenFileDialog(QWidget *parent,
                        const QString &caption,
                        const QString &dir,
                        const QString &filter,
                        const std::function<void(const QString&, const QByteArray&)>& fileContentReady);
void showSaveFileDialog(QWidget *parent,
                        const QString &caption,
                        const QString &dir,
                        const QString &filter,
                        const QString &fileHint,
                        const QString &defaultExtension,
                        const QByteArray &fileContent,
                        std::function<void(const QString& filename)> onSaved = nullptr);

}
#endif // JICOMMON_H
