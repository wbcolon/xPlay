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

#include "xMoviePlayerX.h"
#include "xPlayerSliderWidgetX.h"
#include "xPlayerControlButtonWidget.h"
#include "xPlayerRotelWidget.h"

#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class xPlayerMovieWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMovieWidget(xMoviePlayerX* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMovieWidget() override = default;

signals:
    /**
     * Signal a toggle for enable/disable the full window mode.
     */
    void toggleFullWindow();
    /**
     * Signal a change in the autoplay next setting.
     *
     * @param mode the current state of autoplay.
     */
    void autoPlayNextMovie(bool mode);

public slots:
    /**
     * Display the currently played movie.
     *
     * @param path the path of the movie played.
     * @param movie the filename of the movie played.
     * @param tag the tag for the movie played.
     * @param directory the directory for the movie played.
     */
    void currentMovie(const QString& path, const QString& movie, const QString& tag, const QString& directory);
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
    void currentState(xMoviePlayerX::State state);
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
    /**
     * Reset the the movie/audio/subtitle/scale and crop widgets.
     */
    void clear();


private slots:
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
    static void updateComboBoxEntries(QComboBox* comboBox, const QStringList& entries);

    QComboBox* subtitleBox;
    QComboBox* audioChannelBox;
    QLabel* movieLabel;
    xPlayerSliderWidgetX* sliderWidget;
    xMoviePlayerX* moviePlayer;
    xMoviePlayerX::State moviePlayerState;
    xPlayerControlButtonWidget* controlButtonWidget;
    xPlayerRotelWidget* controlTabRotel;
};

#endif
