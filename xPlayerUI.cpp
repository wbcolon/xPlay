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


