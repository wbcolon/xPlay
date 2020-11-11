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

#include <QSettings>
#include <QStackedWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

class xApplication:public QMainWindow {
public:
    xApplication(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xApplication() noexcept;

private slots:
    /**
     * Display the xPlay configuration dialog.
     */
    void configure();
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
    QStackedWidget* mainView;
};

#endif