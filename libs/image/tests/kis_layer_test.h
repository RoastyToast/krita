/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_LAYER_TESTER_H
#define KIS_LAYER_TESTER_H

#include <QtTest>

#include "kis_layer.h"
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_image.h"

class TestLayer : public KisLayer
{

    Q_OBJECT

public:

    TestLayer(KisImageWSP image, const QString & name, quint8 opacity)
            : KisLayer(image, name, opacity) {
    }

    KisNodeSP clone() {
        return new TestLayer(*this);
    }
    bool allowAsChild(KisNodeSP) const override {
        return true;
    }

    virtual QString nodeType() {
        return "TEST";
    }

    KisPaintDeviceSP original() const override {
        // This test doesn't use updateProjection so just return 0
        return 0;
    }

    KisPaintDeviceSP paintDevice() const override {
        return 0;
    }

    QIcon icon() const override {
        return QIcon();
    }

    KisNodeSP clone() const override {
        return new TestLayer(image(), name(), opacity());
    }

    qint32 x() const override {
        return 0;
    }

    void setX(qint32) override {
    }

    qint32 y() const override {
        return 0;
    }

    void setY(qint32) override {
    }

    QRect extent() const override {
        return QRect();
    }

    QRect exactBounds() const override {
        return QRect();
    }

    using KisLayer::accept;

    bool accept(KisNodeVisitor& v) override {
        return v.visit(this);
    }


};

class KisLayerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testOrdering();
    void testMoveNode();
    void testMoveLayer();
    void testMasksChangeRect();
    void testMoveLayerWithMaskThreaded();
};

#endif

