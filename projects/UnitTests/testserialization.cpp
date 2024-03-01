#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "utest.h"
#pragma GCC diagnostic pop
#include "jigraphicspixmapitem.h"
#include "jigraphicstextitem.h"
#include "jigraphicsview.h"
#include <QJsonDocument>
UTEST(Serialization,pixmapitem)
{
    QImage img(100,100,QImage::Format_ARGB32);
    img.fill(Qt::red);
    QPixmap pixmap = QPixmap::fromImage(img);
    JIImageProvider provider;
    const auto pixId = provider.addPixmap(pixmap);
    JIGraphicsPixmapItem item(&provider);
    item.setRect(QRectF(50,50,100,100));
    item.setPixmapId(pixId);
    item.setRotation(-62.223489647253174);
    item.setSortOrder(23);

    const QJsonObject obj1 = item.asJson();
    ASSERT_FALSE(obj1.isEmpty());
    const int type = obj1[JIGraphicsItem::JK_TYPE].toInt();
    ASSERT_EQ(
        JIGraphicsPixmapItem::Type, //expected
        type
        );

    QJsonDocument doc(obj1);
    qDebug().noquote()<<doc.toJson();

    JIGraphicsPixmapItem item2(&provider);
    ASSERT_TRUE(item2.fromJson(obj1));
    ASSERT_EQ(
        JI::normalizedAngle(item2.rotation()),
        JI::normalizedAngle(item.rotation())
        );
    ASSERT_TRUE(item2.sortOrder()==item.sortOrder());
    ASSERT_TRUE(item2.rect()==item.rect());
    ASSERT_TRUE(item2.pixmapId()==item.pixmapId());
}

UTEST(Serialization,textparams)
{
    JI::TextParams params;
    params.font.setFamily("Arial");
    params.font.setPointSize(12);
    params.font.setBold(true);
    params.font.setItalic(true);
    params.font.setUnderline(true);
    params.font.setStrikeOut(true);
    params.color = Qt::red;
    params.alignment = Qt::AlignCenter;
    params.hollow = true;
    params.text = "Hello world from unit test";
    const QJsonObject obj1 = params.asJson();
    ASSERT_FALSE(obj1.isEmpty());
    QJsonDocument doc(obj1);
    qDebug().noquote()<<doc.toJson();
    JI::TextParams params2;
    ASSERT_TRUE(params2.fromJson(obj1));
    ASSERT_TRUE(params==params2);
    QJsonDocument doc2(params.asJson());
    ASSERT_TRUE(doc==doc2);
    params.hollow = false;
    QJsonDocument doc3(params.asJson());
    ASSERT_FALSE(doc==doc3);

}

UTEST(Serialization,textitem)
{
    JIGraphicsTextItem item;
    item.setRect(QRectF(150,150,100,100));
    JI::TextParams params;
    params.font.setFamily("Arial");
    params.font.setPointSize(12);
    params.font.setBold(true);
    params.font.setItalic(true);
    params.font.setUnderline(true);
    params.font.setStrikeOut(true);
    params.color = Qt::red;
    params.alignment = Qt::AlignCenter;
    params.text = "Hello";
    item.setTextParams(params);
    item.setRotation(-62.223489647253174);
    item.setSortOrder(23);

    const QJsonObject obj1 = item.asJson();
    ASSERT_FALSE(obj1.isEmpty());
    const int type = obj1[JIGraphicsItem::JK_TYPE].toInt();
    ASSERT_EQ(
        JIGraphicsTextItem::Type, //expected
        type
    );

    JIGraphicsTextItem item2;
    ASSERT_TRUE(item2.fromJson(obj1));
    ASSERT_EQ(JI::normalizedAngle(item2.rotation()),
              JI::normalizedAngle(item.rotation()));
    ASSERT_TRUE(item2.sortOrder()==item.sortOrder());
    ASSERT_TRUE(item2.rect().toRect()==item.rect().toRect());
    ASSERT_TRUE(item2.textParams()==item.textParams());
}

UTEST(Serialization,imageprovider)
{
    const QStringList testImages({
        ":/app-icon.png",
        ":/rotate-icon.png"
    });
    JIImageProvider provider;
    for (const QString& imgPath : testImages)
    {
        QPixmap pix(imgPath);
        ASSERT_FALSE(pix.isNull());
        const auto pixId = provider.addPixmap(pix);
        ASSERT_FALSE(pixId.isEmpty());
    }
    ASSERT_GT(provider.count(),1);
    const QJsonObject obj1 = provider.asJson();
    ASSERT_TRUE(obj1.contains(JIImageProvider::JK_IMAGES));
    ASSERT_TRUE(obj1[JIImageProvider::JK_IMAGES].toObject().count()==provider.count());

    JIImageProvider provider2;
    ASSERT_TRUE(provider2.fromJson(obj1));
    ASSERT_TRUE(provider2.pixmapIds()==provider.pixmapIds());
    // check individual pixmaps
    foreach (const auto& pixId , provider.pixmapIds())
    {
        const QPixmap* pix1 = provider.pixmap(pixId);
        const QPixmap* pix2 = provider2.pixmap(pixId);
        ASSERT_TRUE(pix1!=nullptr);
        ASSERT_TRUE(pix2!=nullptr);
        ASSERT_TRUE(pix1->size()==pix2->size());
        ASSERT_TRUE(pix1->toImage()==pix2->toImage());
    }

    ASSERT_TRUE(provider2.asJson()==provider.asJson());
    provider2.clear();
    ASSERT_FALSE(provider2.asJson()==provider.asJson());
}

UTEST(Serialization,graphicsview)
{
    const QStringList testImages({
        ":/app-icon.png",
        ":/rotate-icon.png"
    });
    JIGraphicsView view;
    for (const QString& imgPath : testImages)
    {
        QPixmap pix(imgPath);
        ASSERT_FALSE(pix.isNull());
        view.addPixmapItem(pix);
    }
    JI::TextParams params;
    params.font.setFamily("Arial");
    params.font.setPointSize(12);
    params.font.setBold(true);
    params.text = "Hello";
    view.addTextItem(params);
    view.setOrientation(Qt::Horizontal);
    view.setPaperSize(QSize(210,297)*2);

    const QJsonObject obj = view.asJson(JIGraphicsView::jfItemsAndImages);
    ASSERT_FALSE(obj.isEmpty());
    ASSERT_TRUE(obj.contains(JIGraphicsView::JK_ITEMS));
    ASSERT_TRUE(obj.contains(JIGraphicsView::JK_PAPER_WIDTH));
    ASSERT_TRUE(obj.contains(JIGraphicsView::JK_PAPER_HEIGHT));
    ASSERT_TRUE(obj.contains(JIGraphicsView::JK_ORIENTATION));
    ASSERT_TRUE(obj.contains(JIGraphicsView::JK_PROVIDER));

    JIGraphicsView view2;
    ASSERT_TRUE(view2.fromJson(obj, JIGraphicsView::jfItemsAndImages));
    ASSERT_TRUE(view2.paperSize()==view.paperSize());
    ASSERT_TRUE(view2.orientation()==view.orientation());
    ASSERT_EQ(view2.itemsCount(),view.itemsCount());
}
