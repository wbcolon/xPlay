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

#ifndef __XPLAYERBALANCEWIDGET_H__
#define __XPLAYERBALANCEWIDGET_H__

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QString>

class xPlayerBalanceWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerBalanceWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerBalanceWidget() override = default;

    void setRange(int range);
    int getRange();
    void setBalance(int value);
    int getBalance();

signals:
    void balance(int value);

protected slots:
    void updatedSlider(int value);

protected:
    virtual void updateSlider(int value) = 0;
    virtual void updateSliderRange(int value) = 0;

    QGridLayout* balanceLayout;
    QLabel* balanceLeftLabel;
    QLabel* balanceRightLabel;
    int balanceRange;
    int balanceValue;
};

#endif
