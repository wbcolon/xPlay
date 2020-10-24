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
#include "xPlayerSliderWidget.h"

xPlayerSliderWidget::xPlayerSliderWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags) {
}

QString xPlayerSliderWidget::millisecondsToLabel(qint64 ms) {
    return QString("%1:%2.%3").arg(ms/60000).arg((ms/1000)%60, 2, 10, QChar('0')).
            arg(((ms%1000)+5)/10, 2, 10, QChar('0'));
}

int xPlayerSliderWidget::determineScaleDivider(int length, int sections) {
    for (auto scaleDivider : { 10000, 30000, 60000, 120000, 300000, 600000, 1200000 }) {
        if ((length / scaleDivider) <= sections) {
            return scaleDivider;
        }
    }
    return 1200000;
}
