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
#ifndef __XAPPLICATION_H__
#define __XAPPLICATION_H__

#include "xMusicLibrary.h"
#include "xMusicPlayerX.h"
#include "xMainMusicWidget.h"
#include "xMovieLibrary.h"
#include "xMoviePlayer.h"
#include "xMainMovieWidget.h"
#include "xMainStreamingWidget.h"
#include "xPlayerDBus.h"

#include <QSettings>
#include <QStackedWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

class xApplication:public QMainWindow {
public:
    explicit xApplication(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xApplication() override = default;

private slots:
    /**
     * Display the xPlay configuration dialog.
     */
    void configure();
    /**
     * Display message dialog on database update error.
     */
    void databaseUpdateError();
    /**
     * Set the music library directory if it has been updated.
     */
    void setMusicLibraryDirectory();
    /**
     * Set the Rotel network address/port if its configuration has been updated.
     */
    void setRotelNetworkAddress();
    /**
     * Set the movie library tags and directories if they have been updated.
     */
    void setMovieLibraryTagsAndDirectories();
    /**
     * Set the streaming sites if they have been updated.
     */
    void setStreamingSites();
    /**
     * Set the streaming sites default if they have been updated.
     */
    void setStreamingSitesDefault();
    /**
     * Toggle play/pause in current view (music/movie) after dbus call.
     */
    void dbus_playPause();
    /**
     * Stop the playback in current view (music/movie) after dbus call.
     */
    void dbus_stop();
    /**
     * Play the previous file in queue of the current view (music) after dbus call.
     */
    void dbus_prev();
    /**
     * Play the next file in queue of the current view (music) after dbus call.
     */
    void dbus_next();
    /**
     * Jump within the current file of the current view (movie) after dbus call.
     *
     * @param delta the delta to the current position in ms.
     */
    void dbus_jump(qint64 delta);
    /**
     * Toggle the full window mode in the current view (movie) after dbus call.
     */
    void dbus_fullWindow();
    /**
     * Toggle the scale and crop mode in the current view (movie) after dbus call.
     */
    void dbus_scaleAndCrop();
    /**
     * Toggle mute the audio of the current view (music/movie) after dbus call.
     */
    void dbus_mute();
    /**
     * Change the volume of the current view (music/movie) after dbus call.
     *
     * @param delta the delta to the current volume.
     */
    void dbus_changeVolume(int delta);
    /**
     * Select the current view.
     *
     * @param view the new view as string.
     */
    void dbus_selectView(const QString& view);
    /**
     * Toggle mute for the connected Rotel amp.
     */
    void dbus_muteRotel();
    /**
     * Change the volume of the connected Rotel amp.
     *
     * @param delta the delta to the current volume.
     */
    void dbus_changeRotelVolume(int delta);
    /**
     * Select a new source for the connected Rotel amp.
     *
     * @param source the new source as string.
     */
    void dbus_selectRotelSource(const QString& source);

private:
    /**
     * Generate the File and View menus and connect actions.
     */
    void createMenus();

    xMusicLibrary* musicLibrary;
    xMusicPlayerX* musicPlayer;
    xMainMusicWidget* mainMusicWidget;
    xMovieLibrary* movieLibrary;
    xMoviePlayer* moviePlayer;
    xMainMovieWidget* mainMovieWidget;
    xMainStreamingWidget* mainStreamingWidget;
    xPlayerDBus* mainDbus;
    QStackedWidget* mainView;
};

#endif