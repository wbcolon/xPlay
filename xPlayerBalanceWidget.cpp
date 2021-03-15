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

xPlayerBalanceWidget::xPlayerBalanceWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
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
    setLayout(balanceLayout);
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
