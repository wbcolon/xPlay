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

    xMusicPlayer(QObject* parent = nullptr);
    ~xMusicPlayer() = default;

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
    virtual int getVolume() = 0;

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
                      const QString& track, int bitrate, int sampleRate);
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
     * Signal the current state of the music player
     *
     * @param state the current state
     */
    void currentState(xMusicPlayer::State state);

public slots:
    virtual void playPause() = 0;
    virtual void play(int index) = 0;
    virtual void seek(qint64 position) = 0;
    virtual void stop() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;

    virtual void setVolume(int vol) = 0;

    virtual void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) = 0;
    virtual void dequeTrack(int index) = 0;
    virtual void clearQueue() = 0;

protected:
    /**
     * Generate a path from the given information.
     *
     * @param entry tuple of artist,album and track.
     * @return absolute path to the specified track.
     */
    QString pathFromQueueEntry(const std::tuple<QString, QString, QString>& entry);
    std::pair<int,int> propertiesFromFile(const QString& filename);

    QString baseDirectory;
};
#endif

