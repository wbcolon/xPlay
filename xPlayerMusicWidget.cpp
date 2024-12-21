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

#include <QFileInfo>
#include <QTabWidget>
#include <QMouseEvent>
#include <taglib/fileref.h>

xPlayerMusicWidget::xPlayerMusicWidget(xMusicPlayer* player, QWidget* parent, Qt::WindowFlags flags):
    QWidget(parent, flags),
    musicPlayer(player) {

    infoStacked = new QStackedWidget(this);
    infoPlayer = new QWidget(infoStacked);

    // Create labels for artist, album and track
    auto artistLabel = new QLabel(tr("Artist"), infoPlayer);
    artistName = new QLabel(infoPlayer);
    artistName->setWordWrap(true);
    artistName->setStyleSheet("font-weight: bold");
    auto albumLabel = new QLabel(tr("Album"), infoPlayer);
    albumName = new QLabel(infoPlayer);
    albumName->setWordWrap(true);
    albumName->setStyleSheet("font-weight: bold");
    auto trackLabel = new QLabel(tr("Track"), infoPlayer);
    trackName = new QLabel(infoPlayer);
    trackName->setWordWrap(true);
    trackName->setStyleSheet("font-weight: bold");
    trackSampleRate = new QLabel(infoPlayer);
    trackSampleRate->setAlignment(Qt::AlignRight);
    trackSampleRate->setStyleSheet("font-weight: bold");
    trackBitsPerSample = new QLabel(infoPlayer);
    trackBitsPerSample->setAlignment(Qt::AlignRight|Qt::AlignTop);
    trackBitsPerSample->setStyleSheet("font-weight: bold");
    trackBitrate = new QLabel(infoPlayer);
    trackBitrate->setAlignment(Qt::AlignRight);
    trackBitrate->setStyleSheet("font-weight: bold");
    // Add track slider and the volume knob for Qwt.
    sliderWidget = new xPlayerSliderWidget(infoPlayer);
    connect(sliderWidget, &xPlayerSliderWidget::seek, musicPlayer, &xMusicPlayer::seek);
    // Create label for info mode.
    infoMode = new QLabel(infoStacked);
    infoMode->setAlignment(Qt::AlignCenter);
    //infoMode->setPixmap(QPixmap(":images/xplay-bluesound.png").scaledToHeight(6*xPlayer::LargeIconSize, Qt::SmoothTransformation));
    infoMode->setPixmap(QPixmap(":images/xplay-music-folder.png").scaledToHeight(4*xPlayer::LargeIconSize, Qt::SmoothTransformation));
    // Add player and mode info to stacked widget.
    infoStacked->addWidget(infoMode);
    infoStacked->addWidget(infoPlayer);
    infoStacked->setCurrentWidget(infoMode);
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
    volumeWidget = new xPlayerVolumeWidget(controlTabPlayer);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, musicPlayer, &xMusicPlayer::setVolume);
    connect(volumeWidget, &xPlayerVolumeWidget::muted, musicPlayer, &xMusicPlayer::setMuted);
    connect(musicPlayer, &xMusicPlayer::currentVolume, volumeWidget, &xPlayerVolumeWidget::setVolume);
    connect(musicPlayer, &xMusicPlayer::currentTrackPlayed, sliderWidget, &xPlayerSliderWidget::setPlayed);
    connect(musicPlayer, &xMusicPlayer::currentTrackLength, sliderWidget, &xPlayerSliderWidget::setLength);
    // Create the basic player widget layout.
    // Add labels, buttons and the slider.
    auto infoLayout = new xPlayerLayout(infoPlayer);
    infoLayout->setSpacing(xPlayerLayout::NoSpace);
    infoLayout->addWidget(artistLabel, 0, 0);
    infoLayout->addWidget(artistName, 0, 1, 1, 4);
    infoLayout->addWidget(albumLabel, 1, 0);
    infoLayout->addWidget(albumName, 1, 1, 1, 4);
    infoLayout->addWidget(trackLabel, 2, 0);
    infoLayout->addWidget(trackName, 2, 1, 1, 4);
    trackSampleRateLabel = new QLabel(infoPlayer);
    trackSampleRateLabel->setAlignment(Qt::AlignRight);
    trackBitrateLabel = new QLabel(infoPlayer);
    trackBitrateLabel->setAlignment(Qt::AlignRight);
    infoLayout->addWidget(trackSampleRateLabel, 0, 5);
    infoLayout->addWidget(trackSampleRate, 0, 6);
    infoLayout->addWidget(trackBitrateLabel, 2, 5);
    infoLayout->addWidget(trackBitsPerSample, 1, 6);
    infoLayout->addWidget(trackBitrate, 2, 6);
    infoLayout->addRowSpacer(3, xPlayerLayout::SeparatorSpace);
    infoLayout->addWidget(sliderWidget, 4, 0, 1, 7);
    infoLayout->addColumnSpacer(7, xPlayerLayout::SeparatorSpace);

    auto playerLayout = new xPlayerLayout(this);
    playerLayout->addWidget(infoStacked, 0, 0, 5, 8);

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
    playerLayout->addWidget(controlTab, 0, 8, 5, 1);
    // Connect the buttons to player widget and/or to the music player.
    connect(controlButtonWidget, &xPlayerControlButtonWidget::playPausePressed, musicPlayer, &xMusicPlayer::playPause);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::stopPressed, musicPlayer, &xMusicPlayer::stop);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::previousPressed, musicPlayer, &xMusicPlayer::prev);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::nextPressed, musicPlayer, &xMusicPlayer::next);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::rewindPressed, [=]() {
        musicPlayer->jump(-xPlayer::MusicForwardRewindDelta);
    });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::forwardPressed, [=]() {
        musicPlayer->jump(xPlayer::MusicForwardRewindDelta);
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
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedUseMusicLibraryBluOS,
            this, &xPlayerMusicWidget::updateInfoMode);
    // Assign BluOS Pixmax. Resize to proper size.
    trackBluOS = QPixmap(":images/xplay-bluos.png").scaledToHeight(xPlayer::LargeIconSize, Qt::SmoothTransformation);
    // Do not resize the player widget vertically
    setFixedHeight(sizeHint().height());
}

void xPlayerMusicWidget::clear() {
    // Switch to info mode.
    infoStacked->setCurrentWidget(infoMode);
    // Reset the play/pause button and clear all track info.
    artistName->clear();
    albumName->clear();
    trackName->clear();
    trackSampleRateLabel->clear();
    trackSampleRate->clear();
    trackBitsPerSample->clear();
    trackBitrateLabel->clear();
    trackBitrate->clear();
    sliderWidget->clear();
}

void xPlayerMusicWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if ((event) && (event->button() == Qt::LeftButton)) {
        emit mouseDoubleClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void xPlayerMusicWidget::updateInfoMode() {
    if (xPlayerConfiguration::configuration()->useMusicLibraryBluOS()) {
        infoMode->setPixmap(QPixmap(":images/xplay-bluesound.png").scaledToHeight(5*xPlayer::LargeIconSize, Qt::SmoothTransformation));
    } else {
        infoMode->setPixmap(QPixmap(":images/xplay-music-folder.png").scaledToHeight(4*xPlayer::LargeIconSize, Qt::SmoothTransformation));
    }
}

void xPlayerMusicWidget::currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                                      int bitrate, int sampleRate, int bitsPerSample, const QString& quality) {
    Q_UNUSED(index)
    // Display the current track information (without length)
    artistName->setText(artist);
    albumName->setText(album);
    trackName->setText(track);
    // BluOS player status does not contain sample rate, bits per sample, but contains a quality string.
    if ((sampleRate > 0) && (bitsPerSample > 0)) {
        // Only display the sample rate if given.
        trackSampleRateLabel->setText(tr("Sample rate"));
        trackSampleRate->setText(QString("%1Hz").arg(sampleRate));
        trackBitsPerSample->setText(QString("%1-bit").arg(bitsPerSample));
    } else {
        // The quality string can be either a bit/s value or quality type such as cd or hd.
        trackSampleRateLabel->setText(tr("Quality"));
        bool qualityIsBitrate = false;
        quality.toInt(&qualityIsBitrate);
        if (qualityIsBitrate) {
            trackSampleRate->setText(QFileInfo(track).suffix().toLower());
        } else {
            trackSampleRate->setText(quality.toLower());
        }
        trackBitsPerSample->clear();
    }
    if (bitrate > 0) {
        // Only display bitrate if given.
        trackBitrateLabel->setText(tr("Bitrate"));
        trackBitrate->setText(QString("%1 kbit/s").arg(bitrate));
    } else {
        trackBitrateLabel->clear();
        trackBitrate->clear();
        trackBitrate->setPixmap(trackBluOS);
    }
}

void xPlayerMusicWidget::currentState(xMusicPlayer::State state) {
    // Change to player info if queue is not empty.
    if (!musicPlayer->isQueueEmpty()) {
        infoStacked->setCurrentWidget(infoPlayer);
    }
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

