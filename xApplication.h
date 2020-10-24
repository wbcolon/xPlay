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

#include <QSettings>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "xMusicLibrary.h"
#include "xMusicPlayerX.h"
#include "xMainWidget.h"

class xApplication:public QMainWindow {
public:
    xApplication(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xApplication() = default;

private slots:
    void openMusicLibrary();

private:
    void setMusicLibraryDirectory(const QString& directory);
    void createMenus();

    QSettings* settings;
    QString musicLibraryDirectory;
    xMusicLibrary* musicLibrary;
    xMusicPlayerX* musicPlayer;
    xMainWidget* mainWidget;
};

#endif