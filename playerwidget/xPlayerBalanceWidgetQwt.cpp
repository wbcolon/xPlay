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

#include "xPlayerBalanceWidgetQwt.h"
#include <qwt/qwt_scale_draw.h>
#include <QApplication>

xPlayerBalanceWidgetQwt::xPlayerBalanceWidgetQwt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerBalanceWidget(parent, flags) {
    balanceSlider = new QwtSlider(Qt::Horizontal, this);
    balanceSlider->setValue(0);
    balanceSlider->setScaleStepSize(1.0);
    balanceSlider->setStepAlignment(true);
    balanceSlider->setScalePosition(QwtSlider::LeadingScale);
    balanceSlider->setGroove(false);
    balanceSlider->setTrough(false);
    balanceSlider->setHandleSize(QSize(12, 16));
    balanceSlider->setSpacing(2);
    balanceSlider->setSingleSteps(1);
    auto* balanceScaleDraw = new QwtScaleDraw();
    balanceScaleDraw->enableComponent(QwtAbstractScaleDraw::Labels, false);
    balanceSlider->setScaleDraw(balanceScaleDraw);
    balanceLayout->addWidget(balanceSlider, 0, 1, 1, 2);
    // Connect slider movements to updatedSlider slot.
    connect(balanceSlider, &QwtSlider::valueChanged, [this](double value) { updatedSlider(static_cast<int>(value)); });
}

void xPlayerBalanceWidgetQwt::updateSliderRange(int value) {
    balanceSlider->setLowerBound(-value);
    balanceSlider->setUpperBound(value);
    balanceSlider->setScaleStepSize(value);
    balanceLeftLabel->setFixedWidth(QApplication::fontMetrics().width(QString::number(value)));
    balanceRightLabel->setFixedWidth(QApplication::fontMetrics().width(QString::number(value)));
}

void xPlayerBalanceWidgetQwt::updateSlider(int value) {
    balanceSlider->setValue(static_cast<double>(value));
}