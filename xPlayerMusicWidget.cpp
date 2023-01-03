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
#include "xPlayerVolumeWidget.h"
#include "xPlayerRotelWidget.h"
#include "xPlayerPulseAudioControls.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"

#include <QTabWidget>
#include <QMouseEvent>
#include <taglib/fileref.h>

xPlayerMusicWidget::xPlayerMusicWidget(xMusicPlayer* player, QWidget* parent, Qt::WindowFlags flags):
    QWidget(parent, flags),
    musicPlayer(player) {
    // Create labels for artist, album and track
    auto artistLabel = new QLabel(tr("Artist"), this);
    artistName = new QLabel(this);
    artistName->setWordWrap(true);
    artistName->setStyleSheet("font-weight: bold");
    auto albumLabel = new QLabel(tr("Album"), this);
    albumName = new QLabel(this);
    albumName->setWordWrap(true);
    albumName->setStyleSheet("font-weight: bold");
    auto trackLabel = new QLabel(tr("Track"), this);
    trackName = new QLabel(this);
    trackName->setWordWrap(true);
    trackName->setStyleSheet("font-weight: bold");
    trackSampleRate = new QLabel(this);
    trackSampleRate->setAlignment(Qt::AlignRight);
    trackSampleRate->setStyleSheet("font-weight: bold");
    trackBitsPerSample = new QLabel(this);
    trackBitsPerSample->setAlignment(Qt::AlignRight|Qt::AlignTop);
    trackBitsPerSample->setStyleSheet("font-weight: bold");
    trackBitrate = new QLabel(this);
    trackBitrate->setAlignment(Qt::AlignRight);
    trackBitrate->setStyleSheet("font-weight: bold");
    // Add track slider and the volume knob for Qwt.
    sliderWidget = new xPlayerSliderWidget(this);
    connect(sliderWidget, &xPlayerSliderWidget::seek, musicPlayer, &xMusicPlayer::seek);
    // Add a control tab for player and rotel amp controls
    auto controlTab = new QTabWidget(this);
    auto controlTabPlayer = new QWidget(this);
    controlTabRotel = new xPlayerRotelWidget(this);
    controlTab->setTabPosition(QTabWidget::West);
    controlTab->addTab(controlTabPlayer, "xPlay");
    controlTab->addTab(controlTabRotel, "Rotel");
    controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    // Create control buttons.
    controlButtonWidget = new xPlayerControlButtonWidget(xPlayerControlButtonWidget::MusicPlayerMode, controlTabPlayer);
    volumeWidget = new xPlayerVolumeWidget(true, controlTabPlayer);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, musicPlayer, &xMusicPlayer::setVolume);
    connect(volumeWidget, &xPlayerVolumeWidget::muted, musicPlayer, &xMusicPlayer::setMuted);
    connect(musicPlayer, &xMusicPlayer::currentVolume, volumeWidget, &xPlayerVolumeWidget::setVolume);
    connect(musicPlayer, &xMusicPlayer::currentTrackPlayed, sliderWidget, &xPlayerSliderWidget::trackPlayed);
    connect(musicPlayer, &xMusicPlayer::currentTrackLength, sliderWidget, &xPlayerSliderWidget::trackLength);
    // Create the basic player widget layout.
    // Add labels, buttons and the slider.
    auto playerLayout = new xPlayerLayout(this);
    playerLayout->setSpacing(xPlayerLayout::NoSpace);
    playerLayout->addWidget(artistLabel, 0, 0);
    playerLayout->addWidget(artistName, 0, 1, 1, 4);
    playerLayout->addWidget(albumLabel, 1, 0);
    playerLayout->addWidget(albumName, 1, 1, 1, 4);
    playerLayout->addWidget(trackLabel, 2, 0);
    playerLayout->addWidget(trackName, 2, 1, 1, 4);
    auto trackSampleRateLabel = new QLabel(tr("Sample rate"));
    trackSampleRateLabel->setAlignment(Qt::AlignRight);
    auto trackBitrateLabel = new QLabel(tr("Bitrate"));
    trackBitrateLabel->setAlignment(Qt::AlignRight);
    playerLayout->addWidget(trackSampleRateLabel, 0, 5);
    playerLayout->addWidget(trackSampleRate, 0, 6);
    playerLayout->addWidget(trackBitrateLabel, 2, 5);
    playerLayout->addWidget(trackBitsPerSample, 1, 6);
    playerLayout->addWidget(trackBitrate, 2, 6);
    playerLayout->addWidget(sliderWidget, 4, 0, 1, 7);
    // Create a layout for the music player and playlist control buttons.
    auto controlLayout = new xPlayerLayout(controlTabPlayer);
    controlLayout->setSpacing(xPlayerLayout::NoSpace);
    controlLayout->addWidget(volumeWidget, 0, 0);
    controlLayout->addColumnSpacer(1, xPlayerLayout::MediumSpace);
    controlLayout->addWidget(controlButtonWidget, 0, 2);
    // Fix width of the control tab
    controlTabPlayer->setFixedWidth(controlTabPlayer->sizeHint().width());
    controlTabRotel->setFixedWidth(controlTabPlayer->sizeHint().width());
    controlTab->setFixedWidth(controlTab->sizeHint().width());
    // Add the control tab to the player layout.
    playerLayout->addColumnSpacer(7, xPlayerLayout::SeparatorSpace);
    playerLayout->addWidget(controlTab, 0, 8, 5, 1);
    // Connect the buttons to player widget and/or to the music player.
    connect(controlButtonWidget, &xPlayerControlButtonWidget::playPausePressed, musicPlayer, &xMusicPlayer::playPause);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::stopPressed, musicPlayer, &xMusicPlayer::stop);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::previousPressed, musicPlayer, &xMusicPlayer::prev);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::nextPressed, musicPlayer, &xMusicPlayer::next);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::rewindPressed, [=]() {
        musicPlayer->jump(-xMusicPlayer::ForwardRewindDelta);
    });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::forwardPressed, [=]() {
        musicPlayer->jump(xMusicPlayer::ForwardRewindDelta);
    });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::clearPressed, musicPlayer, &xMusicPlayer::clearQueue);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::clearPressed, this, &xPlayerMusicWidget::clearQueue);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::clearPressed, this, &xPlayerMusicWidget::clear);
    // Connect the music player to the player widget.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xPlayerMusicWidget::currentTrack);
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xPlayerMusicWidget::currentState);
    // Connect Rotel amp widget configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelWidget, [=]() {
        controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    });
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedUseMusicLibraryBluOS, [=]() {
        volumeWidget->setVolume(musicPlayer->getVolume());
    });
    // Do not resize the player widget vertically
    //setFixedHeight(sizeHint().height());
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

void xPlayerMusicWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if ((event) && (event->button() == Qt::LeftButton)) {
        emit mouseDoubleClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void xPlayerMusicWidget::currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                                      int bitrate, int sampleRate, int bitsPerSample) {
    Q_UNUSED(index)
    // Display the current track information (without length)
    artistName->setText(artist);
    albumName->setText(album);
    trackName->setText(track);
    // sample rate, bits per sample and bitrate may not be available for BluOS player tracks.
    if ((sampleRate > 0) && (bitsPerSample > 0)) {
        trackSampleRate->setText(QString("%1Hz").arg(sampleRate));
        trackBitsPerSample->setText(QString("%1-bit").arg(bitsPerSample));
    } else {
        trackSampleRate->setText("n/a");
        trackBitsPerSample->setText("");

    }
    if (bitrate > 0) {
        trackBitrate->setText(QString("%1 kbit/s").arg(bitrate));
    } else {
        trackBitrate->setText("n/a");
    }
}

void xPlayerMusicWidget::currentState(xMusicPlayer::State state) {
    // Update the play/pause state based on the state of the music player.
    switch (state) {
        case xMusicPlayer::PlayingState: {
            // Show "pause" if in play state.
            controlButtonWidget->setPlayPauseState(false);
        } break;
        case xMusicPlayer::PauseState:
        case xMusicPlayer::StopState: {
            // Show "play" if in pause or stop state.
            controlButtonWidget->setPlayPauseState(true);
        } break;
    }
}

