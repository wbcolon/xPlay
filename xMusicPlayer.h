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

#include <phonon/MediaObject>
#include <phonon/MediaSource>
#include <phonon/AudioOutput>
#include <phonon/AudioDataOutput>

#include <QMediaPlayer>

class xMusicPlayer: public QObject {
    Q_OBJECT

public:
    // Default delta for forward and rewind.
    static constexpr qint64 ForwardRewindDelta = 5000;
    // Music player states.
    enum State {
        PlayingState,
        PauseState,
        StopState
    };

    explicit xMusicPlayer(xMusicLibrary* library, QObject* parent = nullptr);
    ~xMusicPlayer() override = default;
    /**
     * Return the volume for the music player
     *
     * @return integer value in between 0 and 100.
     */
    [[nodiscard]] int getVolume() const;
    /**
     * Return if support for visualization exists.
     *
     * @return true if the music player supports visualization, false otherwise.
     */
    [[nodiscard]] bool supportsVisualization();
    /**
     * Return the visualization state for the music player
     *
     * @return true if visualization support is enabled, false otherwise.
     */
    [[nodiscard]] bool getVisualization() const;
    /**
     * Return the mute state for the music player
     *
     * @return true if music player is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted() const;
    /**
     * Return the playing state for the music player.
     *
     * @return true if music player is currently playing a track, false otherwise.
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
     * Return if queue tracks is allowed.
     *
     * @return true if queue track is allowed, false if no tracks should be queued.
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
    void currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                      int bitrate, int sampleRate, int bitsPerSample, const QString& quality);
    /**
     * Update the database overlay for currently played track and add tooltips.
     *
     * @param artist the artist for the currently played track.
     * @param album the album for the currently played track.
     * @param track the track number and name for the currently played track.
     * @param playCount the play count for the currently played track.
     * @param timeStamp the last played time stamp in milli seconds for the currently played track.
     */
    void updatePlayedTrack(const QString& artist, const QString& album, const QString& track, int playCount, qint64 timeStamp);
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
    void currentVolume(int vol);
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
    /**
     * Signal the visualization data for the left and right channel.
     *
     * The signal always transmits 512 samples for left and right.
     *
     * @param left the data for the left channel.
     * @param right the data for the right channel.
     */
    void visualizationStereo(const QVector<qint16>& left, const QVector<qint16>& right);

public slots:
    /**
     * Append the given tracks to the current playlist.
     *
     * Each of the tracks will be added to the playlist as file (@see pathFromQueueEntry).
     * In addition the information is cached in the musicPlaylistEntries data structure
     * to allow for GUI updates of the player widget.
     *
     * @param artist the artist name for all queued tracks.
     * @param album the album name for all queued tracks.
     * @param tracks vector of music file objects.
     */
    void queueTracks(const QString& artist, const QString& album, const std::vector<xMusicLibraryTrackEntry*>& tracks);
    /**
     * Indicate end of queueing tracks and hand over to the actual player.
     *
     * Called after the one or more queueTracks. The permutation for shuffle mode is computed
     * and the state of autoplay is determined and acted on.
     */
    void finishedQueueTracks(bool autoPlay);
    /**
     * Move the queue track after drag-drop action.
     *
     * @param fromIndex start index of the queued track that is moved.
     * @param toIndex destination index the queued track is inserted before.
     */
    void moveQueueTracks(int fromIndex, int toIndex);
    /**
     * Remove the track from the queue.
     *
     * @param index the position of the track in the queue.
     */
    void dequeueTrack(int index);
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
     * @param tag the tag name as string.
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
    void play(int index);
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
    /**
     * Set the visualization support.
     *
     * @param enabled add visualization support if true, disable otherwise.
     */
    void setVisualization(bool enabled);

private slots:
    /**
     * Current playlist track has changed.
     *
     * Called whenever a new entry out of the playlist is played. The call is
     * triggered by the playlist. The currentTrack signal is triggered.
     *
     * @param current the media object of the current track in the playlist.
     */
    void currentTrackSource(const Phonon::MediaSource& current);
    /**
     * Determine current track length through Qt mediaplayer.
     *
     * @param duration the track length in milliseconds.
     */
    void currentTrackDuration(qint64 duration);
    /**
     * Update the actual time played.
     *
     * @param pos the current position in milliseconds.
     */
    void updatePlayed(qint64 pos);
    /**
     * Update the database.
     *
     * @param index the position of the current track in the playlist.
     */
    void updateDatabase(int index);
    /**
     * Track state changes in phonon media player. Try to recover from errors.
     *
     * @param newState the current state of the phonon media player.
     * @param oldState the previous state of the phonon media player.
     */
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    /**
     * Track finished signal from the phonon media player. Track errors.
     */
    void aboutToFinish();
    /**
     * Transform the data from the AudioDataOutput to be used by visualization.
     *
     * @param data the map of samples.
     */
    void visualizationUpdate(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16>>& data);
    /**
     * Handle the status of the BluOS player.
     *
     * @param path the path of the currently plqyed track.
     * @param index the index in the playlist.
     * @param position the currently play portion of the song in ms.
     * @param quality the quality of track as string.
     */
    void playerStatus(const QString& path, int index, qint64 position, const QString& quality);
    /**
     * Possibly reinitialize the music player when we switch music libraries.
     */
    void reInitialize();

private:
    void resetPlayed();
    /**
     * Compute a permutation for 0...elements-1. Allow for a fixed starting index.
     *
     * @param elements the number of elements for the permutation.
     * @param startIndex the fixed starting index if >= 0.
     * @return a vector containing a random permutation of 0...elements-1.
     */
    static QVector<int> computePermutation(int elements, int startIndex);
    /**
     * Extend a given permutation by keeping the initial permutation to extendIndex.
     *
     * @param permutation the current permutation used. Contains extendIndex.
     * @param elements the new number of elements for the permutation.
     * @param extendIndex the index up to the permutation is used in the extended permutation.
     * @return a vector containing an extended random permutation of 0...elements-1.
     */
    static QVector<int> extendPermutation(const QVector<int>& permutation, int elements, int extendIndex);

    xPlayerPulseAudioControls* pulseAudioControls;
    xMusicLibrary* musicLibrary;
    std::vector<std::tuple<QString,QString,xMusicLibraryTrackEntry*>> musicPlaylistEntries;
    QList<Phonon::MediaSource> musicPlaylist;
    QStringList musicPlaylistRemote;
    QVector<int> musicPlaylistPermutation;
    Phonon::MediaObject* musicPlayer;
    Phonon::AudioOutput* musicOutput;
    Phonon::AudioDataOutput* musicVisualization;
    bool musicVisualizationSupported;
    bool musicVisualizationEnabled;
    int musicVisualizationSampleRate;
    xMusicPlayer::State musicPlayerState;
    bool useShuffleMode;
    // Keep track of the time played.
    qint64 musicCurrentPosition;
    qint64 musicCurrentPlayed;
    qint64 musicCurrentDuration;
    int musicCurrentIndex;
    // Keep track only of played artist and album for transition.
    QString musicPlayedArtist;
    QString musicPlayedAlbum;
    int musicPlayedIndex;
    qint64 musicPlayed;
    bool musicPlayedRecorded;
    // We need to track the current remote track played to avoid duplicate database entries.
    QString musicCurrentRemote;
    bool musicRemoteAutoNext;
    // We need to track if the current track played to work around some phonon issues.
    bool musicCurrentFinished;
    // Only required due to track length issues with phonon.
    QMediaPlayer* musicPlayerForTime;
};

#endif
