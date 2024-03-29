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
#include "xPlayerVolumeWidget.h"

#include "xPlayerUI.h"
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include <QLCDNumber>


xPlayerVolumeDial::xPlayerVolumeDial(QWidget *parent):
        QDial(parent) {
}

void xPlayerVolumeDial::paintEvent(QPaintEvent* event) {
    QDial::paintEvent(event);
    if (event) {
        QRect region = event->rect();
        QPainter scale(this);
        QLCDNumber number(3);  // No parent. We render into pixmap.
        number.resize(xPlayer::VolumeWidgetLCDWidth, xPlayer::VolumeWidgetLCDHeight);
        number.setSegmentStyle(QLCDNumber::Flat);
        number.setFrameStyle(QFrame::Panel | QFrame::Sunken);
        number.setAutoFillBackground(true);
        number.display(QString("%1").arg(value(), 3, 10, QChar('0')));
        // Use QDial highlight color for lcd background.
        QPalette numberPalette;
        numberPalette.setColor(QPalette::Window, palette().color(QPalette::Highlight));
        number.setPalette(numberPalette);
        // Render into pixmap.
        QPixmap numberPixmap(xPlayer::VolumeWidgetLCDWidth, xPlayer::VolumeWidgetLCDHeight);
        number.render(&numberPixmap);
        scale.setPen(Qt::SolidLine);
        // Determine position for pixmap.
        auto posX = (region.left() + region.right() - xPlayer::VolumeWidgetLCDWidth)/2;
        auto posY = (region.top() + region.bottom() - xPlayer::VolumeWidgetLCDHeight)/2;
        // Draw lcd pixmap into middle of dial.
        scale.drawPixmap(posX, posY, numberPixmap);
    }
}


xPlayerVolumeWidget::xPlayerVolumeWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        currentVolume(0),
        currentMuted(false) {
    auto volumeLayout = new xPlayerLayout(this);
    volumeKnob = new xPlayerVolumeDial(this);
    volumeKnob->setMinimum(0);
    volumeKnob->setMaximum(100);
    volumeKnob->setSingleStep(1);
    volumeKnob->setWrapping(false);
    volumeKnob->setContentsMargins(0, 0, 0, 0);
    volumeKnob->setNotchesVisible(true);
    auto volumePlusButton = new QPushButton(tr("+"), this);
    volumePlusButton->setFlat(true);
    volumePlusButton->setFixedWidth(xPlayerLayout::HugeSpace);
    auto volumeMinusButton = new QPushButton(tr("-"), this);
    volumeMinusButton->setFlat(true);
    volumeMinusButton->setFixedWidth(xPlayerLayout::HugeSpace);
    volumeMuteButton = new QPushButton(tr("Volume"), this);
    volumeMuteButton->setFlat(true);
    // Only stretch top and bottom.
    volumeLayout->addRowStretcher(0);
    // Qwt implementation. Layout here overlap on purpose
    volumeLayout->addWidget(volumeKnob, 1, 0, 3, 4);
    volumeLayout->addWidget(volumeMinusButton, 4, 0);
    volumeLayout->addWidget(volumeMuteButton, 4, 1, 1, 2);
    volumeLayout->addWidget(volumePlusButton, 4, 3);
    volumeLayout->addItem(new QSpacerItem(1, xPlayer::VolumeWidgetHeight), 3, 0, 1, 4);
    volumeLayout->addRowStretcher(5);
    // Connect the volume slider to the widgets signal. Use lambda to do proper conversion.
    connect(volumeKnob, &QDial::valueChanged, this, &xPlayerVolumeWidget::volumeChanged);
    connect(volumeMuteButton, &QPushButton::pressed, this, &xPlayerVolumeWidget::toggleMuted);
    connect(volumeMinusButton, &QPushButton::pressed, [=]() {
        if (currentVolume > 0) {
            setVolume(currentVolume-1);
        }
    } );
    connect(volumePlusButton, &QPushButton::pressed, [=]() {
        if (currentVolume < 100) {
            setVolume(currentVolume+1);
        }
    } );
    // Set size.
    setFixedWidth(xPlayer::VolumeWidgetWidth);
}

void xPlayerVolumeWidget::setVolume(int vol) {
    currentVolume = vol;
    volumeKnob->setValue(vol);
    volumeMuteButton->setText(tr("Volume"));
}

void xPlayerVolumeWidget::setMuted(bool mute) {
    currentMuted = mute;
    volumeKnob->setDisabled(currentMuted);
    if (currentMuted) {
        volumeMuteButton->setText(tr("Muted"));
    } else {
        volumeMuteButton->setText(tr("Volume"));
    }
}

int xPlayerVolumeWidget::getVolume() const {
    return currentVolume;
}

bool xPlayerVolumeWidget::isMuted() const {
    return currentMuted;
}

void xPlayerVolumeWidget::volumeChanged(int vol) {
    currentVolume = vol;
    volumeMuteButton->setText(tr("Volume"));
    emit volume(vol);
}

void xPlayerVolumeWidget::toggleMuted() {
    setMuted(!isMuted());
    emit muted(isMuted());
}