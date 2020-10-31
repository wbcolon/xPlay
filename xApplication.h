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

#include <QSettings>
#include <QStackedWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

class xApplication:public QMainWindow {
public:
    xApplication(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xApplication() = default;

private slots:
    void configure();

private:
    void setMusicLibraryDirectory(const QString& directory);
    void createMenus();
    void configurationUpdate();

    QSettings* settings;
    QString musicLibraryDirectory;
    xMusicLibrary* musicLibrary;
    xMusicPlayerX* musicPlayer;
    xMainMusicWidget* mainMusicWidget;
    xMovieLibrary* movieLibrary;
    xMoviePlayer* moviePlayer;
    xMainMovieWidget* mainMovieWidget;
    QStackedWidget* mainView;
};

#endif