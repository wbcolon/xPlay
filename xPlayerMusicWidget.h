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
#ifndef __XPLAYERMUSICWIDGET_H__
#define __XPLAYERMUSICWIDGET_H__

#include "xMusicPlayer.h"
#include "xPlayerRotelWidget.h"
#include "xPlayerSliderWidgetX.h"

#include <QLabel>
#include <QPushButton>

class xPlayerMusicWidget: public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMusicWidget(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMusicWidget() override = default;

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

public slots:
    /**
     * Reset the the artist/album/track/played/length labels.
     */
    void clear();

protected slots:
    /**
     * Update the player widget with the provided information.
     *
     * @param index the position of the current track in the playlist.
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track name of the current track.
     * @param bitrate the bitrate in kb/sec.
     * @param sampleRate the sample rate in Hz.
     * @param bitsPerSample the number of bits per sample (mostly 16 of 24).
     */
    void currentTrack(int index, const QString& artist, const QString& album,
                      const QString& track, int bitrate, int sampleRate, int bitsPerSample);
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
    QLabel* trackBitsPerSample;
    /**
     * Play/pause button alternates between "Play" and "Pause" depending on the music players state.
     */
    QPushButton* playPauseButton;
    xPlayerSliderWidgetX* sliderWidget;
    xPlayerRotelWidget* controlTabRotel;
};

#endif
