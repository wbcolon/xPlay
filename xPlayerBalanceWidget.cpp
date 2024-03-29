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

#include "xPlayerBalanceWidget.h"

#include <QApplication>
#include <QPaintEvent>
#include <QPainter>

xPlayerBalanceWidget::xPlayerBalanceWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        balanceRange(0),
        balanceValue(0) {
    balanceLeftLabel = new QLabel();
    balanceLeftLabel->setAlignment(Qt::AlignLeft);
    balanceRightLabel = new QLabel();
    balanceRightLabel->setAlignment(Qt::AlignRight);
    balanceLayout = new QGridLayout();
    balanceLayout->addWidget(balanceLeftLabel, 0, 0, 1, 1);
    balanceLayout->addWidget(balanceRightLabel, 0, 3, 1, 1);
    balanceLayout->setColumnStretch(1, 2);
    balanceLayout->setColumnStretch(2, 2);
    balanceLayout->setSpacing(0);
    balanceSlider = new QSlider(Qt::Horizontal, this);
    balanceSlider->setValue(0);
    balanceSlider->setTickPosition(QSlider::NoTicks);
    balanceLayout->addWidget(balanceSlider, 0, 1, 1, 2);
    setLayout(balanceLayout);
    // Connect slider movements to updatedSlider slot.
    connect(balanceSlider, &QSlider::valueChanged, this, &xPlayerBalanceWidget::updatedSlider);
    setFixedHeight(48);
}

void xPlayerBalanceWidget::updateSliderRange(int value) {
    balanceSlider->setMinimum(-value);
    balanceSlider->setMaximum(value);
    balanceSlider->setTickInterval(value);
    balanceLeftLabel->setFixedWidth(QApplication::fontMetrics().size(Qt::TextSingleLine, QString::number(value)).width());
    balanceRightLabel->setFixedWidth(QApplication::fontMetrics().size(Qt::TextSingleLine, QString::number(value)).width());
    // Set initial labels after the range has been set. The actual value will be set later on.
    updatedSlider(0);
}

void xPlayerBalanceWidget::setBalance(int value) {
    balanceValue = std::clamp(value, -balanceRange, balanceRange);
    updateSlider(balanceValue);
}

int xPlayerBalanceWidget::getBalance() {
    return balanceValue;
}
void xPlayerBalanceWidget::setRange(int range) {
    balanceRange = range;
    updateSliderRange(range);
}

int xPlayerBalanceWidget::getRange() {
    return balanceRange;
}

void xPlayerBalanceWidget::updatedSlider(int value) {
    balanceValue = std::clamp(value, -balanceRange, balanceRange);
    balanceLeftLabel->clear();
    if (balanceValue <= 0) {
        balanceLeftLabel->setText(QString::number(-balanceValue));
    }
    balanceRightLabel->clear();
    if (balanceValue >= 0) {
        balanceRightLabel->setText(QString::number(balanceValue));
    }
    emit balance(balanceValue);
}

void xPlayerBalanceWidget::updateSlider(int value) {
    balanceSlider->setValue(value);
}