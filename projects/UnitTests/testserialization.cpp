#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "utest.h"
#pragma GCC diagnostic pop
#include "slgraphicspixmapitem.h"
#include "slgraphicstextitem.h"
#include "slgraphicsview.h"
#include <QJsonDocument>
UTEST(Serialization,pixmapitem)
{
    QImage img(100,100,QImage::Format_ARGB32);
    img.fill(Qt::red);
    QPixmap pixmap = QPixmap::fromImage(img);
    SLImageProvider provider;
    const auto pixId = provider.addPixmap(pixmap);
    SLGraphicsPixmapItem item(&provider);
    item.setRect(QRectF(50,50,100,100));
    item.setPixmapId(pixId);
    item.setRotation(-62.223489647253174);
    item.setSortOrder(23);

    const QJsonObject obj1 = item.asJson();
    ASSERT_FALSE(obj1.isEmpty());
    const int type = obj1[SLGraphicsItem::JK_TYPE].toInt();
    ASSERT_EQ(
        SLGraphicsPixmapItem::Type, //expected
        type
        );

    QJsonDocument doc(obj1);
    qDebug().noquote()<<doc.toJson();

    SLGraphicsPixmapItem item2(&provider);
    ASSERT_TRUE(item2.fromJson(obj1));
    ASSERT_EQ(
        SL::normalizedAngle(item2.rotation()),
        SL::normalizedAngle(item.rotation())
        );
    ASSERT_TRUE(item2.sortOrder()==item.sortOrder());
    ASSERT_TRUE(item2.rect()==item.rect());
    ASSERT_TRUE(item2.pixmapId()==item.pixmapId());
}

UTEST(Serialization,textparams)
{
    SL::TextParams params;
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
    SL::TextParams params2;
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
    SLGraphicsTextItem item;
    item.setRect(QRectF(150,150,100,100));
    SL::TextParams params;
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
    const int type = obj1[SLGraphicsItem::JK_TYPE].toInt();
    ASSERT_EQ(
        SLGraphicsTextItem::Type, //expected
        type
    );

    SLGraphicsTextItem item2;
    ASSERT_TRUE(item2.fromJson(obj1));
    ASSERT_EQ(SL::normalizedAngle(item2.rotation()),
              SL::normalizedAngle(item.rotation()));
    ASSERT_TRUE(item2.sortOrder()==item.sortOrder());
    ASSERT_TRUE(item2.rect().toRect()==item.rect().toRect());
    ASSERT_TRUE(item2.textParams()==item.textParams());
}

UTEST(Serialization,imageprovider)
{
    const QStringList testImages({
        ":/creativity.png",
        ":/rotate-icon.png"
    });
    SLImageProvider provider;
    for (const QString& imgPath : testImages)
    {
        QPixmap pix(imgPath);
        ASSERT_FALSE(pix.isNull());
        const auto pixId = provider.addPixmap(pix);
        ASSERT_FALSE(pixId.isEmpty());
    }
    ASSERT_GT(provider.count(),1);
    const QJsonObject obj1 = provider.asJson();
    ASSERT_TRUE(obj1.contains(SLImageProvider::JK_IMAGES));
    ASSERT_TRUE(obj1[SLImageProvider::JK_IMAGES].toObject().count()==provider.count());

    SLImageProvider provider2;
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
        ":/creativity.png",
        ":/rotate-icon.png"
    });
    SLGraphicsView view;
    for (const QString& imgPath : testImages)
    {
        QPixmap pix(imgPath);
        ASSERT_FALSE(pix.isNull());
        view.addPixmapItem(pix);
    }
    SL::TextParams params;
    params.font.setFamily("Arial");
    params.font.setPointSize(12);
    params.font.setBold(true);
    params.text = "Hello";
    view.addTextItem(params);
    view.setOrientation(Qt::Horizontal);
    view.setPaperSize(QSize(210,297)*2);

    const QJsonObject obj = view.asJson(SLGraphicsView::jfItemsAndImages);
    ASSERT_FALSE(obj.isEmpty());
    ASSERT_TRUE(obj.contains(SLGraphicsView::JK_ITEMS));
    ASSERT_TRUE(obj.contains(SLGraphicsView::JK_PAPER_WIDTH));
    ASSERT_TRUE(obj.contains(SLGraphicsView::JK_PAPER_HEIGHT));
    ASSERT_TRUE(obj.contains(SLGraphicsView::JK_ORIENTATION));
    ASSERT_TRUE(obj.contains(SLGraphicsView::JK_PROVIDER));

    SLGraphicsView view2;
    ASSERT_TRUE(view2.fromJson(obj, SLGraphicsView::jfItemsAndImages));
    ASSERT_TRUE(view2.paperSize()==view.paperSize());
    ASSERT_TRUE(view2.orientation()==view.orientation());
    ASSERT_EQ(view2.itemsCount(),view.itemsCount());
}
