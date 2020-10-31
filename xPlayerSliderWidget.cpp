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
        QWidget(parent, flags),
        showHours(false) {
}

bool xPlayerSliderWidget::hourScale() {
    return showHours;
}

void xPlayerSliderWidget::useHourScale(bool hourScale) {
    showHours = hourScale;
}

void xPlayerSliderWidget::useScaleSections(int scaleSections) {
    maxScaleSections = scaleSections;
}

QString xPlayerSliderWidget::millisecondsToLabel(qint64 ms) {

    if (showHours) {
        return QString("%1:%2:%3.%4").arg(ms/3600000).
                arg((ms/60000)%60, 2, 10, QChar('0')).
                arg((ms/1000)%60, 2, 10, QChar('0')).
                arg(((ms%1000)+5)/10, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2.%3").arg(ms/60000).
                arg((ms/1000)%60, 2, 10, QChar('0')).
                arg(((ms%1000)+5)/10, 2, 10, QChar('0'));
    }
}

int xPlayerSliderWidget::determineScaleDivider(int length) {
    for (auto scaleDivider : { 10000, 30000, 60000, 120000, 300000, 600000, 1200000, 3000000, 6000000 }) {
        if ((length / scaleDivider) <= maxScaleSections) {
            return scaleDivider;
        }
    }
    return 6000000;
}
