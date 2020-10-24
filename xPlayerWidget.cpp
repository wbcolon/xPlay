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
#include "xPlayerWidget.h"
#include "xPlayerVolumeWidgetX.h"

#include <QGridLayout>
#include <QIcon>
#include <taglib/fileref.h>
#include <taglib/audioproperties.h>

xPlayerWidget::xPlayerWidget(xMusicPlayer* musicPlayer, QWidget* parent, Qt::WindowFlags flags):
    QWidget(parent, flags) {
    // Create labels for artist, album and track
    artistName = new QLabel(this);
    albumName = new QLabel(this);
    trackName = new QLabel(this);
    trackSampleRate = new QLabel(this);
    trackSampleRate->setAlignment(Qt::AlignRight);
    trackBitrate = new QLabel(this);
    trackBitrate->setAlignment(Qt::AlignRight);

    // Create buttons for play/pause and stop
    playPauseButton = new QPushButton(QIcon(":/images/xplay-play.svg"), tr("Play"), this);
    auto stopButton = new QPushButton(QIcon(":/images/xplay-stop.svg"), tr("Stop"), this);
    // Create buttons for playlist control, previous, next and clear queue.
    auto prevButton = new QPushButton(QIcon(":/images/xplay-previous.svg"), tr("Prev"), this);
    auto nextButton = new QPushButton(QIcon(":/images/xplay-next.svg"), tr("Next"), this);
    nextButton->setLayoutDirection(Qt::RightToLeft);
    //nextButton->setStyleSheet("padding-left: 20px; padding-right: 20px;");
    auto clearButton = new QPushButton(QIcon(":/images/xplay-eject.svg"), tr("Clear"), this);
    // Add track slider and the volume knob for Qwt.
    sliderWidget = new xPlayerSliderWidgetX(this);
    xPlayerVolumeWidgetX* volumeWidget = new xPlayerVolumeWidgetX(this);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, musicPlayer, &xMusicPlayer::setVolume);
    connect(sliderWidget, &xPlayerSliderWidget::seek, musicPlayer, &xMusicPlayer::seek);
    connect(musicPlayer, &xMusicPlayer::currentTrackPlayed, sliderWidget, &xPlayerSliderWidget::trackPlayed);
    connect(musicPlayer, &xMusicPlayer::currentTrackLength, sliderWidget, &xPlayerSliderWidget::trackLength);
    // Create the basic player widget layout.
    // Add labels, buttons and the slider.
    auto playerLayout = new QGridLayout(this);
    playerLayout->setSpacing(0);
    playerLayout->addWidget(new QLabel(tr("Artist"), this), 0, 0);
    playerLayout->addWidget(artistName, 0, 1, 1, 4);
    playerLayout->addWidget(new QLabel(tr("Album"), this), 1, 0);
    playerLayout->addWidget(albumName, 1, 1, 1, 4);
    playerLayout->addWidget(new QLabel(tr("Track"), this), 2, 0);
    playerLayout->addWidget(trackName, 2, 1, 1, 7);
    auto trackSampleRateLabel = new QLabel(tr("Sample rate"));
    trackSampleRateLabel->setAlignment(Qt::AlignRight);
    auto trackBitrateLabel = new QLabel(tr("Bitrate"));
    trackBitrateLabel->setAlignment(Qt::AlignRight);
    playerLayout->addWidget(trackSampleRateLabel, 0, 5);
    playerLayout->addWidget(trackSampleRate, 0, 6);
    playerLayout->addWidget(trackBitrateLabel, 1, 5);
    playerLayout->addWidget(trackBitrate, 1, 6);
    playerLayout->addWidget(sliderWidget, 4, 0, 1, 7);
    // Create a layout for the music player and playlist control buttons.
    auto controlLayout = new QGridLayout();
    controlLayout->addWidget(playPauseButton, 0, 5, 1, 2);
    controlLayout->addWidget(stopButton, 1, 5, 1, 2);
    controlLayout->addWidget(prevButton, 2, 5, 1, 1);
    controlLayout->addWidget(nextButton, 2, 6, 1, 1);
    controlLayout->addWidget(clearButton, 3, 5, 1, 2);
    controlLayout->setColumnMinimumWidth(4, 20);
    controlLayout->addWidget(volumeWidget, 0, 0, 4, 4);
    // Add the control layout to the player layout.
    playerLayout->setColumnMinimumWidth(8, 20);
    playerLayout->addLayout(controlLayout, 0, 9, 5, 1);
    // Connect the buttons to player widget and/or to the music player.
    connect(playPauseButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::playPause);
    connect(stopButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::stop);
    connect(prevButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::prev);
    connect(nextButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::next);
    connect(clearButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerWidget::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerWidget::clear);
    // Connect the music player to the player widget.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xPlayerWidget::currentTrack);
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xPlayerWidget::currentState);
    // Do not resize the player widget vertically
    setFixedHeight(sizeHint().height());
    // Setup volume
    volumeWidget->setVolume(musicPlayer->getVolume());
}

void xPlayerWidget::clear() {
    // Reset the play/pause button and clear all track info.
    artistName->clear();
    albumName->clear();
    trackName->clear();
    trackSampleRate->clear();
    trackBitrate->clear();
    sliderWidget->clear();
}

void xPlayerWidget::currentTrack(int index, const QString& artist, const QString& album,
                                 const QString& track, int bitrate, int sampleRate) {
    // Display the current track information (without length)
    artistName->setText(artist);
    albumName->setText(album);
    trackName->setText(track);
    trackSampleRate->setText(QString("%1 Hz").arg(sampleRate));
    trackBitrate->setText(QString("%1 kb/s").arg(bitrate));
    // Signal index update to the Queue.
    emit currentQueueTrack(index);
}

void xPlayerWidget::currentState(xMusicPlayer::State state) {
    // Update the play/pause button based on the state of the music player.
    switch (state) {
        case xMusicPlayer::PlayingState: {
            playPauseButton->setIcon(QIcon(":/images/xplay-pause.svg"));
            playPauseButton->setText(tr("Pause"));
        } break;
        case xMusicPlayer::PauseState:
        case xMusicPlayer::StopState: {
            playPauseButton->setIcon(QIcon(":/images/xplay-play.svg"));
            playPauseButton->setText(tr("Play"));
        } break;
    }
}
