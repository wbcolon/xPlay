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
#include "xMusicPlayerPhonon.h"

#include <cmath>

xMusicPlayerPhonon::xMusicPlayerPhonon(QObject* parent):
        xMusicPlayer(parent) {
    // Setup the media player.
    musicPlayer = new Phonon::MediaObject(this);
    musicOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    Phonon::createPath(musicPlayer, musicOutput);
    // Alternate setup, but we need the output to change volume.
    // musicPlayer = Phonon::createPlayer(Phonon::MusicCategory);
    musicPlayer->setTransitionTime(0);
    musicPlayer->setTickInterval(500);
    // Setup the play list.
    // Connect QMediaPlayer signals to out music player signals.
    connect(musicPlayer, &Phonon::MediaObject::tick, this, &xMusicPlayerPhonon::currentTrackPlayed);
    connect(musicPlayer, &Phonon::MediaObject::currentSourceChanged, this, &xMusicPlayerPhonon::currentTrackSource);
    connect(musicPlayer, &Phonon::MediaObject::stateChanged, this, &xMusicPlayerPhonon::stateChanged);
    connect(musicPlayer, &Phonon::MediaObject::finished, this, &xMusicPlayerPhonon::finished);
    // We only need this one to determine the time due to issues with Phonon
    musicPlayerForTime = new QMediaPlayer(this);
    // This player is muted. It is only used to determine the proper duration.
    musicPlayerForTime->setVolume(0);
    musicPlayerForTime->setMuted(true);
    // Connect musicPlayerForTime.
    connect(musicPlayerForTime, &QMediaPlayer::durationChanged, this, &xMusicPlayerPhonon::currentTrackDuration);
}

void xMusicPlayerPhonon::queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) {
    // Enable auto play if playlist is currently emtpy.
    bool autoPlay = (musicPlayer->queue().size() == 0);
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (const auto& track : tracks) {
        auto queueEntry = std::make_tuple(artist, album, track);
        musicPlaylistEntries.push_back(queueEntry);
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(pathFromQueueEntry(queueEntry)));
        musicPlaylist.push_back(queueSource);
        musicPlayer->enqueue(queueSource);
    }
    // Play if autoplay is enabled.
    if (autoPlay) {
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::dequeTrack(int index) {
    // Determine the index of the currently played song.
    auto currentIndex = musicPlaylist.indexOf(musicPlayer->currentSource());
    // Remove the selected track from the playlist and entries.
    musicPlaylist.removeAt(index);
    musicPlaylistEntries.erase(musicPlaylistEntries.begin()+index);
    // Special handling if the current track is to be removed
    // This track is not in the music player playlist. It is its
    // current source. We therefore need to stop and clear everything.
    if (index == currentIndex) {
        // Determine the state of the music player.
        auto state = musicPlayer->state();
        // We need to stop everything because we are deleting the currently played track
        musicPlayer->stop();
        musicPlayer->clear();
        musicPlayer->clearQueue();
        // Remaining tracks include the new current one.
        for (auto i = currentIndex; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // We do not need to signal an update on the state because we did not change it overall.
        if (state == Phonon::PlayingState) {
            musicPlayer->play();
        }
    } else if (index > currentIndex) {
        // Do not stop, just clear the queue
        musicPlayer->clearQueue();
        // Remaining tracks include the current
        for (auto i = currentIndex+1; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
    }
}

void xMusicPlayerPhonon::clearQueue() {
    // Stop the music player and clear its state (including queue).
    musicPlayer->stop();
    emit currentState(State::StopState);
    musicPlayer->clear();
    // Remove entries from the play lists.
    musicPlaylistEntries.clear();
    musicPlaylist.clear();
}

void xMusicPlayerPhonon::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (musicPlayer->state() == Phonon::PlayingState) {
        musicPlayer->pause();
        emit currentState(State::PauseState);
    } else {
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::play(int index) {
    // Check if the index is valid.
    if ((index >= 0) && (index < musicPlaylist.size())) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue the tracks starting at position index.
        for (auto i = index; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // Play.
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::seek(qint64 position) {
    // Jump to position (in milliseconds) in the current track.
    musicPlayer->seek(position);
}

void xMusicPlayerPhonon::stop() {
    // Stop the media player.
    musicPlayer->stop();
    emit currentState(State::StopState);
}

void xMusicPlayerPhonon::prev() {
    // Jump to the previous element in the playlist if it exists.
    auto position = musicPlaylist.indexOf(musicPlayer->currentSource());
    if (position > 0) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue all tracks starting with position - 1.
        for (auto i = position - 1; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // Play.
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::next() {
    // Jump to the next element in the playlist if it exists.
    auto position = musicPlaylist.indexOf(musicPlayer->currentSource());
    if (position < musicPlaylist.size()-1) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue all tracks starting with position + 1.
        for (auto i = position + 1; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // Play.
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    musicOutput->setVolume(vol/100.0);
}

int xMusicPlayerPhonon::getVolume() {
    return static_cast<int>(std::round(musicOutput->volume()*100.0));
}

void xMusicPlayerPhonon::currentTrackDuration(qint64 duration) {
    emit currentTrackLength(duration);
    musicPlayerForTime->stop();
}

void xMusicPlayerPhonon::currentTrackSource(const Phonon::MediaSource& current) {
    // Check in the index is valid.
    auto index = musicPlaylist.indexOf(current);
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto entry = musicPlaylistEntries[index];
        auto properties = propertiesFromFile(current.fileName());
        emit currentTrack(index, std::get<0>(entry), std::get<1>(entry),
                std::get<2>(entry), properties.first, properties.second);
        // Use hack to determine the proper total length.
        // We need the muted musicPlayerForTime to play until the total time has been determined
        // and the durationChanged signal was triggered.
        musicPlayerForTime->setMedia(QUrl::fromLocalFile(current.fileName()));
        musicPlayerForTime->play();
    }
}

void xMusicPlayerPhonon::stateChanged(Phonon::State newState, Phonon::State oldState) {
    if (newState == Phonon::ErrorState) {
        qCritical() << "xMusicPlayerPhonon: error: " << musicPlayer->errorString();
        if (oldState == Phonon::PlayingState) {
            qInfo() << "xMusicPlayerPhonon: trying to recover...";
            musicPlayer->play();
        }
    }
}

void xMusicPlayerPhonon::finished() {
    if (musicPlayer->state() == Phonon::ErrorState) {
        qCritical() << "xMusicPlayerPhonon: finished: error: " << musicPlayer->errorString();
    }
}

