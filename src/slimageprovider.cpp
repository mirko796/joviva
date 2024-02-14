#include "slimageprovider.h"
#include <QCryptographicHash>
#include <QBitmap>
#include <QBuffer>
SLImageProvider::SLImageProvider()
{

}

SLImageProvider::~SLImageProvider()
{
    clear();
}

SLImageProvider::PixmapId SLImageProvider::addPixmap(const QPixmap &pixmap)
{
    const QImage image = pixmap.toImage();
    const char* rawPtr = reinterpret_cast<const char*>(image.bits());
    const QByteArray raw(rawPtr, image.sizeInBytes());
    const auto hash = QCryptographicHash::hash(raw, QCryptographicHash::Md5);
    const auto hashString = hash.toHex();
    m_pixmaps.insert(hashString, new QPixmap(pixmap));
    return hashString;
}

const QPixmap* SLImageProvider::pixmap(const PixmapId &id, const bool transparentBackground) const
{
    const QPixmap* ret = m_pixmaps.value(id, nullptr);
    if (transparentBackground && ret) {
        ret = m_transparentPixmaps.value(id, nullptr);
        if (!ret) {
            QPixmap* pix = new QPixmap(*m_pixmaps.value(id));
            QImage img = pix->toImage();
            img.convertTo(QImage::Format_Grayscale8);
            // preserve only pixels with gray > threshold
            for (int y = 0; y < img.height(); ++y) {
                uchar* line = img.scanLine(y);
                for (int x = 0; x < img.width(); ++x) {

                    if (*line>200) {
                        *line=255;
                    }
                    ++line;
                }
            }
            // create a mask from the image
            QBitmap mask = QPixmap::fromImage(img).createMaskFromColor(Qt::white);
            pix->setMask(mask);
            m_transparentPixmaps.insert(id, pix);
            ret = pix;
        }
    }
    return ret;
}

QSet<SLImageProvider::PixmapId> SLImageProvider::pixmapIds() const
{
    const auto list = m_pixmaps.keys();
    return QSet<SLImageProvider::PixmapId>(list.begin(), list.end());
}

void SLImageProvider::clear()
{
    qDeleteAll(m_pixmaps);
    qDeleteAll(m_transparentPixmaps);
    m_pixmaps.clear();
    m_transparentPixmaps.clear();
}

QJsonObject SLImageProvider::asJson() const
{
    QJsonObject obj;
    QJsonObject pixObj;
    for (auto it = m_pixmaps.begin(); it != m_pixmaps.end(); ++it) {
        const QPixmap* pix = it.value();
        const QString key = it.key();
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pix->save(&buffer, "PNG");
        buffer.close();
        pixObj[key] = QString::fromLatin1(byteArray.toBase64());
    }
    obj[JK_IMAGES] = pixObj;
    return obj;
}

bool SLImageProvider::fromJson(const QJsonObject &obj)
{
    if (obj.contains(JK_IMAGES)==false) {
        return false;
    }
    const QJsonObject pixObj = obj[JK_IMAGES].toObject();
    for (auto it = pixObj.begin(); it != pixObj.end(); ++it) {
        const QString key = it.key();
        const QString base64 = it.value().toString();
        QByteArray byteArray = QByteArray::fromBase64(base64.toLatin1());
        QPixmap* pix = new QPixmap();
        pix->loadFromData(byteArray, "PNG");
        m_pixmaps.insert(key, pix);
    }
    return true;
}
