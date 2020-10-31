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
#ifndef __XMAINMOVIEWIDGET_H__
#define __XMAINMOVIEWIDGET_H__

#include "xMoviePlayer.h"
#include "xPlayerMovieWidget.h"
#include "xPlayerSliderWidgetX.h"
#include "xPlayerVolumeWidgetX.h"

#include <QStackedWidget>
#include <QListWidget>
#include <QWidget>
#include <vector>

class xMainMovieWidget:public QStackedWidget {
    Q_OBJECT

public:
    xMainMovieWidget(xMoviePlayer* player, QWidget* parent=nullptr);
    ~xMainMovieWidget() = default;

    void connectToRotel(xPlayerRotelWidget* rotel);

signals:
    void scanForTag(const QString& tag);
    void scanForTagAndDirectory(const QString& tag, const QString& directory);

    void playMovie(const QString& moviePath);

public slots:
    void scannedTags(const QStringList& tags);
    void scannedDirectories(const QStringList& directories);
    void scannedMovies(const std::vector<std::pair<QString, QString>>& movies);

    void currentAudioChannels(const QStringList& audioChannels);
    void currentSubtitles(const QStringList& subtitles);
    void currentMoviePlayed(qint64 played);
    void currentMovieLength(qint64 length);

private slots:
    void currentState(xMoviePlayer::State state);
    void setFullWindow(bool mode);
    void toggleFullWindow();

    void selectTag(int index);
    void selectDirectory(int index);
    void selectMovie(QListWidgetItem* item);

private:
    /**
     * Helper function creating a QGroupBox with an QListWidget.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @return pair of pointer to the created QGroupBox and QListWidget.
     */
    auto addGroupBox(const QString& boxLabel, QWidget* parent);

    QListWidget* tagList;
    QListWidget* directoryList;
    QListWidget* movieList;
    QStringList currentMovies;
    xMoviePlayer* moviePlayer;
    QStackedWidget* movieStack;
    xPlayerMovieWidget* moviePlayerWidget;
    QWidget* mainWidget;
    bool fullWindow;
};

#endif
