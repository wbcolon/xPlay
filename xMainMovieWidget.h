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

signals:
    /*
     * Signals used for communication with player widget and the movie player.
     * The corresponding slots are implemented in the movie player widget or
     * movie player classes.
     */
    /**
     * Signal emitted to notify main widget to show/hide the menu bar.
     *
     * @param menu show menu bar if true, hide otherwise.
     */
    void showMenuBar(bool menu);
    /**
     * Signal emitted to scan the movie library for a given tag.
     *
     * @param tag the specified tag as string.
     */
    void scanForTag(const QString& tag);
    /**
     * Signal emitted to scan the movie library for a tag and directory.
     *
     * @param tag the specified tag as string.
     * @param directory the directory for the given tag to search for.
     */
    void scanForTagAndDirectory(const QString& tag, const QString& directory);
    /**
     * Signal emitted in order to play a movie.
     *
     * @param moviePath the absolute path of the movie as string.
     */
    void playMovie(const QString& moviePath);

public slots:
    /**
     * Retrieve a list of tags after scanning of the movie library.
     *
     * @param tags the list of strings of tags.
     */
    void scannedTags(const QStringList& tags);
    /**
     * Retrieve a list of directories after scan for a tag.
     *
     * @param directories the list of strings of directories.
     */
    void scannedDirectories(const QStringList& directories);
    /**
     * Retrieve a list of movies after scan for tag and directory.
     *
     * @param movies the vector of pairs of file name and full paths.
     */
    void scannedMovies(const std::vector<std::pair<QString, QString>>& movies);

private slots:
    /**
     * Get the state of the movie player upon state change.
     *
     * @param state current state of the movie player.
     */
    void currentState(xMoviePlayer::State state);
    /**
     * Enable/disable the full window mode.
     *
     * In contrast to the full screen mode the full window mode only
     * maximizes the video output to the limits of the window. The
     * menu entries are still visible.
     *
     * @param mode enable full window mode if true, disable otherwise.
     */
    void setFullWindow(bool mode);
    /**
     * Toggle between enable/disable of the full window mode.
     */
    void toggleFullWindow();
    /**
     * Select an entry in the tag list widget.
     *
     * @param index the index of the selected tag.
     */
    void selectTag(int index);
    /**
     * Select an entry in the directory list widget.
     *
     * @param index the index of the selected directory.
     */
    void selectDirectory(int index);
    /**
     * Select an entry in the movie list widget by double-click.
     *
     * @param item the pointer to the selected item.
     */
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
