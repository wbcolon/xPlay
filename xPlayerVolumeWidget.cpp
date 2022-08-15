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
#include "xPlayerPulseAudioControls.h"

#include "xPlayerUI.h"
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include <cmath>


xPlayerVolumeWidget::xPlayerVolumeWidget(bool pulseAudioMode, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        currentVolume(0),
        currentMuted(false) {
    auto volumeLayout = new xPlayerLayout(this);
    volumeKnob = new QwtKnob(this);
    volumeKnob->setLowerBound(0);
    volumeKnob->setUpperBound(100);
    volumeKnob->setScaleStepSize(20);
    volumeKnob->setWrapping(false);
    volumeKnob->setContentsMargins(0, 0, 0, 0);
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
    volumeLayout->addWidget(volumeKnob, 1, 0, 4, 4);
    volumeLayout->addWidget(volumeMinusButton, 4, 0);
    volumeLayout->addWidget(volumeMuteButton, 4, 1, 1, 2);
    volumeLayout->addWidget(volumePlusButton, 4, 3);
    volumeLayout->addItem(new QSpacerItem(1, xPlayer::VolumeWidgetHeight), 3, 0, 1, 4);
    volumeLayout->addRowStretcher(5);
    // Connect the volume slider to the widgets signal. Use lambda to do proper conversion.
    connect(volumeKnob, &QwtKnob::valueChanged, [=](double vol) { emit volume(static_cast<int>(vol)); } );
    connect(volumeKnob, &QwtKnob::valueChanged, [=](double vol) { currentVolume = static_cast<int>(vol); } );
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
    if (pulseAudioMode) {
        // Connect to pulse audio controls.
        connect(xPlayerPulseAudioControls::controls(), &xPlayerPulseAudioControls::volume, [=](int vol) {
            setVolume(vol);
        });
        connect(xPlayerPulseAudioControls::controls(), &xPlayerPulseAudioControls::muted, [=](bool muted) {
            setMuted(muted);
        });
    }
    // Set size.
    setFixedWidth(xPlayer::VolumeWidgetWidth);
}

void xPlayerVolumeWidget::setVolume(int vol) {
    currentVolume = vol;
    volumeKnob->setValue(static_cast<double>(vol));
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

void xPlayerVolumeWidget::toggleMuted() {
    setMuted(!isMuted());
    emit muted(isMuted());
}

