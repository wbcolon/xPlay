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
#include <QSizePolicy>

xPlayerControlButtonWidget::xPlayerControlButtonWidget(xPlayerControlButtonMode mode, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        playPauseState(true),
        controlMode(mode) {

    auto controlButtonLayout = new xPlayerLayout();
    controlButtonLayout->setSpacing(xPlayerLayout::NoSpace);
    // Create buttons for play control.
    playPauseButton = new QPushButton(QIcon(":/images/xplay-play.png"), "", this);
    auto stopButton = new QPushButton(QIcon(":/images/xplay-stop.png"), "", this);
    auto rewindButton = new QPushButton(QIcon(":/images/xplay-rewind.png"), "", this);
    auto forwardButton = new QPushButton(QIcon(":/images/xplay-forward.png"), "", this);
    auto previousButton = new QPushButton(QIcon(":/images/xplay-previous.png"), "", this);
    auto nextButton = new QPushButton(QIcon(":/images/xplay-next.png"), "", this);
    // Set commond tool tips.
    rewindButton->setToolTip(tr("Rewind"));
    forwardButton->setToolTip(tr("Forward"));
    stopButton->setToolTip(tr("Stop"));
    controlButtonLayout->addRowSpacer(0, xPlayerLayout::MediumSpace);
    controlButtonLayout->addWidget(playPauseButton, 1, 0, 1, 3);
    controlButtonLayout->addWidget(stopButton, 2, 1, 2, 1);
    controlButtonLayout->addWidget(rewindButton, 2, 0);
    controlButtonLayout->addWidget(forwardButton, 2, 2);
    controlButtonLayout->addWidget(previousButton, 3, 0);
    controlButtonLayout->addWidget(nextButton, 3, 2);
    controlButtonLayout->addRowSpacer(4, xPlayerLayout::SmallSpace);
    // Connect common buttons.
    connect(playPauseButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::playPause);
    connect(stopButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::stop);
    connect(previousButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::previousPressed);
    connect(nextButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::nextPressed);
    connect(rewindButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::rewindPressed);
    connect(forwardButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::forwardPressed);
    if (controlMode == xPlayerControlButtonMode::MusicPlayerMode) {
        // Add tool tips for music player.
        previousButton->setToolTip(tr("Previous Track"));
        nextButton->setToolTip(tr("Next Track"));
        // Add clear queue button
        auto clearButton = new QPushButton(QIcon(":/images/xplay-eject.png"), "", this);
        clearButton->setToolTip(tr("Clear Queue"));
        clearButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        controlButtonLayout->addWidget(clearButton, 5, 0, 1, 3);
        connect(clearButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::clearPressed);
        connect(clearButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::stop);
    } else {
        // Add tool tips for movie player.
        previousButton->setToolTip(tr("Previous Chapter"));
        nextButton->setToolTip(tr("Next Chapter"));
        // Add full window button
        auto fullWindowButton = new QPushButton(QIcon(":/images/xplay-fullwindow.png"), "", this);
        fullWindowButton->setToolTip(tr("Full Window"));
        fullWindowButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        controlButtonLayout->addWidget(fullWindowButton, 5, 0, 1, 3);
        connect(fullWindowButton, &QPushButton::pressed, this, &xPlayerControlButtonWidget::fullWindowPressed);
    }
    controlButtonLayout->addRowSpacer(6, xPlayerLayout::MediumSpace);
    controlButtonLayout->setContentsMargins(0, 0, 0, 0);
    // Set size policies.
    playPauseButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    stopButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    forwardButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    rewindButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    previousButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    nextButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setLayout(controlButtonLayout);
    setContentsMargins(0, 0, 0, 0);
    setFixedWidth(xPlayer::ControlButtonWidgetWidth);
}

void xPlayerControlButtonWidget::setPlayPauseState(bool mode) {
    playPauseState = mode;
    if (playPauseState) {
        playPauseButton->setToolTip(tr("Play"));
        playPauseButton->setIcon(QIcon(":/images/xplay-play.png"));
    } else {
        playPauseButton->setToolTip(tr("Pause"));
        playPauseButton->setIcon(QIcon(":/images/xplay-pause.png"));
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
    playPauseButton->setToolTip(tr("Play"));
    playPauseButton->setIcon(QIcon(":/images/xplay-play.png"));
    emit stopPressed();
}