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

#include "xPlayerUI.h"

namespace xPlayer {

QString millisecondsToTimeFormat(qint64 ms, bool showHours) {
    auto hours = ms/3600000;
    auto minutes = (ms/60000)%60;
    auto seconds = (ms/1000)%60;
    auto hseconds = (ms%1000)/10;
    if (showHours) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2.%3").arg(hours).arg(seconds, 2, 10, QChar('0')).arg(hseconds, 2, 10, QChar('0'));
    }
}

QString millisecondsToShortTimeFormat(qint64 ms) {
    auto hours = ms/3600000;
    auto minutes = (ms/60000)%60;
    auto seconds = (ms/1000)%60;
    // More than one hour
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
    }
}

}

xPlayerLayout::xPlayerLayout():QGridLayout() {
}

xPlayerLayout::xPlayerLayout(QWidget* parent):QGridLayout(parent) {
}

void xPlayerLayout::addRowSpacer(int row, int space) {
    setRowMinimumHeight(row, space);
    setRowStretch(row, 0);
}

void xPlayerLayout::addRowStretcher(int row) {
    setRowMinimumHeight(row, 0);
    setRowStretch(row, 2);
}

void xPlayerLayout::addColumnSpacer(int column, int space) {
    setColumnMinimumWidth(column, space);
    setColumnStretch(column, 0);
}

void xPlayerLayout::addColumnStretcher(int column) {
    setColumnMinimumWidth(column, 0);
    setColumnStretch(column, 2);
}


