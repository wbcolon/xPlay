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
#ifndef __XPLAYERROTELWIDGET_H__
#define __XPLAYERROTELWIDGET_H__

#include "xPlayerRotelControls.h"
#include "xPlayerVolumeWidget.h"
#include "xPlayerBalanceWidget.h"

#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QWidget>
#include <QString>


class xPlayerRotelWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerRotelWidget(QWidget* parent=nullptr, Qt::Orientation orientation=Qt::Horizontal, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerRotelWidget() override = default;

private slots:
    /**
     * Enable all widget elements of controls are connected.
     */
    void connected();
    /**
     * Disable all widget elements if controls loose connection.
     */
    void disconnected();
    /**
     * Update only the volume knobs position.
     *
     * @param vol the new volume as integer.
     */
    void updateVolume(int vol);
    /**
     * Update only the spinbox value.
     *
     * @param b the new bass as integer.
     */
    void updateBass(int b);
    /**
     * Update only the spinbox value.
     *
     * @param t the new treble as integer.
     */
    void updateTreble(int t);
    /**
     * Update only the slider value.
     *
     * @param b the new balance as integer.
     */
    void updateBalance(int b);
    /**
     * Update only the combo box containing the sources.
     *
     * @param source the new source as string.
     */
    void updateSource(const QString& source);
    /**
     * Update only the checkbox for the mute state.
     *
     * @param mute true if mute is enabled, false otherwise.
     */
    void updateMuted(bool mute);

private:
    xPlayerVolumeWidget* rotelVolume;
    xPlayerBalanceWidget* rotelBalance;
    QComboBox* rotelSource;
    QLabel* rotelBassLabel;
    QLabel* rotelTrebleLabel;
    QLabel* rotelBalanceLabel;
    QSpinBox* rotelBass;
    QSpinBox* rotelTreble;
    xPlayerRotelControls* rotelControls;
};

#endif
