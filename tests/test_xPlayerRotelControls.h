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

#include <QtTest>
#include <QtTestWidgets>
#include <QMetaType>


class test_xPlayerRotelControls:public QObject {
    Q_OBJECT

private slots:
    void testCleanupReplyMessage_data();
    void testCleanupReplyMessage();
};
