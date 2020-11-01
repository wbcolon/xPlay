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

#include "xPlayerMovieWidget.h"

#include "xPlayerVolumeWidgetX.h"

#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QDebug>

xPlayerMovieWidget::xPlayerMovieWidget(xMoviePlayer* player, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        moviePlayer(player) {

    // Slider widget for showing movie length and played time.
    sliderWidget = new xPlayerSliderWidgetX(this);
    sliderWidget->useScaleSections(8);
    sliderWidget->useHourScale(true);
    // Name of the movie (filename).
    movieLabel = new QLabel(this);
    movieLabel->setAlignment(Qt::AlignCenter);
    // Control Tab
    auto controlTab = new QTabWidget(this);
    auto controlTabPlayer = new QWidget(this);
    controlTabRotel = new xPlayerRotelWidget(this);
    controlTab->setTabPosition(QTabWidget::West);
    controlTab->addTab(controlTabPlayer, "xPlay");
    controlTab->addTab(controlTabRotel, "Rotel");
    // Create buttons for play/pause and stop
    playPauseButton = new QPushButton(QIcon(":/images/xplay-play.svg"), tr("Play"), controlTabPlayer);
    auto stopButton = new QPushButton(QIcon(":/images/xplay-stop.svg"), tr("Stop"), controlTabPlayer);
    // Create buttons for playlist control, rewind, forward, previous and next.
    auto rewButton = new QPushButton(QIcon(":/images/xplay-rewind.svg"), tr("Rew"), controlTabPlayer);
    auto fwdButton = new QPushButton(QIcon(":/images/xplay-forward.svg"), tr("Fwd"), controlTabPlayer);
    fwdButton->setLayoutDirection(Qt::RightToLeft);
    auto fullWindowButton = new QPushButton(tr("Full Window"));
    audioChannelBox = new QComboBox(controlTabPlayer);
    auto audioChannelLabel = new QLabel(tr("Audio"), controlTabPlayer);
    audioChannelLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    subtitleBox = new QComboBox(controlTabPlayer);
    auto subtitleLabel = new QLabel(tr("Subtitle"), controlTabPlayer);
    subtitleLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    auto scaleAndCropCheck = new QCheckBox(tr("Scale and Crop"),controlTabPlayer);
    scaleAndCropCheck->setChecked(false);
    auto volumeWidget = new xPlayerVolumeWidgetX(controlTabPlayer);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, moviePlayer, &xMoviePlayer::setVolume);
    connect(moviePlayer, &xMoviePlayer::currentMoviePlayed, sliderWidget, &xPlayerSliderWidget::trackPlayed);
    connect(moviePlayer, &xMoviePlayer::currentMovieLength, sliderWidget, &xPlayerSliderWidget::trackLength);
    // Layout
    auto controlLayout = new QGridLayout(controlTabPlayer);
    controlLayout->addWidget(playPauseButton, 0, 5, 1, 2);
    controlLayout->addWidget(stopButton, 1, 5, 1, 2);
    controlLayout->addWidget(rewButton, 2, 5, 1, 1);
    controlLayout->addWidget(fwdButton, 2, 6, 1, 1);
    controlLayout->addWidget(fullWindowButton, 3, 5, 1, 2);
    controlLayout->setColumnMinimumWidth(4, 20);
    controlLayout->addWidget(volumeWidget, 0, 0, 4, 4);
    // Fix sizes of the control tab
    controlTabPlayer->setFixedSize(controlTabPlayer->sizeHint());
    controlTabRotel->setFixedSize(controlTabPlayer->sizeHint());
    controlTab->setFixedSize(controlTab->sizeHint());
    // Layout
    auto movieLayout = new QGridLayout(this);
    movieLayout->addWidget(movieLabel, 0, 0, 1, 8);
    movieLayout->addWidget(sliderWidget, 1, 0, 1, 8);
    movieLayout->addWidget(audioChannelLabel, 2, 3, 1, 2);
    movieLayout->addWidget(audioChannelBox, 3, 3, 1, 2);
    movieLayout->addWidget(subtitleLabel, 2, 5, 1, 2);
    movieLayout->addWidget(subtitleBox, 3, 5, 1, 2);
    movieLayout->addWidget(scaleAndCropCheck, 3, 1, 1, 2);
    movieLayout->addWidget(controlTab, 0, 9, 4, 1);
    movieLabel->setText("no movie");
    // Connect the buttons to player widget and/or to the music player.
    connect(playPauseButton, &QPushButton::pressed, moviePlayer, &xMoviePlayer::playPause);
    connect(stopButton, &QPushButton::pressed, moviePlayer, &xMoviePlayer::stop);
    connect(rewButton, &QPushButton::pressed, [=]() { moviePlayer->jump(-60000); });
    connect(fwdButton, &QPushButton::pressed, [=]() { moviePlayer->jump(60000); });
    connect(fullWindowButton, &QPushButton::pressed, this, &xPlayerMovieWidget::toggleFullWindow);
    // Connect check box.
    connect(scaleAndCropCheck, &QCheckBox::clicked, moviePlayer, &xMoviePlayer::setScaleAndCropMode);
    connect(moviePlayer, &xMoviePlayer::scaleAndCropMode, scaleAndCropCheck, &QCheckBox::setChecked);
    // Connect combo boxes.
    connect(audioChannelBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectAudioChannel(int)));
    connect(subtitleBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectSubtitle(int)));
    // Seek.
    connect(sliderWidget, &xPlayerSliderWidgetX::seek, moviePlayer, &xMoviePlayer::seek);
    // Movie player volume updates.
    connect(moviePlayer, &xMoviePlayer::currentVolume, volumeWidget, &xPlayerVolumeWidgetX::setVolume);
    // Setup volume
    volumeWidget->setVolume(moviePlayer->getVolume());
}

void xPlayerMovieWidget::currentMovie(const QString& movie) {
    movieLabel->setText(movie);
}

void xPlayerMovieWidget::currentAudioChannels(const QStringList& audioChannels) {
    updateComboBoxEntries(audioChannelBox, audioChannels);
}

void xPlayerMovieWidget::currentSubtitles(const QStringList& subtitles) {
    updateComboBoxEntries(subtitleBox, subtitles);
}

void xPlayerMovieWidget::currentMoviePlayed(qint64 played) {
    sliderWidget->trackPlayed(played);
}

void xPlayerMovieWidget::currentMovieLength(qint64 length) {
    sliderWidget->trackLength(length);
}

void xPlayerMovieWidget::currentState(xMoviePlayer::State state) {
    // Update the play/pause button based on the state of the music player.
    moviePlayerState = state;
    switch (moviePlayerState) {
        case xMoviePlayer::PlayingState: {
            playPauseButton->setIcon(QIcon(":/images/xplay-pause.svg"));
            playPauseButton->setText(tr("Pause"));
        } break;
        case xMoviePlayer::PauseState:
        case xMoviePlayer::StopState: {
            playPauseButton->setIcon(QIcon(":/images/xplay-play.svg"));
            playPauseButton->setText(tr("Play"));
        } break;
        default: break;
    }
}

void xPlayerMovieWidget::fullWindowPressed() {
    if ((moviePlayerState == xMoviePlayer::PlayingState) ||
        (moviePlayerState == xMoviePlayer::PauseState)) {
        emit toggleFullWindow();
    }
}

void xPlayerMovieWidget::updateComboBoxEntries(QComboBox* comboBox, const QStringList& entries) {
    // Avoid constant updates
    qDebug() << "xPlayerMovieWidget: updateComboBoxEntries(0): " << comboBox->count() << ", " << entries.size();
    if (comboBox->count() == entries.size()) {
        for (auto i = 0; i < comboBox->count(); ++i) {
            qDebug() << "xPlayerMovieWidget: updateComboBoxEntries(1): " << comboBox->itemText(i) << ", " << entries[i];
            // Refresh if we find unmatched items.
            if (comboBox->itemText(i) != entries[i]) {
                comboBox->clear();
                comboBox->addItems(entries);
                return;
            }
        }
    } else {
        comboBox->clear();
        comboBox->addItems(entries);
    }
}
