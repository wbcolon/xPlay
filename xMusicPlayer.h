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

class xMusicPlayer:public QObject {
    Q_OBJECT

public:
    enum State {
        PlayingState,
        PauseState,
        StopState
    };

    explicit xMusicPlayer(QObject* parent = nullptr);
    ~xMusicPlayer() override = default;

    /**
     * Set the base directory for the music library
     *
     * @param base the absolute path of the base of the music library.
     */
    void setBaseDirectory(const QString& base);
    /**
     * Return the volume for the music player
     *
     * @return integer value in between 0 and 100.
     */
    [[nodiscard]] virtual int getVolume() const = 0;
    /**
     * Return the mute state for the music player
     *
     * @return true if music player is muted, false otherwise.
     */
    [[nodiscard]] virtual bool isMuted() const = 0;
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
     * Signal the current state of the music player
     *
     * @param state the current state
     */
    void currentState(xMusicPlayer::State state);

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
     * Append the given tracks to the current playlist.
     *
     * @param artist the artist name for all queued tracks.
     * @param album the album name for all queued tracks.
     * @param tracks vector of track names.
     */
    virtual void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) = 0;
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

protected:
    /**
     * Generate a path from the given information.
     *
     * @param entry tuple of artist,album and track.
     * @return absolute path to the specified track.
     */
    QString pathFromQueueEntry(const std::tuple<QString, QString, QString>& entry);
    /**
     * Extract music file properties (bitrate, sample rate, bits per sample).
     *
     * @param filename the absolute path to the music file.
     * @return a tuple containing the file properties.
     */
    static std::tuple<int,int,int> propertiesFromFile(const QString& filename);
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

    QString baseDirectory;
};

#endif

