/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_liquify_transform_worker_test.h"

#include <qtest_kde.h>

#include <KoUpdater.h>

#include "testutil.h"

#include <kis_liquify_transform_worker.h>

#include <kis_cage_transform_worker.h>
#include <algorithm>

#include <kis_algebra_2d.h>


void testCage(bool clockwise, bool unityTransform, bool benchmarkPrepareOnly = false, int pixelPrecision = 8, bool testQImage = false)
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_cage_transform.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;

    QRectF bounds(dev->exactBounds());

    origPoints << bounds.topLeft();
    origPoints << 0.5 * (bounds.topLeft() + bounds.topRight());
    origPoints << 0.5 * (bounds.topLeft() + bounds.bottomRight());
    origPoints << 0.5 * (bounds.topRight() + bounds.bottomRight());
    origPoints << bounds.bottomRight();
    origPoints << bounds.bottomLeft();

    if (!clockwise) {
        std::reverse(origPoints.begin(), origPoints.end());
    }

    if (unityTransform) {
        transfPoints = origPoints;
    } else {
        transfPoints << bounds.topLeft();
        transfPoints << 0.5 * (bounds.topLeft() + bounds.topRight());
        transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight());
        transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) +
            (bounds.bottomLeft() - bounds.topLeft());
        transfPoints << bounds.bottomLeft() +
            (bounds.bottomLeft() - bounds.topLeft());
        transfPoints << bounds.bottomLeft();

        if (!clockwise) {
            std::reverse(transfPoints.begin(), transfPoints.end());
        }
    }

    KisCageTransformWorker worker(dev,
                                  origPoints,
                                  updater,
                                  pixelPrecision);

    QImage result;
    QPointF srcQImageOffset(0, 0);
    QPointF dstQImageOffset;

    QBENCHMARK_ONCE {
        if (!testQImage) {
            worker.prepareTransform();
            if (!benchmarkPrepareOnly) {
                worker.setTransformedCage(transfPoints);
                worker.run();

            }
        } else {
            QImage srcImage(image);
            image = QImage(image.size(), QImage::Format_ARGB32);
            QPainter gc(&image);
            gc.drawImage(QPoint(), srcImage);

            image.convertToFormat(QImage::Format_ARGB32);

            KisCageTransformWorker qimageWorker(image,
                                                srcQImageOffset,
                                                origPoints,
                                                updater,
                                                pixelPrecision);
            qimageWorker.prepareTransform();
            qimageWorker.setTransformedCage(transfPoints);
            result = qimageWorker.runOnQImage(&dstQImageOffset);
        }
    }

    QString testName = QString("%1_%2")
        .arg(clockwise ? "clk" : "cclk")
        .arg(unityTransform ? "unity" : "normal");

    if (testQImage) {
        QVERIFY(TestUtil::checkQImage(result, "cage_transform_test", "cage_qimage", testName));
    } else if (!benchmarkPrepareOnly && pixelPrecision == 8) {

        result = dev->convertToQImage(0);
        QVERIFY(TestUtil::checkQImage(result, "cage_transform_test", "cage", testName));
    }
}

void KisLiquifyTransformWorkerTest::testPoints()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    const int pixelPrecision = 8;

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);


    QBENCHMARK_ONCE {
        worker.translatePoints(QPointF(100,100),
                               QPointF(50, 0),
                               50, false, 0.2);

        worker.scalePoints(QPointF(400,100),
                           0.9,
                           50, false, 0.2);

        worker.undoPoints(QPointF(400,100),
                           1.0,
                           50);

        worker.scalePoints(QPointF(400,300),
                           0.5,
                           50, false, 0.2);

        worker.scalePoints(QPointF(100,300),
                           -0.5,
                           30, false, 0.2);

        worker.rotatePoints(QPointF(100,500),
                            M_PI / 4,
                            50, false, 0.2);
    }

    worker.run(dev);

    QImage result = dev->convertToQImage(0);
    TestUtil::checkQImage(result, "liquify_transform_test", "liquify_dev", "unity");
}

void KisLiquifyTransformWorkerTest::testPointsQImage()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    const int pixelPrecision = 8;

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);


    worker.translatePoints(QPointF(100,100),
                           QPointF(50, 0),
                           50, false, 0.2);

    QRect rc = dev->exactBounds();
    dev->setX(50);
    dev->setY(50);
    worker.run(dev);
    rc |= dev->exactBounds();

    QImage resultDev = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    TestUtil::checkQImage(resultDev, "liquify_transform_test", "liquify_qimage", "refDevice");

    QTransform imageToThumbTransform =
        QTransform::fromScale(0.5, 0.5);

    QImage srcImage(image);
    image = QImage(image.size(), QImage::Format_ARGB32);
    QPainter gc(&image);
    gc.setTransform(imageToThumbTransform);
    gc.drawImage(QPoint(), srcImage);

    QPointF newOffset;
    QImage result = worker.runOnQImage(image, QPointF(10, 10), imageToThumbTransform, &newOffset);
    qDebug() << ppVar(newOffset);


    TestUtil::checkQImage(result, "liquify_transform_test", "liquify_qimage", "resultImage");
}

QTEST_KDEMAIN(KisLiquifyTransformWorkerTest, GUI)
