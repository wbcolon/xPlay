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
#include "xPlayerSliderWidgetQt.h"

#include <QGridLayout>

xPlayerSliderWidgetQt::xPlayerSliderWidgetQt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerSliderWidget(parent, flags) {
    auto sliderLayout = new QGridLayout(this);
    // Create a slider that displays the played time and can be used
    // to seek within a track.
    trackSlider = new QSlider(Qt::Horizontal, this);
    trackSlider->setTickPosition(QSlider::TicksBelow);
    trackSlider->setTracking(false);
    // Create labels for length of the track and time played.
    // Labels located on the left and right of a slider.
    trackLengthLabel = new QLabel(this);
    trackLengthLabel->setAlignment(Qt::AlignCenter);
    trackPlayedLabel = new QLabel(this);
    trackPlayedLabel->setAlignment(Qt::AlignCenter);
    sliderLayout->addWidget(trackPlayedLabel, 0, 0);
    sliderLayout->addWidget(trackLengthLabel, 0, 7);
    sliderLayout->addWidget(trackSlider, 0, 1, 1, 6);
    // Connect the track slider to the music player
    connect(trackSlider, &QSlider::sliderMoved, [=](int position) { emit seek(static_cast<qint64>(position)); } );
}

void xPlayerSliderWidgetQt::clear() {
    // Reset the slider range.
    trackSlider->setRange(0, 0);
    // Clear labels.
    trackPlayedLabel->clear();
    trackLengthLabel->clear();
}

void xPlayerSliderWidgetQt::trackLength(qint64 length) {
    // Update the length of the current track.
    trackLengthLabel->setText(xPlayerSliderWidget::millisecondsToLabel(length));
    // Set maximum of slider to the length of the track. Reset the slider position-
    trackSlider->setTickInterval(xPlayerSliderWidget::determineScaleDivider(length, 20));
    trackSlider->setRange(0, length);
    trackSlider->setSliderPosition(0);
}

void xPlayerSliderWidgetQt::trackPlayed(qint64 played) {
    // Update the time played for the current track.
    trackPlayedLabel->setText(xPlayerSliderWidget::millisecondsToLabel(played));
    // Update the slider position.
    trackSlider->setValue(played);
}
