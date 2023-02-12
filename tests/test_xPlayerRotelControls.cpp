/*
 * This file is part of xPlay.
 *
 * xPlay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * xPlay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "test_xPlayerRotelControls.h"
#include "xPlayerRotelControls.h"

#include <QMetaType>

#include <tuple>


void test_xPlayerRotelControls::testCleanupReplyMessage_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expectedOutput");

    QTest::newRow("only amp:freq") << "amp:freq=off$amp:freq=48K$amp:freq=off$amp:freq=48K$amp:freq=off" << QString();
    QTest::newRow("no amp:freq") << "amp:volume=23" << "amp:volume=23";
    QTest::newRow("embedded response") << "amp:freq=48K$amp:volume=34" << "amp:volume=34";
}

void test_xPlayerRotelControls::testCleanupReplyMessage() {
    QFETCH(QString, input);
    QFETCH(QString, expectedOutput);

    auto processedInput = xPlayerRotelControls::cleanupReplyMessage(input);
    QVERIFY(processedInput == expectedOutput);
}

