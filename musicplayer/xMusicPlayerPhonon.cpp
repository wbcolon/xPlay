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

#include <QRandomGenerator>
#include <cmath>

xMusicPlayerPhonon::xMusicPlayerPhonon(QObject* parent):
        xMusicPlayer(parent),
        musicPlaylistPermutation(),
        useShuffleMode(false) {
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
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (const auto& track : tracks) {
        auto queueEntry = std::make_tuple(artist, album, track);
        musicPlaylistEntries.push_back(queueEntry);
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(pathFromQueueEntry(queueEntry)));
        musicPlaylist.push_back(queueSource);
    }
}

void xMusicPlayerPhonon::finishedQueueTracks() {
    // Enable auto play if playlist is currently emtpy.
    bool autoPlay = (musicPlayer->queue().empty());
    // Check if do not have files in the general queue and are currently stopped.
    bool queueEmpty = (musicPlayer->currentSource().type() == Phonon::MediaSource::Invalid);
    // We need to extend the permutation
    if (useShuffleMode) {
        auto currentIndex = musicPlaylist.indexOf(musicPlayer->currentSource());
        if (currentIndex >= 0) {
            currentIndex = musicPlaylistPermutation.indexOf(currentIndex);
            // Check if we are in the process of filling the queue in shuffle mode.
            if ((currentIndex == 0) && (musicPlayer->state() == Phonon::StoppedState)) {
                // Treat as empty queue;
                currentIndex = -1;
            }
        }
        qDebug() << "CURRENT_INDEX: " << currentIndex;
        // The musicPlaylistPermutation still has the old size.
        if ((currentIndex >= 0) && (currentIndex < musicPlaylistPermutation.count())) {
            // A song was already playing.
            musicPlaylistPermutation = extendPermutation(musicPlaylistPermutation.mid(0, currentIndex+1),
                                                         musicPlaylist.count(), currentIndex);
            for (auto i = currentIndex+1; i < musicPlaylistPermutation.count(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        } else {
            // Clear everything.
            musicPlayer->clear();
            musicPlayer->clearQueue();
            // No current song was playing. Queue possibly empty. No start index.
            musicPlaylistPermutation = computePermutation(musicPlaylist.count(), -1);
            for (auto i = 0; i < musicPlaylistPermutation.count(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        }
    }
    // Play if autoplay is enabled.
    if (autoPlay) {
        if (queueEmpty) {
            musicPlayer->play();
            emit currentState(State::PlayingState);
        } else {
            // Go to next if music player queue is empty but files are in the general queue.
            next();
        }
    }
}

void xMusicPlayerPhonon::dequeTrack(int index) {
    // We do not allow deque tracks if in shuffle mode.
    if (useShuffleMode) {
        return;
    }
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
    // We do not allow clicking on a queued tracks if in shuffle mode.
    if (useShuffleMode) {
        return;
    }
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

void xMusicPlayerPhonon::jump(qint64 delta) {
    // Jump to the current position plus delta (in milliseconds) in the current track.
    auto jumpPosition = musicPlayer->currentTime()+delta;
    musicPlayer->seek(std::clamp(jumpPosition, static_cast<qint64>(0), musicPlayer->totalTime()));
}

void xMusicPlayerPhonon::stop() {
    // Stop the media player.
    musicPlayer->stop();
    emit currentState(State::StopState);
}

void xMusicPlayerPhonon::prev() {
    // Jump to the previous element in the playlist if it exists.
    auto position = musicPlaylist.indexOf(musicPlayer->currentSource());
    // If we are in shuffle mode then we need to find the position in our permutation.
    if (useShuffleMode) {
        position = musicPlaylistPermutation.indexOf(position);
    }
    if (position > 0) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue all tracks starting with position - 1.
        if (useShuffleMode) {
            for (auto i = position - 1; i < musicPlaylist.size(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        } else{
            for (auto i = position - 1; i < musicPlaylist.size(); ++i) {
                musicPlayer->enqueue(musicPlaylist[i]);
            }
        }
        // Play.
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::next() {
    // Jump to the next element in the playlist if it exists.
    auto position = musicPlaylist.indexOf(musicPlayer->currentSource());
    // If we are in shuffle mode then we need to find the position in our permutation.
    if (useShuffleMode) {
        position = musicPlaylistPermutation.indexOf(position);
    }
    if (position < musicPlaylist.size()-1) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue all tracks starting with position + 1.
        if (useShuffleMode) {
            for (auto i = position + 1; i < musicPlaylist.size(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        } else {
            for (auto i = position + 1; i < musicPlaylist.size(); ++i) {
                musicPlayer->enqueue(musicPlaylist[i]);
            }
        }
        // Play.
        musicPlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMusicPlayerPhonon::setMuted(bool mute) {
    musicOutput->setMuted(mute);
}

bool xMusicPlayerPhonon::isMuted() const {
    return musicOutput->isMuted();
}

void xMusicPlayerPhonon::setShuffleMode(bool shuffle) {
    useShuffleMode = shuffle;
    if (useShuffleMode) {
        auto currentIndex = musicPlaylist.indexOf(musicPlayer->currentSource());
        if ((currentIndex >= 0) && (currentIndex < musicPlaylist.count())) {
            musicPlaylistPermutation = computePermutation(musicPlaylist.count(), currentIndex);
            // Do not stop, just clear the queue
            musicPlayer->clearQueue();
            // Remaining tracks include the current
            for (auto i = 1; i < musicPlaylistPermutation.count(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        }
    } else {
        musicPlaylistPermutation.clear();
    }
}

bool xMusicPlayerPhonon::getShuffleMode() const {
    return useShuffleMode;
}

void xMusicPlayerPhonon::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    musicOutput->setVolume(vol/100.0);
}

int xMusicPlayerPhonon::getVolume() const {
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
        emit currentTrack(index, std::get<0>(entry), std::get<1>(entry), std::get<2>(entry),
                std::get<0>(properties), std::get<1>(properties), std::get<2>(properties));
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
            emit currentState(xMusicPlayer::PlayingState);
        }
    }
    if ((newState == Phonon::StoppedState) && (oldState == Phonon::PlayingState)) {
        musicPlayer->stop();
        emit currentState(xMusicPlayer::StopState);
    }
}

void xMusicPlayerPhonon::finished() {
    if (musicPlayer->state() == Phonon::ErrorState) {
        qCritical() << "xMusicPlayerPhonon: finished: error: " << musicPlayer->errorString();
    }
}

QVector<int> xMusicPlayerPhonon::computePermutation(int elements, int startIndex) {
    QList<int> input;
    QVector<int> permutation;
    // Setup the input with all indices.
    for (auto i = 0; i < elements; ++i) {
        input.push_back(i);
    }
    // Remove the start index if we have a valid one.
    if ((startIndex >= 0) && (startIndex < elements)) {
        // Add startIndex to the permutation as first element.
        permutation.push_back(startIndex);
        input.removeOne(startIndex);
        --elements;
    }
    // Choose the remaining elements at random.
    for (auto i = 0; i < elements; ++i) {
        auto index = QRandomGenerator::global()->bounded(input.count());
        permutation.push_back(input[index]);
        input.removeAt(index);
    }
    return permutation;
}

QVector<int> xMusicPlayerPhonon::extendPermutation(const QVector<int>& permutation, int elements, int extendIndex) {
    // Return an empty permutation if we do not extend.
    if (elements < permutation.count()) {
        return QVector<int>();
    }
    QList<int> input;
    QVector<int> ePermutation;
    // Setup the input with all indices.
    for (auto i = 0; i < elements; ++i) {
        input.push_back(i);
    }
    if ((extendIndex >= 0) && (extendIndex < elements)) {
        for (auto i = 0; i < permutation.count(); ++i) {
            // Remove the index from the input that are in the permutation.
            input.removeOne(permutation[i]);
            // Add the removed index to the extended permutation.
            ePermutation.push_back(permutation[i]);
            // We have one less element o choose.
            --elements;
            // End loop if we reached the extendIndex value.
            if (permutation[i] == extendIndex) {
                break;
            }
        }
    }
    // Choose the remaining elements at random.
    for (auto i = 0; i < elements; ++i) {
        auto index = QRandomGenerator::global()->bounded(input.count());
        ePermutation.push_back(input[index]);
        input.removeAt(index);
    }
    return ePermutation;
}
