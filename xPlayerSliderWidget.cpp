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
#include "xPlayerSliderWidget.h"
#include "xPlayerUI.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QStyle>
#include <QDebug>

xPlayerSliderScaleWidget::xPlayerSliderScaleWidget(int offset, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        hourScale(true),
        sliderX(offset),
        labelX(0),
        lengthValue(0),
        maxScaleSections(8) {
    useHourScale(false);
    scaleDivider = determineScaleDivider(lengthValue);
    // Determine the slider control thickness.
    auto sliderThickness = style()->pixelMetric(QStyle::PM_SliderControlThickness);
    sliderX = offset + (sliderThickness+1)/2;  // add half to the offset.
    setFixedHeight(xPlayerLayout::HugeSpace);
}

void xPlayerSliderScaleWidget::setLength(qint64 length) {
   lengthValue = length;
   scaleDivider = determineScaleDivider(lengthValue);
   repaint();
}

void xPlayerSliderScaleWidget::useHourScale(bool enable) {
    if (hourScale == enable) {
        return;
    }
    hourScale = enable;
    if (hourScale) {
        labelSize = fontMetrics().size(Qt::TextSingleLine, "88:88:88");
    } else {
        labelSize = fontMetrics().size(Qt::TextSingleLine, "888:88");
    }
    // offset for label box.
    labelX = ((labelSize.width() + 1) / 2);
    labelBox.setSize(labelSize);
}

void xPlayerSliderScaleWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (event) {
        maxScaleSections = event->size().width() / labelBox.width() - 2;
        // Only update scaleDivider if length already set.
        if (lengthValue > 0) {
            scaleDivider = determineScaleDivider(lengthValue);
        }
    }
}

void xPlayerSliderScaleWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    if ((event) && (lengthValue > 0)) {
        QRect region = event->rect();
        QPainter scale(this);
        scale.setPen(Qt::SolidLine);
        auto posY = region.top();
        auto tickY = region.top() + 6;
        auto halfTickY = region.top() + 3;
        // Draw horizontal line
        scale.drawLine(region.left()+sliderX, posY, region.right()-sliderX, posY);
        auto scaleLength = (region.right() - region.left() - 2 * sliderX);
        auto scaleSections = static_cast<double>(lengthValue)/static_cast<double>(scaleDivider);
        auto scaleSectionLength = scaleLength / scaleSections;
        double currentSection = 0.0;
        qint64 currentLabel = 0;
        while (currentSection < scaleLength) {
            // Draw full tick.
            auto currentSectionX = static_cast<int>(std::round(currentSection));
            scale.drawLine(region.left()+sliderX+currentSectionX, posY,
                           region.left()+sliderX+currentSectionX, tickY);
            // Draw half tick.
            auto currentHalfTickX = static_cast<int>(std::round(currentSection + scaleSectionLength/2));
            if (currentHalfTickX < scaleLength) {
                scale.drawLine(region.left()+sliderX+currentHalfTickX, posY,
                               region.left()+sliderX+currentHalfTickX, halfTickY);
            }
            // Draw label for full tick.
            labelBox.moveTo(currentSectionX+sliderX-labelX, tickY);
            scale.drawText(labelBox, Qt::AlignCenter, scaleLabel(currentLabel));
            // Update
            currentLabel += scaleDivider;
            currentSection += scaleSectionLength;
        }
    }
}

qint64 xPlayerSliderScaleWidget::determineScaleDivider(qint64 length) const {
    for (auto divider : { 10000, 30000, 60000, 120000, 300000, 600000, 1200000, 3000000, 6000000 }) {
        if ((length / divider) <= maxScaleSections) {
            return divider;
        }
    }
    return 6000000;
}

QString xPlayerSliderScaleWidget::scaleLabel(qint64 value) const {
    if (hourScale) {
        return QString("%1:%2:%3").arg(value/3600000).arg((value/60000)%60, 2, 10, QChar('0')).
                arg((value/1000)%60, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg((value/60000)).arg((value/1000)%60, 2, 10, QChar('0'));
    }
}


xPlayerSliderWidget::xPlayerSliderWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        showHours(false),
        lengthValue(0),
        seekInProgress(false) {
    auto sliderLayout = new xPlayerLayout(this);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(0);
    // Create a slider that displays the played time and can be used
    // to seek within a track.
    slider = new QSlider(Qt::Horizontal, this);
    // Scale below
    slider->setTracking(false);
    slider->setTickPosition(QSlider::NoTicks);
    slider->setContentsMargins(0, 0, 0, 0);
    // Slider initially empty
    slider->setMinimum(0);
    slider->setMaximum(0);
    scale = new xPlayerSliderScaleWidget(xPlayer::SliderWidgetSliderOffset, this);
    // Create labels for length of the track and time played.
    // Labels located on the left and right of a slider.
    lengthLabel = new QLCDNumber(this);
    lengthLabel->setSegmentStyle(QLCDNumber::Flat);
    lengthLabel->setFrameStyle(QFrame::NoFrame);
    lengthLabel->setDigitCount(8);
    lengthLabel->setFixedHeight(xPlayerLayout::LargeSpace);
    playedLabel = new QLCDNumber(this);
    playedLabel->setSegmentStyle(QLCDNumber::Flat);
    playedLabel->setFrameStyle(QFrame::NoFrame);
    playedLabel->setDigitCount(8);
    playedLabel->setFixedHeight(xPlayerLayout::LargeSpace);
    sliderLayout->addWidget(playedLabel, 0, 0, 1, 1);
    sliderLayout->addWidget(lengthLabel, 0, 7, 1, 1);
    // Add spacer item left and right of the actual slider to allow space for scale label instead.
    sliderLayout->addItem(new QSpacerItem(xPlayer::SliderWidgetSliderOffset, xPlayer::SliderWidgetSliderOffset,
                                          QSizePolicy::Fixed, QSizePolicy::Minimum), 0, 1, 1, 1);
    sliderLayout->addItem(new QSpacerItem(xPlayer::SliderWidgetSliderOffset, xPlayer::SliderWidgetSliderOffset,
                                          QSizePolicy::Fixed, QSizePolicy::Minimum), 0, 6, 1, 1);
    sliderLayout->addWidget(slider, 0, 2, 1, 4);
    sliderLayout->addWidget(scale, 1, 1, 1, 6);
    // Connect the track slider to the music player. Seek only after the slider move is completed.
    connect(slider, &QSlider::sliderPressed, [=]() {
        seekInProgress = true;
    } );
    connect(slider, &QSlider::sliderMoved, [=](qint64 position) {
        // Show currently selected seek time instead of the played time.
        playedLabel->display(xPlayer::millisecondsToTimeFormat(position, showHours));
    } );
    connect(slider, &QSlider::sliderReleased, [=]() {
        seekInProgress = false;
        emit seek(slider->sliderPosition());
    } );
    // Clear played and length LCD display.
    clear();
}

void xPlayerSliderWidget::clear() {
    // Reset the slider range.
    slider->setMinimum(0);
    slider->setMaximum(0);
    // Reset the scale.
    scale->setLength(0);
    // Clear the lcd numbers.
    playedLabel->display("");
    lengthLabel->display("");
    // Reset track length.
    lengthValue = 0;
}

void xPlayerSliderWidget::useHourScale(bool hourScale) {
    showHours = hourScale;
    if (hourScale) {
        // hours:minutes:seconds
        playedLabel->setDigitCount(7);
        lengthLabel->setDigitCount(7);
    } else {
        // minutes:seconds.hseconds
        playedLabel->setDigitCount(8);
        lengthLabel->setDigitCount(8);
    }
    scale->useHourScale(hourScale);
}

void xPlayerSliderWidget::setLength(qint64 length) {
    // Update the length of the current track.
    lengthValue = length;
    if (lengthValue > 0) {
        scale->setLength(length);
        lengthLabel->display(xPlayer::millisecondsToTimeFormat(length, showHours));
        // Set maximum of slider to the length of the track. Reset the slider position-
        slider->setMinimum(0);
        slider->setMaximum(static_cast<int>(length));
        slider->setValue(0);
    }
}

void xPlayerSliderWidget::setPlayed(qint64 played) {
    // Only update the played slider if the length has been set.
    // Do not update if a slider seek is in progress.
    if ((lengthValue > 0) && (!seekInProgress)){
        // Update the time played for the current track.
        playedLabel->display(xPlayer::millisecondsToTimeFormat(played, showHours));
        // Update the slider position.
        slider->setValue(static_cast<int>(played));
    }
}

bool xPlayerSliderWidget::hourScale() const {
    return showHours;
}