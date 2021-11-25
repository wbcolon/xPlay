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

#include <QWidget>

class xMusicFile;
class xMusicLibrary;

class xMusicPlayer:public QObject {
    Q_OBJECT

public:
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
    [[nodiscard]] virtual int getVolume() const = 0;
    /**
     * Return if support for visualization exists.
     *
     * @return true if the music player supports visualization, false otherwise.
     */
    [[nodiscard]] virtual bool supportsVisualization() const = 0;
    /**
     * Return the visualization state for the music player
     *
     * @return true if visualization support is enabled, false otherwise.
     */
    [[nodiscard]] virtual bool getVisualization() const = 0;
    /**
     * Return the mute state for the music player
     *
     * @return true if music player is muted, false otherwise.
     */
    [[nodiscard]] virtual bool isMuted() const = 0;
    /**
     * Return the playing state for the music player.
     *
     * @return true if music player is currently playing a track, false otherwise.
     */
    [[nodiscard]] virtual bool isPlaying() const = 0;
    /**
     * Return the shuffle mode for the music player
     *
     * @return true if the shuffle mode is enabled.
     */
    [[nodiscard]] virtual bool getShuffleMode() const = 0;

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
     */
    void currentTrack(int index, const QString& artist, const QString& album,
                      const QString& track, int bitrate, int sampleRate, int bitsPerSample);
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
    /*
     * A set of slots to be implemented by the derived classes.
     */
    /**
     * Play or pause depending on the current media player state.
     */
    virtual void playPause() = 0;
    /**
     * Play a given entry of the current playlist.
     *
     * @param index the position of the track in the playlist.
     */
    virtual void play(int index) = 0;
    /**
     * Move to the given position in the current track.
     *
     * @param position the position given in milliseconds.
     */
    virtual void seek(qint64 position) = 0;
    /**
     * Jump relative to the actual position in the current track.
     *
     * @param delta the delta to the current position in milliseconds.
     */
    virtual void jump(qint64 delta) = 0;
    /**
     * Stop the media player.
     */
    virtual void stop() = 0;
    /**
     * Jump to the previous track in the playlist.
     */
    virtual void prev() = 0;
    /**
     * Jump to the next track in the playlist.
     */
    virtual void next() = 0;
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    virtual void setMuted(bool mute) = 0;
    /**
     * Set the shuffle mode.
     *
     * @param shuffle enabled shuffle mode if true disable otherwise.
     */
    virtual void setShuffleMode(bool shuffle) = 0;
    /**
     * Set the volume
     *
     * @param vol integer value between 0 (silence) and 100 (full volume)
     */
    virtual void setVolume(int vol) = 0;
    /**
     * Set the visualization support.
     *
     * @param enable add visualization support if true, disable otherwise.
     */
    virtual void setVisualization(bool enable) = 0;
    /**
     * Append the given tracks to the current playlist.
     *
     * @param artist the artist name for all queued tracks.
     * @param album the album name for all queued tracks.
     * @param tracks vector of track names.
     */
    virtual void queueTracks(const QString& artist, const QString& album, const std::vector<xMusicFile*>& tracks) = 0;
    /**
     * Indicate end of queueing tracks and hand over to the actual player.
     */
    virtual void finishedQueueTracks(bool autoPlay) = 0;
    /**
     * Move the queue track after drag-drop action.
     *
     * @param fromIndex start index of the queued track that is moved.
     * @param toIndex destination index the queued track is inserted before.
     */
    virtual void moveQueueTracks(int fromIndex, int toIndex) = 0;
    /**
     * Remove the track from the queue.
     *
     * @param index the position of the track in the queue.
     */
    virtual void dequeTrack(int index) = 0;
    /**
     * Clear the playlist and stop the player.
     */
    virtual void clearQueue() = 0;
    /**
     * Load the playlist from the database and use it as queue.
     *
     * @param name the name for the playlist.
     */
    virtual void loadQueueFromPlaylist(const QString& name) = 0;
    /**
     * Save the current playlist to the database.
     *
     * @param name the name for the playlist.
     */
    virtual void saveQueueToPlaylist(const QString& name) = 0;
    /**
     * Load the playlist with tagged songs from the database.
     *
     * @param tag the tag name as string.
     * @param extend extend the playlist if true, replace it otherwise.
     */
    virtual void loadQueueFromTag(const QString& tag, bool extend) = 0;

protected:
    xMusicLibrary* musicLibrary;
};

#endif

