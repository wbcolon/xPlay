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
#include "xMusicPlayerQt.h"

#include <QMediaControl>
#include <QMediaService>
#include <QMediaGaplessPlaybackControl>

xMusicPlayerQt::xMusicPlayerQt(QObject* parent):
        xMusicPlayer(parent) {
    // Setup the media player.
    musicPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
    // Setup gapless play (does not seem to work).
    auto musicPlayerService = musicPlayer->service();
    auto *musicPlayerControl = static_cast<QMediaGaplessPlaybackControl *>(qobject_cast<QMediaControl*>(musicPlayerService->requestControl("org.qt-project.qt.mediaplayercontrol/5.0")));
    musicPlayerControl->setCrossfadeTime(0);
    // Setup the play list.
    musicPlaylist = new QMediaPlaylist();
    musicPlaylist->setPlaybackMode(QMediaPlaylist::Sequential);
    musicPlayer->setPlaylist(musicPlaylist);
    // Connect QMediaPlayer signals to out music player signals.
    connect(musicPlayer, &QMediaPlayer::durationChanged, this, &xMusicPlayerQt::currentTrackLength);
    connect(musicPlayer, &QMediaPlayer::positionChanged, this, &xMusicPlayerQt::currentTrackPlayed);
    connect(musicPlaylist, &QMediaPlaylist::currentIndexChanged, this, &xMusicPlayerQt::currentTrackIndex);
}

void xMusicPlayerQt::queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) {
    // Enable auto play if playlist is currently emtpy.
    bool autoPlay = (musicPlaylist->mediaCount() == 0);
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (auto track : tracks) {
        auto queueEntry = std::make_tuple(artist, album, track);
        musicPlaylistEntries.push_back(queueEntry);
        musicPlaylist->addMedia(QUrl::fromLocalFile(pathFromQueueEntry(queueEntry)));
    }
    // Play if autoplay is enabled.
    if (autoPlay) {
        playPause();
    }
}

void xMusicPlayerQt::dequeTrack(int index) {
    // Remove track from the play list.
    musicPlaylist->removeMedia(index);
}

void xMusicPlayerQt::clearQueue() {
    // Clear playlist and musicPlaylistEntries.
    musicPlaylist->clear();
    musicPlaylistEntries.clear();
}

void xMusicPlayerQt::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (musicPlayer->state() == QMediaPlayer::PlayingState) {
        musicPlayer->pause();
        emit currentState(State::PauseState);
    } else {
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerQt::play(int index) {
    // Check if the index is valid.
    if ((index >= 0) && (index < musicPlaylist->mediaCount())) {
        // Play the given entry of the playlist.
        musicPlaylist->setCurrentIndex(index);
    }
}

void xMusicPlayerQt::seek(qint64 position) {
    // Jump to position (in milliseconds) in the current track.
    musicPlayer->setPosition(position);
}

void xMusicPlayerQt::stop() {
    // Stop the media player.
    musicPlayer->stop();
    emit currentState(State::StopState);
}

void xMusicPlayerQt::prev() {
    // Jump to the previous element in the playlist.
    musicPlaylist->previous();
}

void xMusicPlayerQt::next() {
    // Jump to the next element in the playlist.
    musicPlaylist->next();
}

void xMusicPlayerQt::setVolume(int vol) {
    musicPlayer->setVolume(vol);
}

int xMusicPlayerQt::getVolume() {
    return musicPlayer->volume();
}

void xMusicPlayerQt::currentTrackIndex(int index) {
    // Check in the index is valid.
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto entry = musicPlaylistEntries[index];
        auto properties = propertiesFromFile(current.fileName());
        emit currentTrack(index, std::get<0>(entry), std::get<1>(entry),
                std::get<2>(entry), properties.first, properties.second);
    }
}
