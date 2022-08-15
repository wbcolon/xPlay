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
#include "xMusicPlayer.h"
#include "xMusicLibrary.h"
#include "xMusicLibraryTrackEntry.h"
#include "xPlayerPulseAudioControls.h"
#include "xPlayerDatabase.h"

#include <QRandomGenerator>
#include <cmath>

constexpr auto xMusicPlayer_MusicVisualizationSamples = 1024;
constexpr auto xMusicPlayer_MusicVisualizationSamplesFactor = xMusicPlayer_MusicVisualizationSamples * 10;

xMusicPlayer::xMusicPlayer(xMusicLibrary* library, QObject* parent):
        QObject(parent),
        musicLibrary(library),
        musicPlaylistPermutation(),
        musicVisualizationEnabled(false),
        musicVisualizationSampleRate(44100 / xMusicPlayer_MusicVisualizationSamplesFactor),
        musicPlayerState(State::StopState),
        useShuffleMode(false),
        musicCurrentFinished(false) {
    // Set up the media player.
    musicPlayer = new Phonon::MediaObject(this);
    musicOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    // Set stream output volume to 100%
    musicOutput->setVolume(1.0);
    musicVisualization = new Phonon::AudioDataOutput(this);
    Phonon::createPath(musicPlayer, musicOutput);
    Phonon::createPath(musicPlayer, musicVisualization);
    // Alternate setup, but we need the output to change volume.
    // musicPlayer = Phonon::createPlayer(Phonon::MusicCategory);
    musicPlayer->setTransitionTime(0);
    musicPlayer->setTickInterval(500);
    musicOutput->setMuted(false);
    musicVisualization->setDataSize(xMusicPlayer_MusicVisualizationSamples);
    // Set up the play list.
    // Connect QMediaPlayer signals to out music player signals.
    connect(musicPlayer, &Phonon::MediaObject::tick, this, &xMusicPlayer::currentTrackPlayed);
    connect(musicPlayer, &Phonon::MediaObject::currentSourceChanged, this, &xMusicPlayer::currentTrackSource);
    connect(musicPlayer, &Phonon::MediaObject::stateChanged, this, &xMusicPlayer::stateChanged);
    connect(musicPlayer, &Phonon::MediaObject::aboutToFinish, this, &xMusicPlayer::aboutToFinish);

    // We only need this one to determine the time due to issues with Phonon
    musicPlayerForTime = new QMediaPlayer(this);
    // This player is muted. It is only used to determine the proper duration.
    musicPlayerForTime->setVolume(0);
    musicPlayerForTime->setMuted(true);
    // Connect musicPlayerForTime.
    connect(musicPlayerForTime, &QMediaPlayer::durationChanged, this, &xMusicPlayer::currentTrackDuration);
}

void xMusicPlayer::queueTracks(const QString& artist, const QString& album, const std::vector<xMusicLibraryTrackEntry*>& tracks) {
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (const auto& track : tracks) {
        auto queueEntry = std::make_tuple(artist, album, track);
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(QString::fromStdString(track->getPath().generic_string())));
        if (queueSource.type() != Phonon::MediaSource::Invalid) {
            musicPlaylistEntries.push_back(queueEntry);
            musicPlaylist.push_back(queueSource);
        }
    }
}

void xMusicPlayer::finishedQueueTracks(bool autoPlay) {
    // Enable autoplay if playlist is currently emtpy, and we are in a stopped state.
    autoPlay = autoPlay && ((musicPlayer->queue().empty()) && (musicPlayer->state() == Phonon::StoppedState));
    // Check if there is an invalid or empty file.
    bool noMedia = ((musicPlayer->currentSource().type() == Phonon::MediaSource::Invalid) ||
                    (musicPlayer->currentSource().type() == Phonon::MediaSource::Empty));
    // Find the index of the current media source in the playlist.
    auto currentIndex = musicPlaylist.indexOf(musicPlayer->currentSource());
    if (useShuffleMode) {
        if (currentIndex >= 0) {
            currentIndex = musicPlaylistPermutation.indexOf(currentIndex);
            // Check if we are in the process of filling the queue in shuffle mode.
            if ((currentIndex == 0) && (musicPlayer->state() == Phonon::StoppedState)) {
                // Treat as empty queue;
                currentIndex = -1;
            }
        }
        // The musicPlaylistPermutation still has the old size.
        if ((currentIndex >= 0) && (currentIndex < musicPlaylistPermutation.count())) {
            // A song was already playing.
            musicPlaylistPermutation = extendPermutation(musicPlaylistPermutation.mid(0, currentIndex+1),
                                                         musicPlaylist.count(), currentIndex);
            // Clear queue and enqueue the remaining entries.
            musicPlayer->clearQueue();
            for (auto i = currentIndex+1; i < musicPlaylistPermutation.count(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        } else {
            // No current song was playing. Queue possibly empty. No start index given.
            musicPlaylistPermutation = computePermutation(musicPlaylist.count(), -1);
            // Clear queue and enqueue all entries.
            musicPlayer->clearQueue();
            for (auto i = 0; i < musicPlaylistPermutation.count(); ++i) {
                musicPlayer->enqueue(musicPlaylist[musicPlaylistPermutation[i]]);
            }
        }
    } else {
        // Enqueue in regular order.
        // If no track currently played and queue empty then currentIndex == -1.
        // Clear queue and enqueue the remaining entries.
        musicPlayer->clearQueue();
        for (auto i = currentIndex+1; i < static_cast<int>(musicPlaylistEntries.size()); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
    }
    // Play if autoplay is enabled.
    if (autoPlay) {
        if (noMedia) {
            emit currentState(musicPlayerState = xMusicPlayer::PlayingState);
            musicPlayer->play();
        } else {
            // Go to next if music player queue is empty but files are in the general queue.
            next();
        }
    }
}

void xMusicPlayer::moveQueueTracks(int fromIndex, int toIndex) {
    if (useShuffleMode) {
        return;
    }
    if (fromIndex == toIndex) {
        return;
    }
    qDebug() << "xMusicPlayer::moveQueueTracks: from " << fromIndex << " to " << toIndex;
    // Move the elements in our list.
    if (fromIndex < toIndex) {
        for (auto index = fromIndex+1; index < toIndex; ++index) {
            std::swap(musicPlaylistEntries[index-1], musicPlaylistEntries[index]);
        }
        // Adjust toIndex since element removed changes to toIndex position.
        musicPlaylist.move(fromIndex, toIndex-1);
    } else {
        for (auto index = fromIndex; index > toIndex; --index) {
            std::swap(musicPlaylistEntries[index], musicPlaylistEntries[index-1]);
        }
        musicPlaylist.move(fromIndex, toIndex);
    }
    // Update playlist.
    auto currentIndex = musicPlaylist.indexOf(musicPlayer->currentSource());
    if ((fromIndex >= currentIndex) || (toIndex >= currentIndex)) {
        // Repopulate playlist if move affected the elements to be played.
        musicPlayer->clearQueue();
        for (auto i = currentIndex+1; i < static_cast<int>(musicPlaylistEntries.size()); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
    }
    // Update queue list.
    auto entryObject = std::get<2>(musicPlaylistEntries[currentIndex]);
    emit currentTrack(currentIndex, entryObject->getArtistName(), entryObject->getAlbumName(), entryObject->getTrackName(),
                      entryObject->getBitrate(), entryObject->getSampleRate(), entryObject->getBitsPerSample());
}

void xMusicPlayer::dequeTrack(int index) {
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

void xMusicPlayer::clearQueue() {
    // Stop the music player and clear its state (including queue).
    emit currentState(musicPlayerState = State::StopState);
    musicPlayer->stop();
    musicPlayer->clear();
    // Remove entries from the play lists.
    musicPlaylistEntries.clear();
    musicPlaylist.clear();
}

void xMusicPlayer::loadQueueFromPlaylist(const QString& name) {
    // Load the playlist from the database.
    auto playlistEntries = xPlayerDatabase::database()->getMusicPlaylist(name);
    clearQueue();
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (auto entry = playlistEntries.begin(); entry != playlistEntries.end(); ++entry) {
        // Split up tuple
        const auto& [entryArtist, entryAlbum, entryTrackName] = *entry;
        auto entryObject = musicLibrary->getTrackEntry(entryArtist, entryAlbum, entryTrackName);
        if (entryObject == nullptr) {
            // Remove invalid entries from the list.
            playlistEntries.erase(entry);
            continue;
        }
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(QString::fromStdString(entryObject->getPath().generic_string())));
        if (queueSource.type() != Phonon::MediaSource::Invalid) {
            musicPlaylistEntries.emplace_back(std::make_tuple(entryArtist, entryAlbum, entryObject));
            musicPlaylist.push_back(queueSource);
            // Enqueue entries.
            musicPlayer->enqueue(queueSource);
        } else {
            // Remove invalid entries from the list.
            playlistEntries.erase(entry);
        }
    }
    emit playlist(playlistEntries);
    finishedQueueTracks(false);
}

void xMusicPlayer::loadQueueFromTag(const QString& tag, bool extend) {
    // Load the playlist from the database.
    auto taggedEntries = xPlayerDatabase::database()->getAllForTag(tag);
    if (!extend) {
        clearQueue();
    }
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (auto entry = taggedEntries.begin(); entry != taggedEntries.end(); ++entry) {
        // Split up tuple
        const auto& [entryArtist, entryAlbum, entryTrackName] = *entry;
        auto entryObject = musicLibrary->getTrackEntry(entryArtist, entryAlbum, entryTrackName);
        if (entryObject == nullptr) {
            // Remove invalid entries from the list.
            taggedEntries.erase(entry);
            continue;
        }
        if ((extend) && (std::find_if(musicPlaylistEntries.begin(), musicPlaylistEntries.end(),
                                      [entryObject](const std::tuple<QString,QString,xMusicLibraryTrackEntry*>& entry) {
                                          return (std::get<2>(entry) == entryObject);
                                      }) != musicPlaylistEntries.end())) {
            continue;
        }
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(QString::fromStdString(entryObject->getPath().generic_string())));
        if (queueSource.type() != Phonon::MediaSource::Invalid) {
            musicPlaylistEntries.emplace_back(std::make_tuple(entryArtist, entryAlbum, entryObject));
            musicPlaylist.push_back(queueSource);
            // Enqueue entries.
            musicPlayer->enqueue(queueSource);
        } else {
            // Remove invalid entries from the list.
            taggedEntries.erase(entry);
        }
    }
    // Update tagged entries if we extend.
    if (extend) {
        taggedEntries.clear();
        for (const auto& entry : musicPlaylistEntries) {
            auto entryObject = std::get<2>(entry);
            taggedEntries.emplace_back(std::make_tuple(entryObject->getArtistName(),
                                                       entryObject->getAlbumName(),
                                                       entryObject->getTrackName()));
        }
    }
    emit playlist(taggedEntries);
    finishedQueueTracks(false);
}

void xMusicPlayer::saveQueueToPlaylist(const QString& name) {
    // Store the current queue to the database.
    // First convert to database format.
    std::vector<std::tuple<QString,QString,QString>> databasePlaylistEntries;
    for (const auto& entry : musicPlaylistEntries) {
        databasePlaylistEntries.emplace_back(std::make_tuple(std::get<0>(entry),
                std::get<1>(entry), std::get<2>(entry)->getTrackName()));
    }
    auto saved = xPlayerDatabase::database()->updateMusicPlaylist(name, databasePlaylistEntries);
    emit playlistState(name, saved);
}

void xMusicPlayer::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (musicPlayer->state() == Phonon::PlayingState) {
        emit currentState(musicPlayerState = State::PauseState);
        musicPlayer->pause();
    } else {
        emit currentState(musicPlayerState = State::PlayingState);
        musicPlayer->play();
    }
}

void xMusicPlayer::play(int index) {
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
        emit currentState(musicPlayerState = State::PlayingState);
        musicPlayer->play();
    }
}

void xMusicPlayer::seek(qint64 position) {
    // Jump to position (in milliseconds) in the current track.
    musicPlayer->seek(position);
}

void xMusicPlayer::jump(qint64 delta) {
    // Jump to the current position plus delta (in milliseconds) in the current track.
    auto jumpPosition = musicPlayer->currentTime()+delta;
    musicPlayer->seek(std::clamp(jumpPosition, static_cast<qint64>(0), musicPlayer->totalTime()));
}

void xMusicPlayer::stop() {
    // Stop the media player.
    emit currentState(musicPlayerState = State::StopState);
    musicPlayer->stop();
}

void xMusicPlayer::prev() {
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
        emit currentState(musicPlayerState = State::PlayingState);
        musicPlayer->play();
    }
}

void xMusicPlayer::next() {
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
        emit currentState(musicPlayerState = State::PlayingState);
        musicPlayer->play();
    }
}

void xMusicPlayer::setMuted(bool mute) {
    // Mute/Unmute the stream and the pulse audio sink.
    musicOutput->setMuted(mute);
    xPlayerPulseAudioControls::controls()->setMuted(mute);
}

bool xMusicPlayer::isMuted() const {
    // The stream and the pulse audio sink is muted.
    return musicOutput->isMuted();
}

bool xMusicPlayer::isPlaying() const {
    return musicPlayer->state() == Phonon::PlayingState;
}

void xMusicPlayer::setShuffleMode(bool shuffle) {
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

bool xMusicPlayer::getShuffleMode() const {
    return useShuffleMode;
}

void xMusicPlayer::setVolume(int vol) {
    if (!musicOutput->isMuted()) {
        vol = std::clamp(vol, 0, 100);
        xPlayerPulseAudioControls::controls()->setVolume(vol);
    }
}

void xMusicPlayer::setVisualization(bool enabled) {
    musicVisualizationEnabled = enabled;
    if (musicVisualizationEnabled) {
        connect(musicVisualization, &Phonon::AudioDataOutput::dataReady, this, &xMusicPlayer::visualizationUpdate);
    } else {
        disconnect(musicVisualization, &Phonon::AudioDataOutput::dataReady, this, &xMusicPlayer::visualizationUpdate);
    }
}

int xMusicPlayer::getVolume() const { // NOLINT
    return xPlayerPulseAudioControls::controls()->getVolume();
}

bool xMusicPlayer::supportsVisualization() {
    return true;
}

bool xMusicPlayer::getVisualization() const {
    return musicVisualizationEnabled;
}

void xMusicPlayer::currentTrackDuration(qint64 duration) {
    emit currentTrackLength(duration);
    musicPlayerForTime->stop();
}

void xMusicPlayer::currentTrackSource(const Phonon::MediaSource& current) {
    // Check in the index is valid.
    auto index = musicPlaylist.indexOf(current);
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto entry = musicPlaylistEntries[index];
        auto entryObject = std::get<2>(entry);
        emit currentTrack(index, std::get<0>(entry), std::get<1>(entry),
                          entryObject->getTrackName(), entryObject->getBitrate(),
                          entryObject->getSampleRate(), entryObject->getBitsPerSample());
        // Save sample rate for music visualization scaling
        musicVisualizationSampleRate = entryObject->getSampleRate() / xMusicPlayer_MusicVisualizationSamplesFactor;
        // Use hack to determine the proper total length.
        // We need the muted musicPlayerForTime to play until the total time has been determined
        // and the durationChanged signal was triggered.
        musicPlayerForTime->setMedia(QUrl::fromLocalFile(current.fileName()));
        musicPlayerForTime->play();
        musicCurrentFinished = false;
    }
}

void xMusicPlayer::stateChanged(Phonon::State newState, Phonon::State oldState) {
    if (newState == Phonon::ErrorState) {
        qCritical() << "xMusicPlayer: error: " << musicPlayer->errorString() << ", track: " << musicPlayer->currentSource();
        if (oldState == Phonon::PlayingState) {
            qInfo() << "xMusicPlayer: trying to recover from error state.";
            play(musicPlaylist.indexOf(musicPlayer->currentSource()));
        }
    } else {
        // Check music player state. Try to recover.
        if ((newState == Phonon::StoppedState) && (oldState == Phonon::PlayingState)) {
            if (musicPlayer->queue().count() == 0) {
                emit currentState(musicPlayerState = xMusicPlayer::StopState);
                musicPlayer->stop();
            } else {
                if (musicPlayerState == State::PlayingState) {
                    if (musicCurrentFinished) {
                        // Queued tracks after the aboutToFinish mark are causing issues.
                        // Phonon stops even though the queue is not empty. Therefor play next track.
                        next();
                    } else {
                        // Actual playback error. We are trying to recover somehow.
                        qInfo() << "xMusicPlayer: trying to recover from state error, current track: " << musicPlayer->currentSource();
                        if (useShuffleMode) {
                            musicPlayer->stop();
                            next();
                        } else {
                            play(musicPlaylist.indexOf(musicPlayer->currentSource()));
                        }
                    }
                }
            }
        }
    }
}

void xMusicPlayer::aboutToFinish() {
    qCritical() << "xMusicPlayer: finished";
    if (musicPlayer->state() == Phonon::ErrorState) {
        qCritical() << "xMusicPlayer: finished: error: " << musicPlayer->errorString();
    } else {
        musicCurrentFinished = true;
    }
}

void xMusicPlayer::visualizationUpdate(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16>>& data) {
    // Use simple counter to drop audio data.
    static int sampleCounter = 0;
    if (sampleCounter++ % musicVisualizationSampleRate) {
        return;
    }
    // Reset sample counter to avoid overflow.
    sampleCounter = 1;
    // Transmit data only if we have any.
    if (data.count() > 0) {
        QVector<qint16> left, right;
        // Only extract the left and right channel.
        left = data[Phonon::AudioDataOutput::LeftChannel];
        right = data[Phonon::AudioDataOutput::RightChannel];
        // Ensure that we have the proper amount of samples.
        left.resize(xMusicPlayer_MusicVisualizationSamples);
        right.resize(xMusicPlayer_MusicVisualizationSamples);
        // Emit signal to visualization.
        emit visualizationStereo(left, right);
    }
}

QVector<int> xMusicPlayer::computePermutation(int elements, int startIndex) {
    QList<int> input;
    QVector<int> permutation;
    // Set up the input with all indices.
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

QVector<int> xMusicPlayer::extendPermutation(const QVector<int>& permutation, int elements, int extendIndex) {
    // Return an empty permutation if we do not extend.
    if (elements < permutation.count()) {
        return QVector<int>{};
    }
    QList<int> input;
    QVector<int> ePermutation;
    // Set up the input with all indices.
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
