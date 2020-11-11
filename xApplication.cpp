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
    // Signal configuration updates.
    xPlayerConfiguration::configuration()->updatedConfiguration();
    // Create Application menus.
    createMenus();
}

xApplication::~xApplication() noexcept {
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

