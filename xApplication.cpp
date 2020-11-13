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
#include <QMenuBar>
#include <QFileDialog>
#include <QInputDialog>

#include "xApplication.h"
#include "xPlayerConfiguration.h"
#include "xPlayerConfigurationDialog.h"

xApplication::xApplication(QWidget* parent, Qt::WindowFlags flags):
        QMainWindow(parent, flags) {
    // Setup music and movie library.
    musicLibrary = new xMusicLibrary(this);
    movieLibrary = new xMovieLibrary(this);
    // Stack for different views.
    mainView = new QStackedWidget(this);
    // Setup players and main widgets
    musicPlayer = new xMusicPlayerX(mainView);
    moviePlayer = new xMoviePlayer(mainView);
    mainMusicWidget = new xMainMusicWidget(musicPlayer, mainView);
    mainMovieWidget = new xMainMovieWidget(moviePlayer, mainView);
    mainStreamingWidget = new xMainStreamingWidget(mainView);
    mainDbus = new xPlayerDBus(mainView);
    // Add to the stack widget
    mainView->addWidget(new QWidget(mainView));
    mainView->addWidget(mainMusicWidget);
    mainView->addWidget(mainMovieWidget);
    mainView->addWidget(mainStreamingWidget);
    mainView->setCurrentWidget(mainMusicWidget);
    // Use main widget as central application widget.
    setCentralWidget(mainView);
    // Connect music library with main music widget.
    // Commands for the music library.
    connect(mainMusicWidget, &xMainMusicWidget::scanForArtist, musicLibrary, &xMusicLibrary::scanForArtist);
    connect(mainMusicWidget, &xMainMusicWidget::scanForArtistAndAlbum, musicLibrary, &xMusicLibrary::scanForArtistAndAlbum);
    // Results back to the main music widget.
    connect(musicLibrary, &xMusicLibrary::scannedArtists, mainMusicWidget, &xMainMusicWidget::scannedArtists);
    connect(musicLibrary, &xMusicLibrary::scannedAlbums, mainMusicWidget, &xMainMusicWidget::scannedAlbums);
    connect(musicLibrary, &xMusicLibrary::scannedTracks, mainMusicWidget, &xMainMusicWidget::scannedTracks);
    // Connect movie library with main movie widget
    connect(mainMovieWidget, &xMainMovieWidget::scanForTag, movieLibrary, &xMovieLibrary::scanForTag);
    connect(mainMovieWidget, &xMainMovieWidget::scanForTagAndDirectory, movieLibrary, &xMovieLibrary::scanForTagAndDirectory);
    connect(mainMovieWidget, &xMainMovieWidget::playMovie, moviePlayer, &xMoviePlayer::setMovie);
    // Results back to the main movie widget.
    connect(movieLibrary, &xMovieLibrary::scannedTags, mainMovieWidget, &xMainMovieWidget::scannedTags);
    connect(movieLibrary, &xMovieLibrary::scannedDirectories, mainMovieWidget, &xMainMovieWidget::scannedDirectories);
    connect(movieLibrary, &xMovieLibrary::scannedMovies, mainMovieWidget, &xMainMovieWidget::scannedMovies);
    // Do not connect main widget to music player here. It is done in the main widget
    // Connect Settings.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryDirectory,
            this, &xApplication::setMusicLibraryDirectory);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieLibraryTagsAndDirectories,
            this, &xApplication::setMovieLibraryTagsAndDirectories);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelNetworkAddress,
            this, &xApplication::setRotelNetworkAddress);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedStreamingSites,
            this, &xApplication::setStreamingSites);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedStreamingSitesDefault,
            this, &xApplication::setStreamingSitesDefault);
    // Dbus.
    connect(mainDbus, &xPlayerDBus::dbus_playPause, this, &xApplication::dbus_playPause);
    connect(mainDbus, &xPlayerDBus::dbus_stop, this, &xApplication::dbus_stop);
    connect(mainDbus, &xPlayerDBus::dbus_previous, this, &xApplication::dbus_previous);
    connect(mainDbus, &xPlayerDBus::dbus_next, this, &xApplication::dbus_next);
    connect(mainDbus, &xPlayerDBus::dbus_jump, this, &xApplication::dbus_jump);
    connect(mainDbus, &xPlayerDBus::dbus_toggleFullWindow, this, &xApplication::dbus_toggleFullWindow);
    connect(mainDbus, &xPlayerDBus::dbus_toggleScaleAndCrop, this, &xApplication::dbus_toggleScaleAndCrop);
    connect(mainDbus, &xPlayerDBus::dbus_mute, this, &xApplication::dbus_mute);
    connect(mainDbus, &xPlayerDBus::dbus_changeVolume, this, &xApplication::dbus_changeVolume);
    connect(mainDbus, &xPlayerDBus::dbus_selectView, this, &xApplication::dbus_selectView);
    // Signal configuration updates.
    xPlayerConfiguration::configuration()->updatedConfiguration();
    // Create Application menus.
    createMenus();
}

xApplication::~xApplication() noexcept {
}

void xApplication::dbus_playPause() {
    auto currentWidget = mainView->currentWidget();
    if (currentWidget == mainMusicWidget) {
        musicPlayer->playPause();
    } else if (currentWidget == mainMovieWidget) {
        moviePlayer->playPause();
    }
}

void xApplication::dbus_stop() {
    auto currentWidget = mainView->currentWidget();
    if (currentWidget == mainMusicWidget) {
        musicPlayer->stop();
    } else if (currentWidget == mainMovieWidget) {
        moviePlayer->stop();
    }
}

void xApplication::dbus_previous() {
    if (mainView->currentWidget() == mainMusicWidget) {
        musicPlayer->prev();
    }
}

void xApplication::dbus_next() {
    if (mainView->currentWidget() == mainMusicWidget) {
        musicPlayer->next();
    }
}

void xApplication::dbus_jump(qint64 delta) {
    if (mainView->currentWidget() == mainMovieWidget) {
        moviePlayer->jump(delta);
    }
}

void xApplication::dbus_toggleFullWindow() {
    if (mainView->currentWidget() == mainMovieWidget) {
        emit moviePlayer->toggleFullWindow();
    }
}

void xApplication::dbus_toggleScaleAndCrop() {
    if (mainView->currentWidget() == mainMovieWidget) {
        moviePlayer->toggleScaleAndCropMode();
    }
}

void xApplication::dbus_mute() {
    // Set the volume to mute and emit the update to the volume widget.
    auto currentWidget = mainView->currentWidget();
    if (currentWidget == mainMusicWidget) {
        musicPlayer->setVolume(0);
        emit musicPlayer->currentVolume(0);
    } else if (currentWidget == mainMovieWidget) {
        moviePlayer->setVolume(0);
        emit moviePlayer->currentVolume(0);
    }
}

void xApplication::dbus_changeVolume(int delta) {
    // Set the volume and emit the update to the volume widget.
    auto currentWidget = mainView->currentWidget();
    if (currentWidget == mainMusicWidget) {
        musicPlayer->setVolume(musicPlayer->getVolume()+delta);
        emit musicPlayer->currentVolume( musicPlayer->getVolume());
    } else if (currentWidget == mainMovieWidget) {
        moviePlayer->setVolume(moviePlayer->getVolume()+delta);
        emit moviePlayer->currentVolume( moviePlayer->getVolume());
    }
}

void xApplication::dbus_selectView(QString view) {
    view = view.toLower();
    if (view == "music") {
        mainView->setCurrentWidget(mainMusicWidget);
    } else if (view == "movie") {
        mainView->setCurrentWidget(mainMovieWidget);
    } else if (view == "streaming") {
        mainView->setCurrentWidget(mainStreamingWidget);
    }
}


void xApplication::setMusicLibraryDirectory() {
    auto musicLibraryDirectory=xPlayerConfiguration::configuration()->getMusicLibraryDirectory();
    musicLibrary->setBaseDirectory(std::filesystem::path(musicLibraryDirectory.toStdString()));
    musicPlayer->setBaseDirectory(musicLibraryDirectory);
    qInfo() << "Update music library path to " << musicLibraryDirectory;
}

void xApplication::setRotelNetworkAddress() {
    auto [rotelAddress,rotelPort] = xPlayerConfiguration::configuration()->getRotelNetworkAddress();
    xPlayerRotelControls::controls()->connect(rotelAddress, rotelPort);
}

void xApplication::setMovieLibraryTagsAndDirectories() {
    movieLibrary->setBaseDirectories(xPlayerConfiguration::configuration()->getMovieLibraryTagAndDirectoryPath());
}

void xApplication::setStreamingSites() {
    mainStreamingWidget->setSites(xPlayerConfiguration::configuration()->getStreamingSites());
}

void xApplication::setStreamingSitesDefault() {
    mainStreamingWidget->setSitesDefault(xPlayerConfiguration::configuration()->getStreamingSitesDefault());
}

void xApplication::createMenus() {
    // Create actions for file menu.
    auto fileMenuConfigure = new QAction("&Configure", this);
    auto fileMenuRescanMusicLibrary = new QAction("Rescan M&usic Library", this);
    auto fileMenuRescanMovieLibrary = new QAction("Rescan M&ovie Library", this);
    auto fileMenuExitAction = new QAction(tr("&Exit"), this);
    // Connect actions from file menu.
    connect(fileMenuConfigure, &QAction::triggered, this, &xApplication::configure);
    connect(fileMenuRescanMusicLibrary, &QAction::triggered, this, &xApplication::setMusicLibraryDirectory);
    connect(fileMenuRescanMovieLibrary, &QAction::triggered, this, &xApplication::setMovieLibraryTagsAndDirectories);
    connect(fileMenuExitAction, &QAction::triggered, this, &xApplication::close);
    // Create file menu.
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(fileMenuConfigure);
    fileMenu->addSeparator();
    fileMenu->addAction(fileMenuRescanMusicLibrary);
    fileMenu->addAction(fileMenuRescanMovieLibrary);
    fileMenu->addSeparator();
    fileMenu->addAction(fileMenuExitAction);
    // Create actions for view menu.
    auto viewMenuSelectMusic = new QAction("Select M&usic View", this);
    auto viewMenuSelectMovie = new QAction("Select M&ovie View", this);
    auto viewMenuSelectStreaming = new QAction("Select Str&eaming View", this);
    // Connect actions from view menu.
    connect(viewMenuSelectMusic, &QAction::triggered, [=]() { mainView->setCurrentWidget(mainMusicWidget); });
    connect(viewMenuSelectMovie, &QAction::triggered, [=]() { mainView->setCurrentWidget(mainMovieWidget); });
    connect(viewMenuSelectStreaming, &QAction::triggered, [=]() { mainView->setCurrentWidget(mainStreamingWidget); });
    // Create view menu.
    auto viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(viewMenuSelectMusic);
    viewMenu->addAction(viewMenuSelectMovie);
    viewMenu->addAction(viewMenuSelectStreaming);
}

void xApplication::configure() {
    xPlayerConfigurationDialog configurationDialog;
    configurationDialog.show();
    configurationDialog.exec();
}

