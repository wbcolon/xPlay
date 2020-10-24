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

#include <QGridLayout>
#include <QLabel>

xPlayerVolumeWidgetQwt::xPlayerVolumeWidgetQwt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerVolumeWidget(parent, flags){
    auto volumeLayout = new QGridLayout(this);
    volumeKnob = new QwtKnob(this);
    volumeKnob->setLowerBound(0);
    volumeKnob->setUpperBound(100);
    volumeKnob->setScaleStepSize(20);
    volumeKnob->setWrapping(false);
    // Qwt implementation. Layout here overlap on purpose
    volumeLayout->addWidget(volumeKnob, 0, 0, 4, 4);
    auto volumeLabel = new QLabel(tr("Volume"));
    volumeLabel->setAlignment(Qt::AlignCenter);
    volumeLayout->addWidget(volumeLabel, 3, 0, 1, 4);
    // Connect the volume slider to the widgets signal. Use lambda to do proper conversion.
    connect(volumeKnob, &QwtKnob::valueChanged, [=](double vol) { emit volume(static_cast<int>(vol)); } );
    connect(volumeKnob, &QwtKnob::valueChanged, [=](double vol) { currentVolume=static_cast<int>(vol); } );
}

void xPlayerVolumeWidgetQwt::setVolume(int vol) {
    currentVolume = vol;
    volumeKnob->setValue(static_cast<double>(vol));
}
