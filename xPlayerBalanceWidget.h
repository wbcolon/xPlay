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

#include <qwt/qwt_slider.h>

class xPlayerBalanceWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerBalanceWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerBalanceWidget() override = default;
    /**
     * Setup the range allowed for balance.
     *
     * The allowed balance will be from -range to +range.
     *
     * @param range the maximal range value as integer.
     */
    void setRange(int range);
    /**
     * Retrieve the current balance range.
     *
     * @return the maximal range value as integer.
     */
    int getRange();
    /**
     * Set the current balance.
     *
     * @param value the current balance as integer.
     */
    void setBalance(int value);
    /**
     * Retrieve the current balance.
     *
     * @return the current balance as integer.
     */
    int getBalance();

signals:
    /**
     * Signal emitted whenever the balance is changed.
     *
     * @param value the current balance as integer.
     */
    void balance(int value);

private slots:
    /**
     * Update the slider and the corresponding labels.
     *
     * @param value the new balance as integer.
     */
    void updatedSlider(int value);

private:
    /**
     * Update the slider and labels widget on balance changes.
     *
     * @param value the new balance as integer.
     */
    void updateSlider(int value);
    /**
     * Update the slider and labels widget on range changes.
     *
     * @param value the new range value as integer.
     */
    void updateSliderRange(int value);

    QwtSlider* balanceSlider;
    QGridLayout* balanceLayout;
    QLabel* balanceLeftLabel;
    QLabel* balanceRightLabel;
    int balanceRange;
    int balanceValue;
};

#endif
