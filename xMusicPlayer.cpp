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
#include <QTimer>
#include <QUrl>
#include <cmath>

xMusicPlayer::xMusicPlayer(xMusicLibrary* library, QObject* parent):
        QObject(parent),
        pulseAudioControls(xPlayerPulseAudioControls::controls()),
        musicLibrary(library),
        musicPlaylistPermutation(),
        musicPlaylistFinished(false),
        musicPlayer(nullptr),
        musicPlayerState(State::StopState),
        useShuffleMode(false),
        musicCurrentPosition(-1),
        musicCurrentPlayed(0),
        musicCurrentDuration(-1),
        musicCurrentIndex(-1),
        musicNextIndex(-1),
        musicPlayedArtist(),
        musicPlayedAlbum(),
        musicPlayedIndex(-1),
        musicPlayed(-1),
        musicPlayedRecorded(false),
        musicCurrentRemote(),
        musicRemoteAutoNext(false),
        musicDiscoverer(nullptr),
        musicTimer(nullptr) {
    // Initialize GStreamer.
    gst_init(nullptr, nullptr);

    // Set up the media player.
    musicPlayer = gst_element_factory_make("playbin3", "playbin3");
    if (!musicPlayer) {
        qCritical() << "Failed to create playbin3 element. Playback not available.";
    }

    // Use PipeWire for audio output. PipeWire negotiates the stream format,
    // including the sample rate, with the configured output device.
    GstElement* pipewireSink = gst_element_factory_make("pipewiresink", "pipewire-audio-sink");
    if (pipewireSink) {
        if (musicPlayer) {
            g_object_set(musicPlayer, "audio-sink", pipewireSink, nullptr);
        }
        gst_object_unref(pipewireSink);
    } else {
        qCritical() << "Failed to create pipewiresink element. Audio output not possible.";
    }

    // Set up GStreamer discoverer.
    GError* error = nullptr;
    musicDiscoverer = gst_discoverer_new(5 * GST_SECOND, &error);
    if (error) {
        qCritical() << "Failed to create discoverer: " << error->message;
        g_error_free(error);
    }

    // Set up timer for position updates.
    musicTimer = new QTimer(this);
    musicTimer->setInterval(xPlayer::MusicTickDelta);
    connect(musicTimer, &QTimer::timeout, this, [this]() {
        if (musicPlayerState == State::PlayingState) {
            gint64 pos;
            if (gst_element_query_position(musicPlayer, GST_FORMAT_TIME, &pos)) {
                updatePlayed(pos / GST_MSECOND);
            }
        }
    });

    // Handle bus messages.
    GstBus* bus = gst_element_get_bus(musicPlayer);
    gst_bus_add_watch(bus, xMusicPlayer::gst_BusWatchCallback, this);
    gst_object_unref(bus);

    // Connect about-to-finish for gapless playback.
    g_signal_connect(musicPlayer, "about-to-finish", G_CALLBACK(xMusicPlayer::gst_AboutToFinishCallback), this);

    connect(musicLibrary, &xMusicLibrary::scanningInitialized, this, &xMusicPlayer::reInitialize);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMusicPlayed, this, [this]() {
        musicPlayed = xPlayerConfiguration::configuration()->getDatabaseMusicPlayed();
    });
    // Connect status update from BluOS player.
    connect(xPlayerBluOSControls::controls(), &xPlayerBluOSControls::playerStatus, this, &xMusicPlayer::playerStatus);
    connect(xPlayerBluOSControls::controls(), &xPlayerBluOSControls::playerStopped, this, &xMusicPlayer::stop);
}

xMusicPlayer::~xMusicPlayer() {
    if (musicPlayer) {
        gst_element_set_state(musicPlayer, GST_STATE_NULL);
        gst_object_unref(musicPlayer);
    }
    if (musicDiscoverer) {
        g_object_unref(musicDiscoverer);
    }
}

void xMusicPlayer::queueTracks(const QString& artist, const QString& album, const std::vector<xMusicLibraryTrackEntry*>& tracks) {
    if (musicLibrary->isLocal()) {
        // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
        for (const auto& track : tracks) {
            auto queueEntry = std::make_tuple(artist, album, track);
            auto queueSource = QUrl::fromLocalFile(track->getUrl().toLocalFile());
            if (queueSource.isValid()) {
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
        // Enable autoplay if playlist is currently empty or finished, and we are in a stopped state.
        autoPlay = autoPlay && (((musicPlaylistFinished) || (musicCurrentIndex == -1)) && musicPlayerState == State::StopState);
        // Check if there is an invalid or empty file.
        noMedia = (musicCurrentIndex == -1);

        if (useShuffleMode) {
            if (musicCurrentIndex >= 0) {
                musicCurrentIndex = musicPlaylistPermutation.indexOf(musicCurrentIndex);
            }
            // The musicPlaylistPermutation still has the old size.
            if ((musicCurrentIndex >= 0) && (musicCurrentIndex < musicPlaylistPermutation.count())) {
                // A song was already playing.
                musicPlaylistPermutation = extendPermutation(musicPlaylistPermutation.mid(0, musicCurrentIndex + 1),
                                                             musicPlaylist.count(), musicCurrentIndex);
            } else {
                // No current song was playing. Queue possibly empty. No start index given.
                musicPlaylistPermutation = computePermutation(musicPlaylist.count(), -1);
            }
        }
        // Play if autoplay is enabled.
        if (autoPlay) {
            if (noMedia) {
                // Play first song. Use permutation if in shuffle mode.
                play(useShuffleMode ? musicPlaylistPermutation[0] : 0);
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
        } else if (autoPlay) {
            emit currentState(musicPlayerState = xMusicPlayer::PlayingState);
            xPlayerBluOSControls::controls()->play();
        }
    }
    // Allow shuffle mode to be enabled.
    emit allowShuffleMode(true);
}

void xMusicPlayer::moveQueueTracks(qsizetype fromIndex, qsizetype toIndex) {
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
    if ((fromIndex >= musicCurrentIndex) || (toIndex >= musicCurrentIndex)) {
        // No action needed for playbin3, it will just continue playing.
    }
    // Update queue list.
    auto entryObject = std::get<2>(musicPlaylistEntries[musicCurrentIndex]);
    emit currentTrack(musicCurrentIndex, entryObject->getArtistName(), entryObject->getAlbumName(), entryObject->getTrackName(),
                      entryObject->getBitrate(), entryObject->getSampleRate(), entryObject->getBitsPerSample(), {});
}

void xMusicPlayer::dequeueTrack(qsizetype index) {
    // We do not allow deque tracks if in shuffle mode.
    if (useShuffleMode) {
        return;
    }
    if (musicLibrary->isLocal()) {
        // Determine the index of the currently played song.
        auto currentIndex = musicCurrentIndex;
        // Remove the selected track from the playlist and entries.
        musicPlaylist.removeAt(index);
        musicPlaylistEntries.erase(musicPlaylistEntries.begin()+index);
        // Special handling if the current track is to be removed
        if (index == currentIndex) {
            // Determine the state of the music player.
            auto state = musicPlayerState;
            // We need to stop everything because we are deleting the currently played track
            stop();
            // We do not need to signal an update on the state because we did not change it overall.
            if (state == State::PlayingState) {
                play(currentIndex);
            }
        } else if (index < currentIndex) {
            musicCurrentIndex--;
        }
    } else {
        // Remove the selected track from the remote playlist and entries.
        musicPlaylistRemote.removeAt(index);
        musicPlaylistEntries.erase(musicPlaylistEntries.begin()+index);
        xPlayerBluOSControls::controls()->removeQueue(index);
    }
}

void xMusicPlayer::clearQueue() {
    // Stop the music player and clear its state.
    emit currentState(musicPlayerState = State::StopState);
    if (musicLibrary->isLocal()) {
        stop();
        emit allowShuffleMode(true);
    } else {
        xPlayerBluOSControls::controls()->clearQueue();
        xPlayerBluOSControls::controls()->stop();
        emit allowShuffleMode(false);
    }
    // Remove entries from the playlists.
    musicPlaylistEntries.clear();
    musicPlaylistRemote.clear();
    musicPlaylist.clear();
    musicCurrentRemote.clear();
    // Reset played.
    resetPlayed();
    // Reset current and next index.
    musicCurrentIndex = -1;
    musicNextIndex = -1;
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
        auto queueSource = QUrl::fromLocalFile(entryObject->getUrl().toLocalFile());
        if (queueSource.isValid()) {
            musicPlaylistEntries.emplace_back(entryArtist, entryAlbum, entryObject);
            musicPlaylist.push_back(queueSource);
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
        auto queueSource = QUrl::fromLocalFile(entryObject->getUrl().toLocalFile());
        if (queueSource.isValid()) {
            musicPlaylistEntries.emplace_back(entryArtist, entryAlbum, entryObject);
            musicPlaylist.push_back(queueSource);
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
        if (musicPlayerState == State::PlayingState) {
            emit currentState(musicPlayerState = State::PauseState);
            gst_element_set_state(musicPlayer, GST_STATE_PAUSED);
            musicTimer->stop();
        } else {
            emit currentState(musicPlayerState = State::PlayingState);
            gst_element_set_state(musicPlayer, GST_STATE_PLAYING);
            musicTimer->start();
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

void xMusicPlayer::play(qsizetype index) {
    resetPlayed();
    if (musicLibrary->isLocal()) {
        // Check if the index is valid.
        if ((index >= 0) && (index < musicPlaylist.size())) {
            // Stop the player.
            gst_element_set_state(musicPlayer, GST_STATE_READY);
            musicTimer->stop();

            musicCurrentIndex = index;
            musicNextIndex = index;
            musicPlaylistFinished = false;
            // Set the URI to the selected track.
            g_object_set(musicPlayer, "uri", musicPlaylist[index].toEncoded().constData(), NULL);

            // Play.
            emit currentState(musicPlayerState = State::PlayingState);
            gst_element_set_state(musicPlayer, GST_STATE_PLAYING);
            musicTimer->start();
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
        gst_element_seek_simple(musicPlayer, GST_FORMAT_TIME,
                                static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                                position * GST_MSECOND);
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
        gst_element_seek_simple(musicPlayer, GST_FORMAT_TIME,
                                static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                                musicCurrentPosition * GST_MSECOND);
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
        gst_element_set_state(musicPlayer, GST_STATE_NULL);
        // gst_element_set_state(musicPlayer, GST_STATE_READY);
        musicTimer->stop();
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
        auto position = musicCurrentIndex;
        // If we are in shuffle mode then we need to find the position in our permutation.
        if (useShuffleMode) {
            position = musicPlaylistPermutation.indexOf(position);
        }
        if (position > 0) {
            // Determine new index.
            if (useShuffleMode) {
                musicCurrentIndex = musicPlaylistPermutation[position - 1];
            } else {
                musicCurrentIndex = position - 1;
            }
            // Play the track.
            play(musicCurrentIndex);
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
        auto position = musicCurrentIndex;
        // If we are in shuffle mode then we need to find the position in our permutation.
        if (useShuffleMode) {
            position = musicPlaylistPermutation.indexOf(position);
        }
        if (position < musicPlaylist.size() - 1) {
            // Determine new index.
            if (useShuffleMode) {
                musicCurrentIndex = musicPlaylistPermutation[position + 1];
            } else {
                musicCurrentIndex = position + 1;
            }
            // Play the track.
            play(musicCurrentIndex);
        } else {
            // Queue is empty, stop playback.
            stop();
            musicPlaylistFinished = true;
        }
    } else {
        xPlayerBluOSControls::controls()->next();
    }
}

void xMusicPlayer::setMuted(bool mute) {
    // Mute/Unmute the stream and the pulseaudio sink.
    if (musicLibrary->isLocal()) {
        g_object_set(musicPlayer, "mute", mute, NULL);
        pulseAudioControls->setMuted(mute);
    } else {
        xPlayerBluOSControls::controls()->setMuted(mute);
    }
}

bool xMusicPlayer::isMuted() const {
    // The stream and the pulseaudio sink is muted. Use stream state to check.
    if (musicLibrary->isLocal()) {
        gboolean mute;
        g_object_get(musicPlayer, "mute", &mute, NULL);
        return mute;
    } else {
        return xPlayerBluOSControls::controls()->isMuted();
    }
}

bool xMusicPlayer::isPlaying() const {
    if (musicLibrary->isLocal()) {
        return musicPlayerState == State::PlayingState;
    } else {
        return xPlayerBluOSControls::controls()->state() == "play";
    }
}

void xMusicPlayer::setShuffleMode(bool shuffle) {
    useShuffleMode = shuffle;
    if (musicLibrary->isLocal()) {
        if (useShuffleMode) {
            auto currentIndex = musicCurrentIndex;
            if ((currentIndex >= 0) && (currentIndex < musicPlaylist.count())) {
                musicPlaylistPermutation = computePermutation(musicPlaylist.count(), currentIndex);
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
    vol = qBound(vol, 0, 100);
    if (musicLibrary->isLocal()) {
        pulseAudioControls->setVolume(vol);
    } else {
        xPlayerBluOSControls::controls()->setVolume(vol);
    }
}

qsizetype xMusicPlayer::getVolume() const {
    if (musicLibrary->isLocal()) {
        return pulseAudioControls->getVolume();
    } else {
        return xPlayerBluOSControls::controls()->getVolume();
    }
}

void xMusicPlayer::currentTrackDuration(qint64 duration) {
    emit currentTrackLength(duration);
    // Update current duration.
    musicCurrentDuration = duration;
}

void xMusicPlayer::updatePlayed(qint64 pos) {
    /* The VLC backend for phonon-kde produces way to many tick updates. Ignore most of them. */
    if (std::abs(pos - musicCurrentPosition) < xPlayer::MusicTickDeltaIgnore) {
        return;
    }
    // qDebug() << "xMusicPlayer::updatePlayed: pos:" << pos << ", currentPos: " << musicCurrentPosition
    //     << ", currentPlayed: " << musicCurrentPlayed << ", currentDuration: " << musicCurrentDuration
    //     << ", playedIndex: " << musicPlayedIndex << ", played: " << musicPlayed;
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

void xMusicPlayer::updateDatabase(qsizetype index) {
    // Retrieve info for the currently played track and emit the information.
    if ((index >= 0) && (index < static_cast<qsizetype>(musicPlaylistEntries.size()))) {
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

void xMusicPlayer::currentTrackSource(const QUrl& current) {
    // Check in the index is valid.
    auto index = musicPlaylist.indexOf(current);
    if ((index >= 0) && (index < static_cast<qsizetype>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto [artist, album, entryObject] = musicPlaylistEntries[index];

        emit currentTrack(index, artist, album, entryObject->getTrackName(), entryObject->getBitrate(),
                          entryObject->getSampleRate(), entryObject->getBitsPerSample(), {});

        // Reset played as we have a new track.
        resetPlayed();

        // Use GStreamer discoverer to determine proper duration.
        GError* gstError = nullptr;
        GstDiscovererInfo* gstInfo = nullptr;

        gstInfo = gst_discoverer_discover_uri(musicDiscoverer, current.toEncoded().constData(), &gstError);
        if (gstInfo) {
            currentTrackDuration(static_cast<qint64>(gst_discoverer_info_get_duration(gstInfo) / GST_MSECOND));
            gst_discoverer_info_unref(gstInfo);
        } else if (gstError) {
            qCritical() << "Discoverer error: " << gstError->message;
            g_error_free(gstError);
        }

        // Update current index.
        musicCurrentIndex = index;
    }
}

void xMusicPlayer::aboutToFinish() {
    if (musicLibrary->isLocal()) {
        auto position = musicCurrentIndex;
        if (useShuffleMode) {
            position = musicPlaylistPermutation.indexOf(position);
        }
        if (position < musicPlaylist.size() - 1) {
            if (useShuffleMode) {
                musicNextIndex = musicPlaylistPermutation[position + 1];
            } else {
                musicNextIndex = position + 1;
            }
            auto currentEntry = std::get<2>(musicPlaylistEntries[musicCurrentIndex]);
            auto nextEntry = std::get<2>(musicPlaylistEntries[musicNextIndex]);
            if ((nextEntry->getSampleRate() == currentEntry->getSampleRate()) &&
                (nextEntry->getBitsPerSample() == currentEntry->getBitsPerSample())) {
                // Use GStreamer's about-to-finish to set the next URI for gapless playback.
                // - Gapless playback only works if there is no change in the sample rate or bits per sample.
                //   It will otherwise result in audio distortion.
                g_object_set(musicPlayer, "uri", musicPlaylist[musicNextIndex].toEncoded().constData(), NULL);
            }
            // The actual update of current track source will happen when the stream starts.
        }
    }
}

void xMusicPlayer::playerStatus(const QString& path, qsizetype index, qint64 position, const QString& quality) {
    // Translate updates from the BluOS player. Index will be incorrect for shuffle mode.
    if (musicPlaylistRemote.value(index) != path) {
        index = musicPlaylistRemote.indexOf(path);
    }
    if ((index >= 0) && (index < static_cast<qsizetype>(musicPlaylistEntries.size()))) {
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

void xMusicPlayer::gst_AboutToFinishCallback(GstElement*, gpointer user_data) {
    auto player = static_cast<xMusicPlayer*>(user_data);
    player->aboutToFinish();
}

gboolean xMusicPlayer::gst_BusWatchCallback(GstBus*, GstMessage* msg, gpointer user_data) {
    auto player = static_cast<xMusicPlayer*>(user_data);
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            // Handle EOS by moving to the next track if not already handled by about-to-finish.
            // - EOS used in cases where the sample rate and/or bits per sample has changed.
            player->musicNextIndex = player->musicCurrentIndex;
            player->next();
            break;
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(msg, &err, &debug);
            qCritical() << "GStreamer error: " << err->message;
            g_error_free(err);
            g_free(debug);
            player->stop();
            break;
        }
        case GST_MESSAGE_STREAM_START: {
            // Update current track index only when the stream starts.
            player->musicCurrentIndex = player->musicNextIndex;
            player->currentTrackSource(player->musicPlaylist[player->musicCurrentIndex]);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(player->musicPlayer)) {
                GstState old_state, new_state, pending_state;
                gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                if (new_state == GST_STATE_PLAYING) {
                    emit player->currentState(player->musicPlayerState = State::PlayingState);
                } else if (new_state == GST_STATE_PAUSED && pending_state == GST_STATE_VOID_PENDING) {
                    emit player->currentState(player->musicPlayerState = State::PauseState);
                }
            }
            break;
        }
        default:
            break;
    }
    return TRUE;
}

QVector<qsizetype> xMusicPlayer::computePermutation(qsizetype elements, qsizetype startIndex) {
    QList<qsizetype> input;
    QVector<qsizetype> permutation;
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

QVector<qsizetype> xMusicPlayer::extendPermutation(const QVector<qsizetype>& permutation, qsizetype elements, qsizetype extendIndex) {
    // Return an empty permutation if we do not extend.
    if (elements < permutation.count()) {
        return QVector<qsizetype>{};
    }
    QList<qsizetype> input;
    QVector<qsizetype> ePermutation;
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
