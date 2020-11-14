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
#include "xPlayerMusicWidget.h"
#include "xPlayerVolumeWidgetX.h"
#include "xPlayerRotelWidget.h"

#include <QGridLayout>
#include <QTabWidget>
#include <QIcon>
#include <taglib/fileref.h>
#include <taglib/audioproperties.h>

xPlayerMusicWidget::xPlayerMusicWidget(xMusicPlayer* musicPlayer, QWidget* parent, Qt::WindowFlags flags):
    QWidget(parent, flags) {
    // Create labels for artist, album and track
    artistName = new QLabel(this);
    artistName->setWordWrap(true);
    albumName = new QLabel(this);
    albumName->setWordWrap(true);
    trackName = new QLabel(this);
    trackName->setWordWrap(true);
    trackSampleRate = new QLabel(this);
    trackSampleRate->setAlignment(Qt::AlignRight);
    trackBitsPerSample = new QLabel(this);
    trackBitsPerSample->setAlignment(Qt::AlignRight|Qt::AlignTop);
    trackBitrate = new QLabel(this);
    trackBitrate->setAlignment(Qt::AlignRight);
    // Add track slider and the volume knob for Qwt.
    sliderWidget = new xPlayerSliderWidgetX(this);
    connect(sliderWidget, &xPlayerSliderWidget::seek, musicPlayer, &xMusicPlayer::seek);
    // Add a control tab for player and rotel amp controls
    auto controlTab = new QTabWidget(this);
    auto controlTabPlayer = new QWidget(this);
    controlTabRotel = new xPlayerRotelWidget(this);
    controlTab->setTabPosition(QTabWidget::West);
    controlTab->addTab(controlTabPlayer, "xPlay");
    controlTab->addTab(controlTabRotel, "Rotel");
    // Create buttons for play/pause and stop
    playPauseButton = new QPushButton(QIcon(":/images/xplay-play.svg"), tr("Play"), controlTabPlayer);
    auto stopButton = new QPushButton(QIcon(":/images/xplay-stop.svg"), tr("Stop"), controlTabPlayer);
    // Create buttons for playlist control, previous, next and clear queue.
    auto prevButton = new QPushButton(QIcon(":/images/xplay-previous.svg"), tr("Prev"), controlTabPlayer);
    auto nextButton = new QPushButton(QIcon(":/images/xplay-next.svg"), tr("Next"), controlTabPlayer);
    nextButton->setLayoutDirection(Qt::RightToLeft);
    //nextButton->setStyleSheet("padding-left: 20px; padding-right: 20px;");
    auto clearButton = new QPushButton(QIcon(":/images/xplay-eject.svg"), tr("Clear"), controlTabPlayer);
    xPlayerVolumeWidgetX* volumeWidget = new xPlayerVolumeWidgetX(controlTabPlayer);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, musicPlayer, &xMusicPlayer::setVolume);
    connect(volumeWidget, &xPlayerVolumeWidget::muted, musicPlayer, &xMusicPlayer::setMuted);
    connect(musicPlayer, &xMusicPlayer::currentVolume, volumeWidget, &xPlayerVolumeWidgetX::setVolume);
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
    playerLayout->addWidget(trackName, 2, 1, 1, 4);
    auto trackSampleRateLabel = new QLabel(tr("Sample rate"));
    trackSampleRateLabel->setAlignment(Qt::AlignRight);
    //auto trackBitsPerSampleLabel = new QLabel(tr("Bits per sample"));
    //trackBitsPerSampleLabel->setAlignment(Qt::AlignRight);
    auto trackBitrateLabel = new QLabel(tr("Bitrate"));
    trackBitrateLabel->setAlignment(Qt::AlignRight);
    playerLayout->addWidget(trackSampleRateLabel, 0, 5);
    playerLayout->addWidget(trackSampleRate, 0, 6);
    //playerLayout->addWidget(trackBitsPerSampleLabel, 1, 5);
    playerLayout->addWidget(trackBitrateLabel, 2, 5);
    playerLayout->addWidget(trackBitsPerSample, 1, 6);
    playerLayout->addWidget(trackBitrate, 2, 6);
    playerLayout->addWidget(sliderWidget, 4, 0, 1, 7);
    // Create a layout for the music player and playlist control buttons.
    auto controlLayout = new QGridLayout(controlTabPlayer);
    controlLayout->addWidget(playPauseButton, 0, 5, 1, 2);
    controlLayout->addWidget(stopButton, 1, 5, 1, 2);
    controlLayout->addWidget(prevButton, 2, 5, 1, 1);
    controlLayout->addWidget(nextButton, 2, 6, 1, 1);
    controlLayout->addWidget(clearButton, 3, 5, 1, 2);
    controlLayout->setColumnMinimumWidth(4, 20);
    controlLayout->addWidget(volumeWidget, 0, 0, 4, 4);
    // Fix sizes of the control tab
    controlTabPlayer->setFixedSize(controlTabPlayer->sizeHint());
    controlTabRotel->setFixedSize(controlTabPlayer->sizeHint());
    controlTab->setFixedSize(controlTab->sizeHint());
    // Add the control tab to the player layout.
    playerLayout->setColumnMinimumWidth(7, 50);
    playerLayout->setColumnStretch(7, 0);
    playerLayout->addWidget(controlTab, 0, 8, 5, 1);
    // Connect the buttons to player widget and/or to the music player.
    connect(playPauseButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::playPause);
    connect(stopButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::stop);
    connect(prevButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::prev);
    connect(nextButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::next);
    connect(clearButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerMusicWidget::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerMusicWidget::clear);
    // Connect the music player to the player widget.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xPlayerMusicWidget::currentTrack);
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xPlayerMusicWidget::currentState);
    // Do not resize the player widget vertically
    setFixedHeight(sizeHint().height());
    // Setup volume
    volumeWidget->setVolume(musicPlayer->getVolume());
}

void xPlayerMusicWidget::clear() {
    // Reset the play/pause button and clear all track info.
    artistName->clear();
    albumName->clear();
    trackName->clear();
    trackSampleRate->clear();
    trackBitsPerSample->clear();
    trackBitrate->clear();
    sliderWidget->clear();
}

void xPlayerMusicWidget::currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                                      int bitrate, int sampleRate, int bitsPerSample) {
    // Display the current track information (without length)
    artistName->setText(artist);
    albumName->setText(album);
    trackName->setText(track);
    trackSampleRate->setText(QString("%1Hz").arg(sampleRate));
    trackBitsPerSample->setText(QString("%1-bit").arg(bitsPerSample));
    trackBitrate->setText(QString("%1 kbit/s").arg(bitrate));
    // Signal index update to the Queue.
    emit currentQueueTrack(index);
}

void xPlayerMusicWidget::currentState(xMusicPlayer::State state) {
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
