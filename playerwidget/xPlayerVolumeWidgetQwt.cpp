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
#include "xPlayerVolumeWidgetQwt.h"

#include "xPlayerUI.h"
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>

xPlayerVolumeWidgetQwt::xPlayerVolumeWidgetQwt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerVolumeWidget(parent, flags) {
    auto volumeLayout = new xPlayerLayout(this);
    volumeKnob = new QwtKnob(this);
    volumeKnob->setLowerBound(0);
    volumeKnob->setUpperBound(100);
    volumeKnob->setScaleStepSize(20);
    volumeKnob->setWrapping(false);
    volumeMuteButton = new QPushButton(tr("Volume"), this);
    volumeMuteButton->setFlat(true);
    // Only stretch top and bottom.
    volumeLayout->addRowStretcher(0);
    // Qwt implementation. Layout here overlap on purpose
    volumeLayout->addWidget(volumeKnob, 1, 0, 4, 4);
    volumeLayout->addWidget(volumeMuteButton, 4, 0, 1, 4);
    volumeLayout->addRowStretcher(5);
    // Connect the volume slider to the widgets signal. Use lambda to do proper conversion.
    connect(volumeKnob, &QwtKnob::valueChanged, [=](double vol) { emit volume(static_cast<int>(vol)); } );
    connect(volumeKnob, &QwtKnob::valueChanged, [=](double vol) { currentVolume=static_cast<int>(vol); } );
    connect(volumeMuteButton, &QPushButton::pressed, this, &xPlayerVolumeWidget::toggleMuted);
    setFixedWidth(xPlayer::VolumeWidgetWidth);
}

void xPlayerVolumeWidgetQwt::setVolume(int vol) {
    currentVolume = vol;
    volumeKnob->setValue(static_cast<double>(vol));
}

void xPlayerVolumeWidgetQwt::setMuted(bool mute) {
    currentMuted = mute;
    volumeKnob->setDisabled(currentMuted);
    if (currentMuted) {
        volumeMuteButton->setText(tr("Muted"));
    } else {
        volumeMuteButton->setText(tr("Volume"));
    }
}

