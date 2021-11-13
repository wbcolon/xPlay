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
#include "xPlayerSliderWidgetQwt.h"
#include "xPlayerUI.h"

xPlayerSliderWidgetQwt::xPlayerSliderWidgetQwt(QWidget *parent, Qt::WindowFlags flags):
        xPlayerSliderWidget(parent, flags) {
    auto sliderLayout = new xPlayerLayout(this);
    // Create a slider that displays the played time and can be used
    // to seek within a track.
    trackSlider = new QwtSlider(Qt::Horizontal, this);
    // Scale below
    trackSlider->setScalePosition(QwtSlider::LeadingScale);
    trackSlider->setTracking(false);
    scaleDraw = new xPlayerWidgetScaleDraw();
    trackSlider->setScaleDraw(scaleDraw);
    // Slider initially empty
    trackSlider->setLowerBound(0);
    trackSlider->setUpperBound(0);
    trackSlider->setGroove(false);
    trackSlider->setTrough(false);
    // Adjust the size of the Handle. A little smaller.
    trackSlider->setHandleSize(QSize(16, 24));
    trackSlider->setSpacing(2);
    // Create labels for length of the track and time played.
    // Labels located on the left and right of a slider.
    trackLengthLabel = new QLCDNumber(this);
    trackLengthLabel->setSegmentStyle(QLCDNumber::Flat);
    trackLengthLabel->setFrameStyle(QFrame::NoFrame);
    trackLengthLabel->setDigitCount(8);
    trackLengthLabel->setFixedHeight(xPlayerLayout::LargeSpace);
    trackPlayedLabel = new QLCDNumber(this);
    trackPlayedLabel->setSegmentStyle(QLCDNumber::Flat);
    trackPlayedLabel->setFrameStyle(QFrame::NoFrame);
    trackPlayedLabel->setDigitCount(8);
    trackPlayedLabel->setFixedHeight(xPlayerLayout::LargeSpace);
    sliderLayout->addWidget(trackPlayedLabel, 0, 0);
    sliderLayout->addWidget(trackLengthLabel, 0, 7);
    sliderLayout->addWidget(trackSlider, 0, 1, 2, 6);
    // Connect the track slider to the music player. Do proper conversion using lambdas.
    connect(trackSlider, &QwtSlider::sliderMoved, [=](double position) { emit seek(static_cast<qint64>(position)); } );
    // Setup max sections.
    useScaleSections(10); // NOLINT
}

void xPlayerSliderWidgetQwt::clear() {
    // Reset the slider range.
    trackSlider->setLowerBound(0);
    trackSlider->setUpperBound(0);
    // Clear the lcd numbers.
    trackPlayedLabel->display("");
    trackLengthLabel->display("");
}

void xPlayerSliderWidgetQwt::useHourScale(bool hourScale) {
    scaleDraw->useHourScale(hourScale);
    xPlayerSliderWidget::useHourScale(hourScale);
    if (hourScale) {
        // hours:minutes:seconds
        trackPlayedLabel->setDigitCount(7);
        trackLengthLabel->setDigitCount(7);
    } else {
        // minutes:seconds.hseconds
        trackPlayedLabel->setDigitCount(8);
        trackLengthLabel->setDigitCount(8);
    }
}

void xPlayerSliderWidgetQwt::trackLength(qint64 length) {
    // Update the length of the current track.
    trackLengthLabel->display(xPlayer::millisecondsToTimeFormat(length, showHours));
    // Set maximum of slider to the length of the track. Reset the slider position-
    trackSlider->setScaleStepSize(determineScaleDivider(length));
    trackSlider->setLowerBound(0);
    trackSlider->setUpperBound(length);
    trackSlider->setValue(0);
}

void xPlayerSliderWidgetQwt::trackPlayed(qint64 played) {
    // Update the time played for the current track.
    trackPlayedLabel->display(xPlayer::millisecondsToTimeFormat(played, showHours));
    // Update the slider position.
    trackSlider->setValue(played);
}
