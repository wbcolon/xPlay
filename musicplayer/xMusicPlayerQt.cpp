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
#include "xPlayerDatabase.h"

#include <QMediaControl>
#include <QMediaService>
#include <QMediaGaplessPlaybackControl>

xMusicPlayerQt::xMusicPlayerQt(QObject* parent):
        xMusicPlayer(parent) {
    // Setup the media player.
    musicPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
    musicPlayer->setMuted(false);
    // Setup gap-less play (does not seem to work).
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
    connect(musicPlayer, &QMediaPlayer::stateChanged, this, &xMusicPlayerQt::currentStateChanged);
    connect(musicPlaylist, &QMediaPlaylist::currentIndexChanged, this, &xMusicPlayerQt::currentTrackIndex);
}

void xMusicPlayerQt::queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) {
    // Do we really add any tracks.
    if (tracks.size() == 0) {
        return;
    }
    // Enable auto play if playlist is currently emtpy.
    bool autoPlay = (musicPlaylist->mediaCount() == 0) ||
            ((musicPlaylist->currentIndex() == -1) && (musicPlayer->state() == QMediaPlayer::StoppedState));
    // Check if do not have files in the general queue.
    auto queueEntries = musicPlaylistEntries.size();
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (const auto& track : tracks) {
        auto queueEntry = std::make_tuple(artist, album, track);
        musicPlaylistEntries.push_back(queueEntry);
        musicPlaylist->addMedia(QUrl::fromLocalFile(pathFromQueueEntry(queueEntry)));
    }
    // Play if autoplay is enabled.
    if (autoPlay) {
        if (queueEntries > 0) {
            musicPlaylist->setCurrentIndex(queueEntries);
        }
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerQt::finishedQueueTracks() {
    // No need to do anything here.
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

void xMusicPlayerQt::loadQueueFromPlaylist(const QString& name) {
    // Load the playlist from the database.
    auto playlistEntries = xPlayerDatabase::database()->getMusicPlaylist(name);
    clearQueue();
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (const auto& entry : playlistEntries) {
        musicPlaylist->addMedia(QUrl::fromLocalFile(pathFromQueueEntry(entry)));
    }
    emit playlist(playlistEntries);
}

void xMusicPlayerQt::saveQueueToPlaylist(const QString& name) {
    // Store the current queue to the database.
    auto saved = xPlayerDatabase::database()->updateMusicPlaylist(name, musicPlaylistEntries);
    emit playlistState(name, saved);
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

void xMusicPlayerQt::jump(qint64 delta) {
    // Jump to the current position plus delta (in milliseconds) in the current track.
    auto jumpPosition = musicPlayer->position()+delta;
    musicPlayer->setPosition(std::clamp(jumpPosition, static_cast<qint64>(0), musicPlayer->duration()));
}

void xMusicPlayerQt::stop() {
    // Stop the media player.
    musicPlayer->stop();
    emit currentState(State::StopState);
}

void xMusicPlayerQt::prev() {
    // Jump to the previous element in the playlist.
    musicPlaylist->previous();
    musicPlayer->play();
    emit currentState(State::PlayingState);
}

void xMusicPlayerQt::next() {
    // Jump to the next element in the playlist.
    musicPlaylist->next();
    musicPlayer->play();
    emit currentState(State::PlayingState);
}

void xMusicPlayerQt::setMuted(bool mute) {
    musicPlayer->setMuted(mute);
}

bool xMusicPlayerQt::isMuted() const {
    return musicPlayer->isMuted();
}

void xMusicPlayerQt::setShuffleMode(bool shuffle) {
    useShuffleMode = shuffle;
    if (useShuffleMode) {
        musicPlaylist->setPlaybackMode(QMediaPlaylist::Random);
    } else {
        musicPlaylist->setPlaybackMode(QMediaPlaylist::Sequential);
    }
}

bool xMusicPlayerQt::getShuffleMode() const {
    return useShuffleMode;
}

void xMusicPlayerQt::setVolume(int vol) {
    musicPlayer->setVolume(vol);
}

int xMusicPlayerQt::getVolume() const {
    return musicPlayer->volume();
}

void xMusicPlayerQt::currentTrackIndex(int index) {
    // Check in the index is valid.
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto entry = musicPlaylistEntries[index];
        auto current = baseDirectory+"/"+std::get<0>(entry)+"/"+std::get<1>(entry)+"/"+std::get<2>(entry);
        auto properties = propertiesFromFile(current);
        emit currentTrack(index, std::get<0>(entry), std::get<1>(entry), std::get<2>(entry),
                std::get<0>(properties), std::get<1>(properties), std::get<2>(properties));
    }
}

void xMusicPlayerQt::currentStateChanged(QMediaPlayer::State newState) {
    if (newState == QMediaPlayer::StoppedState) {
        musicPlayer->stop();
        emit currentState(State::StopState);
    }
   qDebug() << "xMusicPlayerQt: State: " << newState;
}
