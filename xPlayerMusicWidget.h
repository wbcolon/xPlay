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
#include "xPlayerSliderWidget.h"
#include "xPlayerControlButtonWidget.h"

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

class xPlayerMusicWidget: public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMusicWidget(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMusicWidget() override = default;

signals:
    /**
     * Signal that the playlist needs to be cleared.
     */
    void clearQueue();
    /**
     * Emitted whenever a double-click with the left button occurs.
     */
    void mouseDoubleClicked();

public slots:
    /**
     * Reset the the artist/album/track/played/length labels.
     */
    void clear();

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    /**
     * Update the info mode label.
     */
    void updateInfoMode();
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
     * @param quality the quality as string.
     */
    void currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                      int bitrate, int sampleRate, int bitsPerSample, const QString& quality);
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
    QStackedWidget* infoStacked;
    QWidget* infoPlayer;
    QLabel* artistName;
    QLabel* albumName;
    QLabel* trackName;
    QLabel* trackSampleRateLabel;
    QLabel* trackSampleRate;
    QLabel* trackBitrateLabel;
    QLabel* trackBitrate;
    QLabel* trackBitsPerSample;
    QPixmap trackBluOS;
    QLabel* infoMode;
    /**
     * Play/pause button alternates between "Play" and "Pause" depending on the music players state.
     */
    xPlayerControlButtonWidget* controlButtonWidget;
    xPlayerSliderWidget* sliderWidget;
    xPlayerRotelWidget* controlTabRotel;
    xPlayerVolumeWidget* volumeWidget;
    xMusicPlayer* musicPlayer;
};

#endif
