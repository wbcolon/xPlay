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
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include "xPlayerDatabase.h"
#include "xPlayerBluOSControl.h"

#include <QRandomGenerator>
#include <QAudioOutput>
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
        musicCurrentPosition(-1),
        musicCurrentPlayed(0),
        musicCurrentDuration(-1),
        musicCurrentIndex(-1),
        musicPlayedArtist(),
        musicPlayedAlbum(),
        musicPlayedIndex(-1),
        musicPlayed(-1),
        musicPlayedRecorded(false),
        musicCurrentRemote(),
        musicRemoteAutoNext(false),
        musicCurrentFinished(false) {
    pulseAudioControls = xPlayerPulseAudioControls::controls();
    // Set up the media player.
    musicPlayer = new Phonon::MediaObject(this);
    musicOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    // Set stream output volume to 100%
    //musicOutput->setVolume(1.0);
    musicOutput->setVolume(100);
    musicVisualization = new Phonon::AudioDataOutput(this);
    Phonon::createPath(musicPlayer, musicOutput);
    musicVisualizationSupported = Phonon::createPath(musicPlayer, musicVisualization).isValid();
    qDebug() << "xMusicPlayer::xMusicPlayer: visualization support: " << musicVisualizationSupported;
    // Alternate setup, but we need the output to change volume.
    // musicPlayer = Phonon::createPlayer(Phonon::MusicCategory);
    musicPlayer->setTransitionTime(0);
    musicPlayer->setTickInterval(xPlayer::MusicTickDelta);
    musicOutput->setMuted(false);
    musicVisualization->setDataSize(xMusicPlayer_MusicVisualizationSamples);
    // Set up the playlist.
    // Connect QMediaPlayer signals to our music player signals.
    connect(musicPlayer, &Phonon::MediaObject::tick, this, &xMusicPlayer::updatePlayed);
    connect(musicPlayer, &Phonon::MediaObject::currentSourceChanged, this, &xMusicPlayer::currentTrackSource);
    connect(musicPlayer, &Phonon::MediaObject::stateChanged, this, &xMusicPlayer::stateChanged);
    connect(musicPlayer, &Phonon::MediaObject::aboutToFinish, this, &xMusicPlayer::aboutToFinish);
    connect(musicLibrary, &xMusicLibrary::scanningInitialized, this, &xMusicPlayer::reInitialize);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMusicPlayed, [=]() {
        musicPlayed = xPlayerConfiguration::configuration()->getDatabaseMusicPlayed();
    });
    // Connect status update from BluOS player.
    connect(xPlayerBluOSControls::controls(), &xPlayerBluOSControls::playerStatus, this, &xMusicPlayer::playerStatus);
    connect(xPlayerBluOSControls::controls(), &xPlayerBluOSControls::playerStopped, this, &xMusicPlayer::stop);
    // We only need this one to determine the time due to issues with Phonon
    musicPlayerForTime = new QMediaPlayer(this);
    auto audioOutputForTime = new QAudioOutput(this);
    musicPlayerForTime->setAudioOutput(audioOutputForTime);
    // This player is muted. It is only used to determine the proper duration.
    audioOutputForTime->setVolume(0);
    audioOutputForTime->setMuted(true);
    // Connect musicPlayerForTime.
    connect(musicPlayerForTime, &QMediaPlayer::durationChanged, this, &xMusicPlayer::currentTrackDuration);
}

void xMusicPlayer::queueTracks(const QString& artist, const QString& album, const std::vector<xMusicLibraryTrackEntry*>& tracks) {
    if (musicLibrary->isLocal()) {
        // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
        for (const auto& track : tracks) {
            auto queueEntry = std::make_tuple(artist, album, track);
            auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(track->getUrl().toLocalFile()));
            if (queueSource.type() != Phonon::MediaSource::Invalid) {
                musicPlaylistEntries.push_back(queueEntry);
                musicPlaylist.push_back(queueSource);
            }
        }
    } else {
        musicRemoteAutoNext |= (!musicPlaylistRemote.empty()) &&
                (musicPlaylistRemote.constLast() == musicCurrentRemote) &&
                (musicPlayerState == State::StopState);
        // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
        for (const auto& track : tracks) {
            auto queueEntry = std::make_tuple(artist, album, track);
            musicPlaylistEntries.push_back(queueEntry);
            musicPlaylistRemote.push_back(track->getTrackPath());
        }
    }
}

void xMusicPlayer::finishedQueueTracks(bool autoPlay) {
    bool noMedia = false;
    if (musicLibrary->isLocal()) {
        // Enable autoplay if playlist is currently emtpy, and we are in a stopped state.
        autoPlay = autoPlay && ((musicPlayer->queue().empty()) && (musicPlayer->state() == Phonon::StoppedState));
        // Check if there is an invalid or empty file.
        noMedia = ((musicPlayer->currentSource().type() == Phonon::MediaSource::Invalid) ||
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
                musicPlaylistPermutation = extendPermutation(musicPlaylistPermutation.mid(0, currentIndex + 1),
                                                             musicPlaylist.count(), currentIndex);
                // Clear queue and enqueue the remaining entries.
                musicPlayer->clearQueue();
                for (auto i = currentIndex + 1; i < musicPlaylistPermutation.count(); ++i) {
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
            for (auto i = currentIndex + 1; i < static_cast<int>(musicPlaylistEntries.size()); ++i) {
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
    } else {
        auto queue = xPlayerBluOSControls::controls()->queue();
        // Do we autoplay.
        autoPlay = autoPlay && ((queue.empty()) && (musicPlayerState == State::StopState));
        // Add tracks to queue.
        for (auto i = queue.size(); i < musicPlaylistRemote.size(); ++i) {
            xPlayerBluOSControls::controls()->addQueue(musicPlaylistRemote[i]);
        }
        // Play if autoplay is enabled.
        if (musicRemoteAutoNext) {
            emit currentState(musicPlayerState = xMusicPlayer::PlayingState);
            xPlayerBluOSControls::controls()->next();
            musicRemoteAutoNext = false;
        } else {
            if (autoPlay) {
                emit currentState(musicPlayerState = xMusicPlayer::PlayingState);
                xPlayerBluOSControls::controls()->play();
            }
        }
    }
    // Allow shuffle mode to be enabled.
    emit allowShuffleMode(true);
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
                      entryObject->getBitrate(), entryObject->getSampleRate(), entryObject->getBitsPerSample(), {});
    // Update current index.
    musicCurrentIndex = currentIndex;
}

void xMusicPlayer::dequeueTrack(int index) {
    // We do not allow deque tracks if in shuffle mode.
    if (useShuffleMode) {
        return;
    }
    if (musicLibrary->isLocal()) {
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
    } else {
        // Remove the selected track from the remote playlist and entries.
        musicPlaylistRemote.removeAt(index);
        musicPlaylistEntries.erase(musicPlaylistEntries.begin()+index);
        xPlayerBluOSControls::controls()->removeQueue(index);
    }
}

void xMusicPlayer::clearQueue() {
    // Stop the music player and clear its state (including queue).
    emit currentState(musicPlayerState = State::StopState);
    if (musicLibrary->isLocal()) {
        musicPlayer->clearQueue();
        musicPlayer->stop();
        musicPlayer->clear();
        emit allowShuffleMode(true);
    } else {
        xPlayerBluOSControls::controls()->clearQueue();
        xPlayerBluOSControls::controls()->stop();
        emit allowShuffleMode(false);
    }
    // Remove entries from the play lists.
    musicPlaylistEntries.clear();
    musicPlaylistRemote.clear();
    musicPlaylist.clear();
    musicCurrentRemote.clear();
    // Reset played.
    resetPlayed();
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
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(entryObject->getUrl().toLocalFile()));
        if (queueSource.type() != Phonon::MediaSource::Invalid) {
            musicPlaylistEntries.emplace_back(entryArtist, entryAlbum, entryObject);
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
        auto queueSource = Phonon::MediaSource(QUrl::fromLocalFile(entryObject->getUrl().toLocalFile()));
        if (queueSource.type() != Phonon::MediaSource::Invalid) {
            musicPlaylistEntries.emplace_back(entryArtist, entryAlbum, entryObject);
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
            taggedEntries.emplace_back(entryObject->getArtistName(),
                                                       entryObject->getAlbumName(),
                                                       entryObject->getTrackName());
        }
    }
    emit playlist(taggedEntries);
    finishedQueueTracks(false);
}

void xMusicPlayer::saveQueueToPlaylist(const QString& name) {
    // Store the current queue to the database.
    // First convert to database format.
    std::vector<std::tuple<QString,QString,QString>> databasePlaylistEntries;
    for (const auto& [artist, album, trackEntry] : musicPlaylistEntries) {
        databasePlaylistEntries.emplace_back(artist, album, trackEntry->getTrackName());
    }
    auto saved = xPlayerDatabase::database()->updateMusicPlaylist(name, databasePlaylistEntries);
    emit playlistState(name, saved);
}

void xMusicPlayer::reInitialize() {
    if (musicLibrary->isLocal()) {
        emit allowShuffleMode(true);
        emit currentVolume(xPlayerPulseAudioControls::controls()->getVolume());
    } else {
        emit allowShuffleMode(false);
        emit currentVolume(xPlayerBluOSControls::controls()->getVolume());
    }
}

void xMusicPlayer::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (musicLibrary->isLocal()) {
        if (musicPlayer->state() == Phonon::PlayingState) {
            emit currentState(musicPlayerState = State::PauseState);
            musicPlayer->pause();
        } else {
            emit currentState(musicPlayerState = State::PlayingState);
            musicPlayer->play();
        }
    } else {
        auto state = xPlayerBluOSControls::controls()->state();
        if (state == "play") {
            emit currentState(musicPlayerState = State::PauseState);
            xPlayerBluOSControls::controls()->pause();
        } else {
            emit currentState(musicPlayerState = State::PlayingState);
            xPlayerBluOSControls::controls()->play();
        }
    }
}

void xMusicPlayer::play(int index) {
    // We do not allow clicking on a queued tracks if in shuffle mode.
    if (useShuffleMode) {
        return;
    }
    // Reset played.
    resetPlayed();
    if (musicLibrary->isLocal()) {
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
    } else {
        if ((index >= 0) && (index < musicPlaylistRemote.size())) {
            emit currentState(musicPlayerState = State::PlayingState);
            xPlayerBluOSControls::controls()->play(index);
        }
    }
}

void xMusicPlayer::seek(qint64 position) {
    // Update current position.
    musicCurrentPosition = position;
    // Jump to position (in milliseconds) in the current track.
    if (musicLibrary->isLocal()) {
        musicPlayer->seek(position);
    } else {
        xPlayerBluOSControls::controls()->seek(position);
        // Seeking will start the BluOS player.
        emit currentState(musicPlayerState = State::PlayingState);
    }
}

void xMusicPlayer::jump(qint64 delta) {
    // Update current position.
    musicCurrentPosition = std::clamp(musicCurrentPosition+delta,static_cast<qint64>(0), musicCurrentDuration);
    // Jump to the current position plus delta (in milliseconds) in the current track.
    if (musicLibrary->isLocal()) {
        musicPlayer->seek(musicCurrentPosition);
    } else {
        xPlayerBluOSControls::controls()->seek(musicCurrentPosition);
        // Seeking will start the BluOS player.
        emit currentState(musicPlayerState = State::PlayingState);
    }
}

void xMusicPlayer::stop() {
    // Stop the media player.
    emit currentState(musicPlayerState = State::StopState);
    if (musicLibrary->isLocal()) {
        musicPlayer->stop();
    } else {
        xPlayerBluOSControls::controls()->stop();
    }
    // Update current position.
    musicCurrentPosition = 0;
    emit currentTrackPlayed(musicCurrentPosition);
}

void xMusicPlayer::prev() {
    // Reset played. We skip to the previous track.
    resetPlayed();
    if (musicLibrary->isLocal()) {
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
            } else {
                for (auto i = position - 1; i < musicPlaylist.size(); ++i) {
                    musicPlayer->enqueue(musicPlaylist[i]);
                }
            }
            // Play.
            musicPlayer->play();
        }
    } else {
        xPlayerBluOSControls::controls()->prev();
    }
    emit currentState(musicPlayerState = State::PlayingState);
}

void xMusicPlayer::next() {
    // Reset played. We skip to the next track.
    resetPlayed();
    if (musicLibrary->isLocal()) {
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
        }
    } else {
        xPlayerBluOSControls::controls()->next();
    }
    emit currentState(musicPlayerState = State::PlayingState);
}

void xMusicPlayer::setMuted(bool mute) {
    // Mute/Unmute the stream and the pulseaudio sink.
    if (musicLibrary->isLocal()) {
        musicOutput->setMuted(mute);
        pulseAudioControls->setMuted(mute);
    } else {
        xPlayerBluOSControls::controls()->setMuted(mute);
    }
}

bool xMusicPlayer::isMuted() const {
    // The stream and the pulseaudio sink is muted. Use stream state to check.
    if (musicLibrary->isLocal()) {
        return musicOutput->isMuted();
    } else {
        return xPlayerBluOSControls::controls()->isMuted();
    }
}

bool xMusicPlayer::isPlaying() const {
    if (musicLibrary->isLocal()) {
        return musicPlayer->state() == Phonon::PlayingState;
    } else {
        return xPlayerBluOSControls::controls()->state() == "play";
    }
}

void xMusicPlayer::setShuffleMode(bool shuffle) {
    useShuffleMode = shuffle;
    if (musicLibrary->isLocal()) {
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
    } else {
        xPlayerBluOSControls::controls()->setShuffle(useShuffleMode);
    }
}

bool xMusicPlayer::getShuffleMode() const {
    return useShuffleMode;
}

bool xMusicPlayer::isQueueEmpty() const {
    return musicPlaylistEntries.empty();
}

bool xMusicPlayer::isQueueTracksAllowed() const {
    return (!useShuffleMode) || musicLibrary->isLocal();
}

void xMusicPlayer::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    if (musicLibrary->isLocal()) {
        if (!musicOutput->isMuted()) {
            pulseAudioControls->setVolume(vol);
        }
    } else {
        xPlayerBluOSControls::controls()->setVolume(vol);
    }
}

void xMusicPlayer::setVisualization(bool enabled) {
    if (musicLibrary->isLocal()) {
        musicVisualizationEnabled = enabled;
        if (musicVisualizationEnabled) {
            connect(musicVisualization, &Phonon::AudioDataOutput::dataReady, this, &xMusicPlayer::visualizationUpdate);
        } else {
            disconnect(musicVisualization, &Phonon::AudioDataOutput::dataReady, this, &xMusicPlayer::visualizationUpdate);
        }
    }
}

int xMusicPlayer::getVolume() const {
    if (musicLibrary->isLocal()) {
        return pulseAudioControls->getVolume();
    } else {
        return xPlayerBluOSControls::controls()->getVolume();
    }
}

bool xMusicPlayer::supportsVisualization() {
    return musicVisualizationSupported;
}

bool xMusicPlayer::getVisualization() const {
    return musicVisualizationEnabled;
}

void xMusicPlayer::currentTrackDuration(qint64 duration) {
    emit currentTrackLength(duration);
    musicPlayerForTime->stop();
    // Reset media to avoid open files.
    // UPDATA QT6: Segfault if source is set to an empty URL
    // musicPlayerForTime->setSource(QUrl());
    // Update current duration.
    musicCurrentDuration = duration;
}

void xMusicPlayer::updatePlayed(qint64 pos) {
    /* The VLC backend for phonon-kde produces way to many tick update. Ignore most of them. */
    if (std::abs(pos - musicCurrentPosition) < xPlayer::MusicTickDeltaIgnore) {
        return;
    }
    qDebug() << "xMusicPlayer::updatePlayed: pos:" << pos << ", currentPos: " << musicCurrentPosition
        << ", currentPlayed: " << musicCurrentPlayed << ", currentDuration: " << musicCurrentDuration
        << ", playedIndex: " << musicPlayedIndex << ", played: " << musicPlayed;
    if (pos < musicCurrentPosition) {
        qCritical() << "xMusicPlayer::updatePlayed: illegal positions: " << pos << "," << musicCurrentPosition;
        musicCurrentPosition = pos; // correct position.
    }
    musicCurrentPlayed += (pos - musicCurrentPosition);
    musicCurrentPosition = pos;
    // Have we recorded the track already.
    if ((!musicPlayedRecorded) && (musicCurrentDuration >= 0)) {
        // Need to keep track of the played index. The current source may change earlier due to gapless playback.
        if (musicPlayedIndex < 0) {
            musicPlayedIndex = musicCurrentIndex;
        }
        // Determine if we need to update the database.
        auto update = false;
        if ((musicCurrentDuration - 2000) <= musicPlayed) {
            // Update if we are close to the end (within 2 seconds towards the end)
            update = (musicCurrentPosition >= musicCurrentDuration - 2000) && (musicCurrentPosition <= musicCurrentPlayed);
        } else {
            // Update if we played enough of the song.
            update = (musicCurrentPlayed >= musicPlayed);
        }
        if (update) {
            updateDatabase(musicPlayedIndex);
            musicPlayedRecorded = true;
            musicPlayedIndex = -1;
        }
    }
    emit currentTrackPlayed(musicCurrentPosition);
}

void xMusicPlayer::updateDatabase(int index) {
    // Retrieve info for the currently played track and emit the information.
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        auto [artist, album, entryObject] = musicPlaylistEntries[index];
        auto trackName = entryObject->getTrackName();
        // Update database.
        auto result = xPlayerDatabase::database()->updateMusicFile(artist, album, trackName,
                                                                   entryObject->getSampleRate(),
                                                                   entryObject->getBitsPerSample());
        if (result.second > 0) {
            // Update database overlay.
            emit updatePlayedTrack(artist, album, trackName, result.first, result.second);
        }
        // Update transitions
        if (((!musicPlayedArtist.isEmpty()) && (!musicPlayedAlbum.isEmpty())) &&
            ((musicPlayedArtist != artist) || (musicPlayedAlbum != album))) {
            auto transition = xPlayerDatabase::database()->updateTransition(musicPlayedArtist, musicPlayedAlbum,
                                                                            artist, album, useShuffleMode);
            // Currently unused.
            Q_UNUSED(transition)
        }
    }

}

void xMusicPlayer::currentTrackSource(const Phonon::MediaSource& current) {
    // Check in the index is valid.
    auto index = musicPlaylist.indexOf(current);
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto [artist, album, entryObject] = musicPlaylistEntries[index];

        emit currentTrack(index, artist, album, entryObject->getTrackName(), entryObject->getBitrate(),
                          entryObject->getSampleRate(), entryObject->getBitsPerSample(), {});
        // Save sample rate for music visualization scaling
        musicVisualizationSampleRate = entryObject->getSampleRate() / xMusicPlayer_MusicVisualizationSamplesFactor;
        // Use hack to determine the proper total length.
        // We need the muted musicPlayerForTime to play until the total time has been determined
        // and the durationChanged signal was triggered.
        musicPlayerForTime->setSource(QUrl::fromLocalFile(current.fileName()));
        musicPlayerForTime->play();
        musicCurrentFinished = false;
        // Update current index.
        musicCurrentIndex = index;
        // Reset played. We have a new track.
        resetPlayed();
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

void xMusicPlayer::playerStatus(const QString& path, int index, qint64 position, const QString& quality) {
    // Translate updates from the BluOS player. Index will be incorrect for shuffle mode.
    if (musicPlaylistRemote.value(index) != path) {
        index = musicPlaylistRemote.indexOf(path);
    }
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        if ((musicCurrentRemote != path) || (musicCurrentIndex != index)) {
            // Update currently played remote track.
            musicCurrentRemote = path;
            auto [artist, album, entryObject] = musicPlaylistEntries[index];
            // Reset played. We have a new track.
            resetPlayed();
            musicCurrentDuration = entryObject->getLength();
            emit currentTrackLength(musicCurrentDuration);
            emit currentTrack(index, artist, album, entryObject->getTrackName(), entryObject->getBitrate(),
                              entryObject->getSampleRate(), entryObject->getBitsPerSample(), quality);
            // Update current index.
            musicCurrentIndex = index;
        }
        if (!musicPlayedRecorded) {
            // Update played.
            updatePlayed(position);
        }
        emit currentTrackPlayed(position);
    }
}

void xMusicPlayer::resetPlayed() {
    musicCurrentPosition = 0;
    musicCurrentPlayed = 0;
    musicCurrentDuration = -1;
    musicPlayedRecorded = false;
    musicPlayedIndex = -1;
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
