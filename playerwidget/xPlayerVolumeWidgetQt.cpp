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
#include "xPlayerVolumeWidgetQt.h"

#include <QGridLayout>
#include <QLabel>

xPlayerVolumeWidgetQt::xPlayerVolumeWidgetQt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerVolumeWidget(parent, flags) {
    auto volumeLayout = new QGridLayout(this);
    volumeDial = new QDial(this);
    volumeDial->setRange(0, 100);
    volumeDial->setSingleStep(10);
    volumeDial->setNotchesVisible(true);
    volumeDial->setNotchTarget(10);
    volumeDial->setWrapping(false);
    auto volumeLabel = new QLabel(this);
    volumeLabel->setAlignment(Qt::AlignRight);
    volumeMuteButton = new QPushButton(tr("Volume"), this);
    volumeMuteButton->setFlat(true);
    // Qt implementation.
    volumeLayout->addWidget(volumeDial, 0, 0, 3, 4);
    volumeLayout->addWidget(volumeMuteButton, 3, 0, 1, 3);
    volumeLayout->addWidget(volumeLabel, 3, 3, 1, 1);
    // Connect the volume slider to the music player
    connect(volumeDial, &QDial::valueChanged, this, &xPlayerVolumeWidget::volume);
    connect(volumeDial, &QDial::valueChanged, this, [=](int vol) { currentVolume=vol; } );
    connect(volumeDial, &QDial::valueChanged, this, [=](int vol) { volumeLabel->setText(QString::number(vol)); } );
    connect(volumeMuteButton, &QPushButton::pressed, this, &xPlayerVolumeWidget::toggleMuted);
    setFixedWidth(170);
}

void xPlayerVolumeWidgetQt::setVolume(int vol) {
    currentVolume = vol;
    volumeDial->setValue(currentVolume);
}

void xPlayerVolumeWidgetQt::setMuted(bool mute) {
    currentMuted = mute;
    volumeDial->setDisabled(currentMuted);
    if (currentMuted) {
        volumeMuteButton->setText(tr("Muted"));
    } else {
        volumeMuteButton->setText(tr("Volume"));
    }
}

void xPlayerVolumeWidgetQt::mouseDoubleClickEvent(QMouseEvent* event) {
    // Toggle the muted mode.
    setMuted(!isMuted());
    // Tell the rest of the world.
    emit muted(isMuted());
    xPlayerVolumeWidget::mouseDoubleClickEvent(event);
}

