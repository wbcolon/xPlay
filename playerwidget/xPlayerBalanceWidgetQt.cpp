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

#include "xPlayerBalanceWidgetQt.h"
#include <QApplication>

xPlayerBalanceWidgetQt::xPlayerBalanceWidgetQt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerBalanceWidget(parent, flags) {
    balanceSlider = new QSlider(Qt::Horizontal, this);
    balanceSlider->setTickPosition(QSlider::NoTicks);
    balanceSlider->setValue(0);
    balanceLayout->addWidget(balanceSlider, 0, 1, 1, 2);
    // Connect slider movements to updatedSlider slot.
    connect(balanceSlider, &QSlider::valueChanged, this, &xPlayerBalanceWidgetQt::updatedSlider);
    // Initial values.
    updatedSlider(0);
}

void xPlayerBalanceWidgetQt::updateSliderRange(int value) {
    balanceSlider->setRange(-value, value);
    balanceSlider->setSingleStep(1);
    balanceLeftLabel->setFixedWidth(QApplication::fontMetrics().width(QString::number(value)));
    balanceRightLabel->setFixedWidth(QApplication::fontMetrics().width(QString::number(value)));
}

void xPlayerBalanceWidgetQt::updateSlider(int value) {
    balanceSlider->setValue(value);
}