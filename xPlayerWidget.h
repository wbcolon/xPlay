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
#ifndef __XPLAYERWIDGET_H__
#define __XPLAYERWIDGET_H__

#include "xMusicPlayer.h"
#include "xPlayerSliderWidget.h"
#include "xPlayConfig.h"

#include <QLabel>
#include <QPushButton>

class xPlayerWidget:public QWidget {
    Q_OBJECT

public:
    xPlayerWidget(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerWidget() = default;

signals:
    /**
     * Signal the position of the current track.
     *
     * @param index the position of the track in the playlist.
     */
    void currentQueueTrack(int index);
    /**
     * Signal that the playlist needs to be cleared.
     */
    void clearQueue();

protected slots:
    /**
     * Reset the the artist/album/track/played/length labels.
     */
    void clear();

    /**
     * Update the player widget with the provided information.
     *
     * @param index the position of the current track in the playlist.
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track name of the current track.
     * @param bitrate the bitrate in kb/sec.
     * @param sampleRate the sample rate in Hz.
     */
    void currentTrack(int index, const QString& artist, const QString& album,
                      const QString& track, int bitrate, int sampleRate);
    /**
     * Update the player UI based on the player state.
     *
     * @param state the current state of the player.
     */
    void currentState(xMusicPlayer::State state);

private:
    /**
     * Labels to display information about the current track.
     */
    QLabel* artistName;
    QLabel* albumName;
    QLabel* trackName;
    QLabel* trackSampleRate;
    QLabel* trackBitrate;
    /**
     * Play/pause button alternates between "Play" and "Pause" depending on the music players state.
     */
    QPushButton* playPauseButton;
    xPlayerSliderWidget* sliderWidget;
};

#endif
