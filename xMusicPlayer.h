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
#ifndef __XMUSICPLAYER_H__
#define __XMUSICPLAYER_H__

#include "xMusicLibrary.h"
#include "xPlayerPulseAudioControls.h"

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <QMediaPlayer>

class xMusicPlayer: public QObject {
    Q_OBJECT

public:
    // Music player states.
    enum State {
        PlayingState,
        PauseState,
        StopState
    };

    explicit xMusicPlayer(xMusicLibrary* library, QObject* parent = nullptr);
    ~xMusicPlayer();
    /**
     * Return the volume for the music player
     *
     * @return integer value in between 0 and 100.
     */
    [[nodiscard]] qsizetype getVolume() const;
    /**
     * Return the mute state for the music player
     *
     * @return true if the music player is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted() const;
    /**
     * Return the playing state for the music player.
     *
     * @return true if the music player is currently playing a track, false otherwise.
     */
    [[nodiscard]] bool isPlaying() const;
    /**
     * Return the shuffle mode for the music player
     *
     * @return true if the shuffle mode is enabled.
     */
    [[nodiscard]] bool getShuffleMode() const;
    /**
     * Return the state of the queue.
     *
     * @return true if the queue is empty, false otherwise.
     */
    [[nodiscard]] bool isQueueEmpty() const;
    /**
     * Return if it is allowed to queue tracks.
     *
     * @return true if it is allowed queueing tracks, false if no tracks should be queued.
     */
    [[nodiscard]] bool isQueueTracksAllowed() const;

signals:
    /**
     * Signals information of the currently played track to the receiver.
     *
     * @param index the position of the current track in the playlist
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track the name of the current track.
     * @param bitrate the bitrate in kb/sec.
     * @param sampleRate the sample rate in Hz.
     * @param bitsPerSample the bits per sample.
     * @param quality the quality as string.
     */
    void currentTrack(qsizetype index, const QString& artist, const QString& album, const QString& track,
                      qsizetype bitrate, qsizetype sampleRate, qsizetype bitsPerSample, const QString& quality);
    /**
     * Update the database overlay for currently played track and add tooltips.
     *
     * @param artist the artist for the currently played track.
     * @param album the album for the currently played track.
     * @param track the track number and name for the currently played track.
     * @param playCount the play count for the currently played track.
     * @param timeStamp the last played time stamp in milliseconds for the currently played track.
     */
    void updatePlayedTrack(const QString& artist, const QString& album, const QString& track, qsizetype playCount, qint64 timeStamp);
    /**
     * Signal the amount played for the current track.
     *
     * @param played the amount played in milliseconds.
     */
    void currentTrackPlayed(qint64 played);
    /**
     * Signal the length of the current track
     *
     * @param length the length in milliseconds.
     */
    void currentTrackLength(qint64 length);
    /**
     * Signal the current volume of the music player.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    void currentVolume(qsizetype vol);
    /**
     * Signal the current state of the music player.
     *
     * @param state the current state
     */
    void currentState(xMusicPlayer::State state);
    /**
     * Signal if the music player allows enabling the shuffle mode in its current state.
     *
     * @param enable shuffle mode can be enabled if true, not otherwise.
     */
    void allowShuffleMode(bool enable);
    /**
     * Signal the currently loaded playlist to update queue UI.
     *
     * @param entries playlist entries as tuple of artist, album and track.
     */
    void playlist(const std::vector<std::tuple<QString,QString,QString>>& entries);
    /**
     * Signal the state of the saved playlist.
     *
     * @param name the name for the playlist.
     * @param saved true is playlist was saved, false otherwise.
     */
    void playlistState(const QString& name, bool saved);

public slots:
    /**
     * Append the given tracks to the current playlist.
     *
     * Each of the tracks will be added to the playlist as a file (@see pathFromQueueEntry).
     * In addition, the information is cached in the musicPlaylistEntries data structure
     * to allow for GUI updates of the player widget.
     *
     * @param artist the artist name for all queued tracks.
     * @param album the album name for all queued tracks.
     * @param tracks vector of music file objects.
     */
    void queueTracks(const QString& artist, const QString& album, const std::vector<xMusicLibraryTrackEntry*>& tracks);
    /**
     * Indicate the end of queueing tracks and hand over to the actual player.
     *
     * Called after the one or more queueTracks. The permutation for shuffle mode is computed,
     * and the state of autoplay is determined and acted on.
     */
    void finishedQueueTracks(bool autoPlay);
    /**
     * Move the queue track after the drag-drop action.
     *
     * @param fromIndex start the index of the queued track that is moved.
     * @param toIndex destination index the queued track is inserted before.
     */
    void moveQueueTracks(qsizetype fromIndex, qsizetype toIndex);
    /**
     * Remove the track from the queue.
     *
     * @param index the position of the track in the queue.
     */
    void dequeueTrack(qsizetype index);
    /**
     * Clear the playlist and stop the player.
     */
    void clearQueue();
    /**
     * Load the playlist from the database and add it to the queue.
     *
     * @param name the name for the playlist.
     */
    void loadQueueFromPlaylist(const QString& name);
    /**
     * Save the current playlist to the database
     *
     * @param name the name for the playlist.
     */
    void saveQueueToPlaylist(const QString& name);
    /**
     * Load the playlist with tagged songs from the database.
     *
     * @param tag the tag name as a string.
     * @param extend extend the playlist if true, replace it otherwise.
     */
    void loadQueueFromTag(const QString& tag, bool extend);
    /**
     * Play or pause depending on the current media player state.
     */
    void playPause();
    /**
     * Play a given entry of the current playlist.
     *
     * @param index the position of the track in the playlist.
     */
    void play(qsizetype index);
    /**
     * Move to the given position in the current track.
     *
     * @param position the position given in millisecond.
     */
    void seek(qint64 position);
    /**
     * Jump relative to the actual position in the current track.
     *
     * @param delta the delta to the current position in milliseconds.
     */
    void jump(qint64 delta);
    /**
     * Stop the media player.
     */
    void stop();
    /**
     * Jump to the previous track in the playlist.
     */
    void prev();
    /**
     * Jump to the next track in the playlist.
     */
    void next();
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute);
    /**
     * Set the shuffle mode.
     *
     * @param shuffle enabled shuffle mode if true disable otherwise.
     */
    void setShuffleMode(bool shuffle);
    /**
     * Set the volume
     *
     * @param vol integer value between 0 (silence) and 100 (full volume)
     */
    void setVolume(int vol);

    void currentTrackSource(const QUrl& current);
    void currentTrackDuration(qint64 duration);
    void updatePlayed(qint64 pos);
    void updateDatabase(qsizetype index);
    void handleBusMessage();
    void aboutToFinish();

private slots:
    /**
     * Handle the status of the BluOS player.
     *
     * @param path the path of the currently played track.
     * @param index the index in the playlist.
     * @param position the currently play portion of the song in ms.
     * @param quality the quality of track as string.
     */
    void playerStatus(const QString& path, qsizetype index, qint64 position, const QString& quality);
    /**
     * Possibly reinitialize the music player when we switch music libraries.
     */
    void reInitialize();

private:
    /**
     * Reset played indicator variables.
     */
    void resetPlayed();
    /**
     * Compute a permutation for 0...elements-1. Allow for a fixed starting index.
     *
     * @param elements the number of elements for the permutation.
     * @param startIndex the fixed starting index if >= 0.
     * @return a vector containing a random permutation of 0...elements-1.
     */
    static QVector<qsizetype> computePermutation(qsizetype elements, qsizetype startIndex);
    /**
     * Extend a given permutation by keeping the initial permutation to extendIndex.
     *
     * @param permutation The current permutation used. Contains extendIndex.
     * @param elements the new number of elements for the permutation.
     * @param extendIndex the index up to the permutation is used in the extended permutation.
     * @return a vector containing an extended random permutation of 0...elements-1.
     */
    static QVector<qsizetype> extendPermutation(const QVector<qsizetype>& permutation, qsizetype elements, qsizetype extendIndex);

    xPlayerPulseAudioControls* pulseAudioControls;
    xMusicLibrary* musicLibrary;
    std::vector<std::tuple<QString,QString,xMusicLibraryTrackEntry*>> musicPlaylistEntries;
    QList<QUrl> musicPlaylist;
    QStringList musicPlaylistRemote;
    QVector<qsizetype> musicPlaylistPermutation;
    GstElement* musicPlayer;
    xMusicPlayer::State musicPlayerState;
    bool useShuffleMode;
    // Keep track of the time played.
    qint64 musicCurrentPosition;
    qint64 musicCurrentPlayed;
    qint64 musicCurrentDuration;
    qsizetype musicCurrentIndex;
    // Keep track only of played artist and album for transition.
    QString musicPlayedArtist;
    QString musicPlayedAlbum;
    qsizetype musicPlayedIndex;
    qint64 musicPlayed;
    bool musicPlayedRecorded;
    // We need to track the current remote track played to avoid duplicate database entries.
    QString musicCurrentRemote;
    bool musicRemoteAutoNext;
    // We need to track if the current track played to work around some phonon issues.
    bool musicCurrentFinished;
    // Required to determine track length and other properties.
    GstDiscoverer* musicDiscoverer;
    // Timer for position updates.
    QTimer* musicTimer;
};

#endif
