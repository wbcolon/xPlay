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

#include "xPlayerControlButtonWidget.h"
#include "xPlayerUI.h"

#include <QPushButton>

xPlayerControlButtonWidget::xPlayerControlButtonWidget(xPlayerControlButtonMode mode, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        playPauseState(true),
        controlMode(mode) {

    auto controlButtonLayout = new xPlayerLayout();
    controlButtonLayout->setSpacing(xPlayerLayout::NoSpace);
    playPauseButton = new QPushButton(QIcon(":/images/xplay-play.svg"), tr("Play"), this);
    auto stopButton = new QPushButton(QIcon(":/images/xplay-stop.svg"), tr("Stop"), this);
    controlButtonLayout->addRowStretcher(0);
    controlButtonLayout->addWidget(playPauseButton, 1, 0, 1, 2);
    controlButtonLayout->addWidget(stopButton, 2, 0, 1, 2);
    connect(playPauseButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::playPause);
    connect(stopButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::stop);
    if (controlMode == xPlayerControlButtonMode::MusicPlayerMode) {
        // Create buttons for playlist control, previous, next and clear queue.
        auto previousButton = new QPushButton(QIcon(":/images/xplay-previous.svg"), tr("Prev"), this);
        auto nextButton = new QPushButton(QIcon(":/images/xplay-next.svg"), tr("Next"), this);
        nextButton->setLayoutDirection(Qt::RightToLeft);
        auto clearButton = new QPushButton(QIcon(":/images/xplay-eject.svg"), tr("Clear"), this);
        controlButtonLayout->addWidget(previousButton, 3, 0);
        controlButtonLayout->addWidget(nextButton, 3, 1);
        controlButtonLayout->addRowSpacer(4, xPlayerLayout::SmallSpace);
        controlButtonLayout->addWidget(clearButton, 5, 0, 1, 2);
        connect(previousButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::previousPressed);
        connect(nextButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::nextPressed);
        connect(clearButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::clearPressed);
        connect(clearButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::stop);
    } else {
        // Create buttons for movie control, rewind, forward and full window.
        auto rewindButton = new QPushButton(QIcon(":/images/xplay-rewind.svg"), tr("Rwd"), this);
        auto forwardButton = new QPushButton(QIcon(":/images/xplay-forward.svg"), tr("Fwd"), this);
        forwardButton->setLayoutDirection(Qt::RightToLeft);
        auto fullWindowButton = new QPushButton(tr("Full Window"), this);
        controlButtonLayout->addWidget(rewindButton, 3, 0);
        controlButtonLayout->addWidget(forwardButton, 3, 1);
        controlButtonLayout->addRowSpacer(4, xPlayerLayout::SmallSpace);
        controlButtonLayout->addWidget(fullWindowButton, 5, 0, 1, 2);
        connect(rewindButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::rewindPressed);
        connect(forwardButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::forwardPressed);
        connect(fullWindowButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::fullWindowPressed);
    }
    controlButtonLayout->addRowStretcher(6);
    controlButtonLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(controlButtonLayout);
    setContentsMargins(0, 0, 0, 0);
    setFixedWidth(xPlayerControlButtonWidgetWidth);
}

void xPlayerControlButtonWidget::setPlayPauseState(bool mode) {
    playPauseState = mode;
    if (playPauseState) {
        playPauseButton->setText(tr("Play"));
        playPauseButton->setIcon(QIcon(":/images/xplay-play.svg"));
    } else {
        playPauseButton->setText(tr("Pause"));
        playPauseButton->setIcon(QIcon(":/images/xplay-pause.svg"));
    }
}

bool xPlayerControlButtonWidget::getPlayPauseState() const {
    return playPauseState;
}

void xPlayerControlButtonWidget::playPause() {
    setPlayPauseState(!playPauseState);
    emit playPausePressed();
}

void xPlayerControlButtonWidget::stop() {
    playPauseState = true;
    playPauseButton->setText(tr("Play"));
    playPauseButton->setIcon(QIcon(":/images/xplay-play.svg"));
    emit stopPressed();
}