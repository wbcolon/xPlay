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

#ifndef __XPLAYERMOVIEWIDGET_H__
#define __XPLAYERMOVIEWIDGET_H__

#include "xMoviePlayer.h"
#include "xPlayerSliderWidgetX.h"
#include "xPlayerRotelWidget.h"

#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class xPlayerMovieWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMovieWidget(xMoviePlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMovieWidget() override = default;
    /**
     * Display the currently played movie.
     *
     * @param movie the filename of the movie played.
     */
    void currentMovie(const QString& movie);

signals:
    /**
     * Signal a toggle for enable/disable the full window mode.
     */
    void toggleFullWindow();

public slots:
    /**
     * Retrieve the audio channels for the current movie.
     *
     * @param audioChannels the list of strings of audio channels.
     */
    void currentAudioChannels(const QStringList& audioChannels);
    /**
     * Retrieve the subtitles for the current movie.
     *
     * @param subtitles the list of strings of subtitles.
     */
    void currentSubtitles(const QStringList& subtitles);
    /**
     * Retrieve the state of the movie player.
     *
     * @param state the current state of the movie player.
     */
    void currentState(xMoviePlayer::State state);
    /**
     * Retrieve the time played for the current movie.
     *
     * @param played the time played in ms.
     */
    void currentMoviePlayed(qint64 played);
    /**
     * Retrieve the length of the current movie.
     *
     * @param length the length of the movie in ms.
     */
    void currentMovieLength(qint64 length);

protected slots:
    /**
     * The full window button is pressed.
     */
    void fullWindowPressed();

private:
    /**
     * Only update combo box if entries have changed.
     *
     * @param comboBox an existing combo box widget with entries.
     * @param entries list of new entries to update combo box.
     */
    void updateComboBoxEntries(QComboBox* comboBox, const QStringList& entries);
    /**
     * Helper function creating a QGroupBox with an QListWidget.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @return pair of pointer to the created QGroupBox and QListWidget.
     */
    auto addGroupBox(const QString& boxLabel);

    QComboBox* subtitleBox;
    QComboBox* audioChannelBox;
    QLabel* movieLabel;
    xPlayerSliderWidgetX* sliderWidget;
    xMoviePlayer* moviePlayer;
    xMoviePlayer::State moviePlayerState;
    xPlayerRotelWidget* controlTabRotel;
    QPushButton* playPauseButton;
};

#endif
