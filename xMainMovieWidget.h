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
#include "xPlayerSliderWidget.h"
#include "xPlayerVolumeWidget.h"
#include "xPlayerListWidget.h"

#include <QStackedWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QWidget>
#include <vector>

class xMovieLibraryEntry;

class xMainMovieWidget:public QStackedWidget {
    Q_OBJECT

public:
    explicit xMainMovieWidget(xMoviePlayer* player, QWidget* parent=nullptr);
    ~xMainMovieWidget() override = default;
    /**
     * Perform initial commands required when switching to this view.
     */
    void initializeView();
    /**
     * Clear tags, directories, movies and player.
     */
    void clear();

signals:
    /*
     * Signals used for communication with player widget and the movie player.
     * The corresponding slots are implemented in the movie player widget or
     * movie player classes.
     */
    /**
     * Signal emitted to notify main widget to update the window title.
     *
     * @param title the new window title as string.
     */
    void showWindowTitle(const QString& title);
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
     * @param path the absolute path of the movie as string.
     * @param name the name displayed for the movie.
     * @param tag the tag for the movie played.
     * @param directory the directory for the movie played.
     */
    void setMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory);
    /**
     * Signal emitted in order to set the movie queue.
     *
     * @param queue list of pairs of path and name to be displayed.
     */
    void setMovieQueue(const QList<std::pair<std::filesystem::path, QString>>& queue);
    /**
     * Signal emitted in order to clear the movie queue;
     */
    void clearMovieQueue();

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
    void scannedMovies(const std::vector<xMovieLibraryEntry*>& movies);

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
     * Select an entry in the tag list widget.
     *
     * @param index the listIndex of the selected tag.
     */
    void selectTag(int index);
    /**
     * Select an entry in the directory list widget.
     *
     * @param index the listIndex of the selected directory.
     */
    void selectDirectory(int index);
    /**
     * Select an entry in the movie list widget by double-click.
     *
     * @param item the pointer to the selected listItem.
     */
    void selectMovie(xPlayerListWidgetItem* item);
    /**
     * Select an entry in the movie list widget based on its path.
     *
     * @param path the path of the currently played movie.
     * @param name the name displayed for the movie.
     * @param tag the tag for the movie played.
     * @param directory the directory for the movie played.
     */
    void updateSelectedMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory);
    /**
     * Set the autoplay next mode and update/clear the movie queue.
     *
     * @param mode clear the queue if true, else update queue.
     */
    void setAutoPlayNextMovie(bool mode);
    /**
     * Update the movie database overlay on configuration changes.
     */
    void updatedDatabaseMovieOverlay();
    /**
     * Update the database overlay for currently played movie and add tooltips.
     *
     * @param tag the tag for the currently played movie.
     * @param directory the directory for the currently played movie.
     * @param movie the name of the currently played movie.
     * @param playCount the play count for the currently played movie.
     * @param timeStamp the last played time stamp in milli seconds for the currently played movie.
     */
    void updatePlayedMovie(const QString& tag, const QString& directory, const QString& movie, int playCount, qint64 timeStamp);
    /**
     * Update the playback time in the window title.
     *
     * @param timeStamp the playback time in ms.
     */
    void updateWindowTitlePlayBack(qint64 timeStamp);

private:
    /**
     * Create movie queue and emit the queue to the player.
     *
     * @param index the listIndex of the currently selected (played) movie.
     */
    void updateMovieQueue(int index);
    /**
     * Update the database overlay for played tags.
     */
    void updatePlayedTags();
    /**
     * Update the database overlay for played directories.
     */
    void updatePlayedDirectories();
    /**
     * Update the database overlay for played movies and add tooltips.
     */
    void updatePlayedMovies();
    /**
     * Emit signal to initiate update of window title with new currently playing movie.
     *
     * @param path the path of the currently played movie.
     * @param name the displayed name of the currently playing movie.
     * @param tag the tag for the currently movie played.
     * @param directory the directory for the currently movie played.
     */
    void updateWindowTitle(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory);
    /**
     * Create the window title based on the current movie played and the full window mode.
     *
     * @return the new window title as string.
     */
    QString createWindowTitle();
    /**
     * Helper function creating a QGroupBox with an QListWidget.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @param displayTime configure whether or not to display time column.
     * @return pair of pointer to the created QGroupBox and QListWidget.
     */
    static auto addGroupBox(const QString& boxLabel, bool displayTime, QWidget* parent);

    xPlayerListWidget* tagList;
    xPlayerListWidget* directoryList;
    xPlayerListWidget* movieList;
    QGroupBox* movieFilterBox;
    QLineEdit* movieFilter;
    QList<xMovieLibraryEntry*> currentMovies;
    QString currentMovieName;
    QString currentMovieTag;
    QString currentMovieDirectory;
    //xMoviePlayer* moviePlayer;
    xMoviePlayer* moviePlayer;
    QStackedWidget* movieStack;
    xPlayerMovieWidget* moviePlayerWidget;
    QWidget* mainWidget;
    bool autoPlayNextMovie;
    bool useDatabaseMovieOverlay;
    qint64 databaseCutOff;
};

#endif
