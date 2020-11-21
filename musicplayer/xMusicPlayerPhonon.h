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
#ifndef __XMUSICPLAYERPHONON_H__
#define __XMUSICPLAYERPHONON_H__

#include "xMusicPlayer.h"

#include <phonon/MediaObject>
#include <phonon/MediaSource>
#include <phonon/AudioOutput>
#include <QMediaPlayer>

class xMusicPlayerPhonon:public xMusicPlayer {
    Q_OBJECT

public:
    explicit xMusicPlayerPhonon(QObject* parent = nullptr);
    ~xMusicPlayerPhonon() override = default;
    /**
     * Return the volume for the music player
     *
     * @return integer value in between 0 and 100.
     */
    [[nodiscard]] int getVolume() const override;
    /**
     * Return the mute state for the music player
     *
     * @return true if music player is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted() const override;

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
     * @param tracks vector of track names.
     */
    void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) override;
    /**
     * Remove the track from the queue.
     *
     * @param index the position of the track in the queue.
     */
    void dequeTrack(int index) override;
    /**
     * Clear the playlist and stop the player.
     */
    void clearQueue() override;
    /**
     * Play or pause depending on the current media player state.
     */
    void playPause() override;
    /**
     * Play a given entry of the current playlist.
     *
     * @param index the position of the track in the playlist.
     */
    void play(int index) override;
    /**
     * Move to the given position in the current track.
     *
     * @param position the position given in millisecond.
     */
    void seek(qint64 position) override;
    /**
     * Jump relative to the actual position in the current track.
     *
     * @param delta the delta to the current position in milliseconds.
     */
    void jump(qint64 delta) override;
    /**
     * Stop the media player.
     */
    void stop() override;
    /**
     * Jump to the previous track in the playlist.
     */
    void prev() override;
    /**
     * Jump to the next track in the playlist.
     */
    void next() override;
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute) override;
    /**
     * Set the volume
     *
     * @param vol integer value between 0 (silence) and 100 (full volume)
     */
    void setVolume(int vol) override;

private slots:
    /**
     * Current playlist track has changed.
     *
     * Called whenever a new entry out of the playlist is played. The call is
     * triggered by the playlist. The currentTrack signal is triggered.
     *
     * @param index the position of the current track in the playlist.
     */
    void currentTrackSource(const Phonon::MediaSource& current);
    void currentTrackDuration(qint64 duration);

    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void finished();

private:
    std::vector<std::tuple<QString,QString,QString>> musicPlaylistEntries;
    QList<Phonon::MediaSource> musicPlaylist;
    Phonon::MediaObject* musicPlayer;
    Phonon::AudioOutput* musicOutput;
    // Only required due to track length issues with phonon.
    QMediaPlayer* musicPlayerForTime;
};

#endif
