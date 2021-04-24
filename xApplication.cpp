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
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QApplication>
#include <QGridLayout>
#include <QMetaType>

#include "xApplication.h"
#include "xMusicFile.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include "xPlayerConfigurationDialog.h"
#include "xPlayerDatabase.h"
#include "xPlayerConfig.h"

xApplication::xApplication(QWidget* parent, Qt::WindowFlags flags):
        QMainWindow(parent, flags) {
    // Register Type
    qRegisterMetaType<xMusicFile>();
    qRegisterMetaType<xMusicFile*>();
    qRegisterMetaType<std::list<xMusicFile*>>();
    qRegisterMetaType<std::vector<xMusicFile*>>();
    // Setup music and movie library.
    musicLibrary = new xMusicLibrary(this);
    movieLibrary = new xMovieLibrary(this);
    // Stack for different views.
    mainView = new QStackedWidget(this);
    // Setup players and main widgets
    musicPlayer = new xMusicPlayerX(musicLibrary, mainView);
    moviePlayer = new xMoviePlayerX(mainView);
    mainMusicWidget = new xMainMusicWidget(musicPlayer, musicLibrary, mainView);
    mainMovieWidget = new xMainMovieWidget(moviePlayer, mainView);
#ifdef USE_STREAMING
    mainStreamingWidget = new xMainStreamingWidget(mainView);
#else
    mainStreamingWidget = nullptr;
#endif
    // Add to the stack widget
    mainView->addWidget(mainMusicWidget);
    mainView->addWidget(mainMovieWidget);
#ifdef USE_STREAMING
    mainView->addWidget(mainStreamingWidget);
#endif
    mainView->setCurrentWidget(mainMusicWidget);
    mainDbus = new xPlayerDBus(mainView);
    // Use main widget as central application widget.
    setCentralWidget(mainView);
    // Create Application menus.
    createMenus();
    // Connect music library with main music widget.
    // Commands for the music library.
    connect(mainMusicWidget, SIGNAL(scan(xMusicLibraryFilter)), musicLibrary, SLOT(scan(xMusicLibraryFilter)));
    connect(mainMusicWidget, SIGNAL(scanForArtist(QString,xMusicLibraryFilter)),
            musicLibrary, SLOT(scanForArtist(QString,xMusicLibraryFilter)));
    connect(mainMusicWidget, &xMainMusicWidget::scanForArtistAndAlbum, musicLibrary, &xMusicLibrary::scanForArtistAndAlbum);
    connect(mainMusicWidget, SIGNAL(scanAllAlbumsForArtist(QString,xMusicLibraryFilter)),
            musicLibrary, SLOT(scanAllAlbumsForArtist(QString,xMusicLibraryFilter)));
    connect(mainMusicWidget, SIGNAL(scanAllAlbumsForListArtists(QStringList,xMusicLibraryFilter)),
            musicLibrary, SLOT(scanAllAlbumsForListArtists(QStringList,xMusicLibraryFilter&)));
    // Results back to the main music widget.
    connect(musicLibrary, &xMusicLibrary::scanningError, this, &xApplication::scanningErrorMusicLibrary);
    connect(musicLibrary, &xMusicLibrary::scannedArtists, mainMusicWidget, &xMainMusicWidget::scannedArtists);
    connect(musicLibrary, &xMusicLibrary::scannedAlbums, mainMusicWidget, &xMainMusicWidget::scannedAlbums);
    connect(musicLibrary, &xMusicLibrary::scannedTracks, mainMusicWidget, &xMainMusicWidget::scannedTracks);
    connect(musicLibrary, &xMusicLibrary::scannedAllAlbumTracks, mainMusicWidget, &xMainMusicWidget::scannedAllAlbumTracks);
    connect(musicLibrary, &xMusicLibrary::scannedListArtistsAllAlbumTracks, mainMusicWidget, &xMainMusicWidget::scannedListArtistsAllAlbumTracks);
    // Connect music or movie library for application.
    connect(musicLibrary, &xMusicLibrary::scannedUnknownEntries, this, &xApplication::unknownTracks);
    connect(movieLibrary, &xMovieLibrary::scannedUnknownEntries, this, &xApplication::unknownMovies);
    // Connect movie library with main movie widget
    connect(mainMovieWidget, &xMainMovieWidget::scanForTag, movieLibrary, &xMovieLibrary::scanForTag);
    connect(mainMovieWidget, &xMainMovieWidget::scanForTagAndDirectory, movieLibrary, &xMovieLibrary::scanForTagAndDirectory);
    // Results back to the main movie widget.
    connect(movieLibrary, &xMovieLibrary::scannedTags, mainMovieWidget, &xMainMovieWidget::scannedTags);
    connect(movieLibrary, &xMovieLibrary::scannedDirectories, mainMovieWidget, &xMainMovieWidget::scannedDirectories);
    connect(movieLibrary, &xMovieLibrary::scannedMovies, mainMovieWidget, &xMainMovieWidget::scannedMovies);
    // Connect window title and menu bar to main widgets.
    connect(mainMusicWidget, &xMainMusicWidget::showMenuBar, menuBar(), &QMenuBar::setVisible);
    connect(mainMusicWidget, &xMainMusicWidget::showWindowTitle, this, &xApplication::setWindowTitle);
    connect(mainMovieWidget, &xMainMovieWidget::showMenuBar, menuBar(), &QMenuBar::setVisible);
    connect(mainMovieWidget, &xMainMovieWidget::showWindowTitle, this, &xApplication::setWindowTitle);
#ifdef USE_STREAMING
    connect(mainStreamingWidget, &xMainStreamingWidget::showMenuBar, menuBar(), &QMenuBar::setVisible);
    connect(mainStreamingWidget, &xMainStreamingWidget::showWindowTitle, this, &xApplication::setWindowTitle);
#endif
    // Do not connect main widget to music player here. It is done in the main widget
    // Connect Settings.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryDirectory,
            this, &xApplication::setMusicLibraryDirectory);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieLibraryTagsAndDirectories,
            this, &xApplication::setMovieLibraryTagsAndDirectories);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelNetworkAddress,
            this, &xApplication::setRotelNetworkAddress);
#ifdef USE_STREAMING
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedStreamingSites,
            this, &xApplication::setStreamingSites);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedStreamingSitesDefault,
            this, &xApplication::setStreamingSitesDefault);
#endif
    // Connect database.
    connect(xPlayerDatabase::database(), &xPlayerDatabase::databaseUpdateError,
            this, &xApplication::databaseUpdateError);
    // Signal configuration updates.
    xPlayerConfiguration::configuration()->updatedConfiguration();
    // Connect signals generated by dbus commands.
    connect(mainDbus, &xPlayerDBus::dbus_playPause, this, &xApplication::dbus_playPause);
    connect(mainDbus, &xPlayerDBus::dbus_stop, this, &xApplication::dbus_stop);
    connect(mainDbus, &xPlayerDBus::dbus_prev, this, &xApplication::dbus_prev);
    connect(mainDbus, &xPlayerDBus::dbus_next, this, &xApplication::dbus_next);
    connect(mainDbus, &xPlayerDBus::dbus_jump, this, &xApplication::dbus_jump);
    connect(mainDbus, &xPlayerDBus::dbus_fullWindow, this, &xApplication::dbus_fullWindow);
    connect(mainDbus, &xPlayerDBus::dbus_scaleAndCrop, this, &xApplication::dbus_scaleAndCrop);
    connect(mainDbus, &xPlayerDBus::dbus_mute, this, &xApplication::dbus_mute);
    connect(mainDbus, &xPlayerDBus::dbus_changeVolume, this, &xApplication::dbus_changeVolume);
    connect(mainDbus, &xPlayerDBus::dbus_selectView, this, &xApplication::dbus_selectView);
    connect(mainDbus, &xPlayerDBus::dbus_muteRotel, this, &xApplication::dbus_muteRotel);
    connect(mainDbus, &xPlayerDBus::dbus_changeRotelVolume, this, &xApplication::dbus_changeRotelVolume);
    connect(mainDbus, &xPlayerDBus::dbus_selectRotelSource, this, &xApplication::dbus_selectRotelSource);
    // Set title and connect signal from movie view.
    setWindowTitle(QApplication::applicationName());
}

void xApplication::closeEvent(QCloseEvent* event) {
    // Do some local cleanup before closing.
    musicLibrary->cleanup();
    QMainWindow::closeEvent(event);
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

void xApplication::dbus_prev() {
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
    auto currentWidget = mainView->currentWidget();
    if (currentWidget == mainMusicWidget) {
        musicPlayer->jump(delta);
    } else if (currentWidget == mainMovieWidget) {
        moviePlayer->jump(delta);
    }
}

void xApplication::dbus_fullWindow() {
    if (mainView->currentWidget() == mainMovieWidget) {
        emit moviePlayer->toggleFullWindow();
    }
}

void xApplication::dbus_scaleAndCrop() {
    if (mainView->currentWidget() == mainMovieWidget) {
        moviePlayer->toggleScaleAndCropMode();
    }
}

void xApplication::dbus_mute() {
    // Set the volume to mute and emit the update to the volume widget.
    auto currentWidget = mainView->currentWidget();
    if (currentWidget == mainMusicWidget) {
        musicPlayer->setMuted(!musicPlayer->isMuted());
    } else if (currentWidget == mainMovieWidget) {
        moviePlayer->setMuted(!moviePlayer->isMuted());
    } else if (currentWidget == mainStreamingWidget) {
        mainStreamingWidget->setMuted(!mainStreamingWidget->isMuted());
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

void xApplication::dbus_selectView(const QString& view) {
    // Change and initialize view.
    if (!view.compare("music", Qt::CaseInsensitive)) {
        mainView->setCurrentWidget(mainMusicWidget);
        mainMusicWidget->initializeView();
    } else if (!view.compare("movie", Qt::CaseInsensitive)) {
        mainView->setCurrentWidget(mainMovieWidget);
        mainMovieWidget->initializeView();
    } else if (!view.compare("streaming", Qt::CaseInsensitive)) {
#ifdef USE_STREAMING
        mainView->setCurrentWidget(mainStreamingWidget);
        mainStreamingWidget->initializeView();
#endif
    }
}

void xApplication::dbus_muteRotel() {
    // Mute the Rotel amp.
    auto rotelControls = xPlayerRotelControls::controls();
    rotelControls->setMuted(!rotelControls->isMuted());
}

void xApplication::dbus_changeRotelVolume(int delta) {
    // Set the volume for the Rotel amp.
    auto rotelControls = xPlayerRotelControls::controls();
    auto rotelVolume = rotelControls->getVolume();
    // Only adjust volume if getVolume was successful.
    if (rotelVolume >= 0) {
        rotelControls->setVolume(rotelVolume+delta);
    }
}

void xApplication::dbus_selectRotelSource(const QString& source) {
    // Set a new source for the Rotel amp.
    xPlayerRotelControls::controls()->setSource(source);
}

void xApplication::unknownTracks(const std::list<std::tuple<QString,QString,QString>>& entries) {
    // Do we have any unknown tracks.
    if (entries.empty()) {
        QMessageBox::information(this, "Music Database", "No unknown entries found.");
    } else {
        if (unknownEntriesDialog("Music Database", entries) == QDialog::Accepted) {
            xPlayerDatabase::database()->removeTracks(entries);
        }
    }
}

void xApplication::unknownMovies(const std::list<std::tuple<QString,QString,QString>>& entries) {
    // Do we have any unknown tracks.
    if (entries.empty()) {
        QMessageBox::information(this, "Movie Database", "No unknown entries found.");
    } else {
        if (unknownEntriesDialog("Movie Database", entries) == QDialog::Accepted) {
            xPlayerDatabase::database()->removeMovies(entries);
        }
    }
}

int xApplication::unknownEntriesDialog(const QString& dialogTitle, const std::list<std::tuple<QString, QString, QString>>& entries) {
    auto unknownDialog = new QDialog(this);
    unknownDialog->setWindowTitle(dialogTitle);
    auto unknownDialogLayout = new xPlayerLayout();
    auto unknownEntriesList = new QListWidget(unknownDialog);
    // Fill list widget.
    for (const auto& entry : entries) {
        unknownEntriesList->addItem(QString("%1/%2/%3").arg(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry)));
    }
    auto unknownDialogButtons = new QDialogButtonBox(unknownDialog);
    unknownDialogButtons->addButton(QDialogButtonBox::Discard);
    unknownDialogButtons->addButton(QDialogButtonBox::Cancel);
    unknownDialogButtons->button(QDialogButtonBox::Discard)->setText(tr("Remove"));
    unknownDialogLayout->addWidget(new QLabel(tr("Unknown Entries"), unknownDialog), 0, 0, 1, 4);
    unknownDialogLayout->addWidget(unknownEntriesList, 1, 0, 4, 4);
    unknownDialogLayout->setRowStretch(1, 2);
    unknownDialogLayout->addRowSpacer(5, xPlayerLayout::LargeSpace);
    unknownDialogLayout->addWidget(unknownDialogButtons, 6, 0, 4, 4);
    unknownDialog->setLayout(unknownDialogLayout);
    // Connect buttons to playlist actions and close afterwards.
    connect(unknownDialogButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed, unknownDialog, &QDialog::accept);
    connect(unknownDialogButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, unknownDialog, &QDialog::reject);
    unknownDialog->resize(unknownDialog->sizeHint());
    // Set the minimum width as 3/2 of its height.
    unknownDialog->setMinimumWidth(unknownDialog->sizeHint().height()*3/2);
    return unknownDialog->exec();
}

void xApplication::setMusicLibraryDirectory() {
    auto musicLibraryDirectory=xPlayerConfiguration::configuration()->getMusicLibraryDirectory();
    mainMusicWidget->clear();
    musicLibrary->setBaseDirectory(std::filesystem::path(musicLibraryDirectory.toStdString()));
    qInfo() << "Update music library path to " << musicLibraryDirectory;
}

void xApplication::scanningErrorMusicLibrary() {
    QMessageBox::critical(this, "Music Library",
                          "Unable to scan the music error. Please check your configuration.");
}

void xApplication::setRotelNetworkAddress() {
    auto [rotelAddress,rotelPort] = xPlayerConfiguration::configuration()->getRotelNetworkAddress();
    xPlayerRotelControls::controls()->connect(rotelAddress, rotelPort);
}

void xApplication::setMovieLibraryTagsAndDirectories() {
    mainMovieWidget->clear();
    movieLibrary->setBaseDirectories(xPlayerConfiguration::configuration()->getMovieLibraryTagAndDirectoryPath());
}

void xApplication::setStreamingSites() {
    mainStreamingWidget->setSites(xPlayerConfiguration::configuration()->getStreamingSites());
}

void xApplication::setStreamingSitesDefault() {
    mainStreamingWidget->setSitesDefault(xPlayerConfiguration::configuration()->getStreamingSitesDefault());
}

void xApplication::checkMusicDatabase() {
    musicLibrary->scanForUnknownEntries(xPlayerDatabase::database()->getAllTracks());
}

void xApplication::checkMovieDatabase() {
    movieLibrary->scanForUnknownEntries(xPlayerDatabase::database()->getAllMovies());
}

void xApplication::createMenus() {
    // Create actions for file menu.
    auto fileMenuConfigure = new QAction("&Configure", this);
    auto fileMenuRescanMusicLibrary = new QAction("Rescan M&usic Library", this);
    auto fileMenuRescanMovieLibrary = new QAction("Rescan M&ovie Library", this);
    auto fileMenuCheckMusicDatabase = new QAction("Check Mu&sic Database", this);
    auto fileMenuCheckMovieDatabase = new QAction("Check Mo&vie Database", this);
    auto fileMenuExitAction = new QAction(tr("&Exit"), this);
    // Connect actions from file menu.
    connect(fileMenuConfigure, &QAction::triggered, this, &xApplication::configure);
    connect(fileMenuRescanMusicLibrary, &QAction::triggered, this, &xApplication::setMusicLibraryDirectory);
    connect(fileMenuRescanMovieLibrary, &QAction::triggered, this, &xApplication::setMovieLibraryTagsAndDirectories);
    connect(fileMenuCheckMusicDatabase, &QAction::triggered, this, &xApplication::checkMusicDatabase);
    connect(fileMenuCheckMovieDatabase, &QAction::triggered, this, &xApplication::checkMovieDatabase);
    connect(fileMenuExitAction, &QAction::triggered, this, &xApplication::close);
    // Create file menu.
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(fileMenuConfigure);
    fileMenu->addSeparator();
    fileMenu->addAction(fileMenuRescanMusicLibrary);
    fileMenu->addAction(fileMenuRescanMovieLibrary);
    fileMenu->addSeparator();
    fileMenu->addAction(fileMenuCheckMusicDatabase);
    fileMenu->addAction(fileMenuCheckMovieDatabase);
    fileMenu->addSeparator();
    fileMenu->addAction(fileMenuExitAction);
    // Create actions for view menu.
    auto viewMenuSelectMusic = new QAction("Select M&usic View", this);
    auto viewMenuSelectMovie = new QAction("Select M&ovie View", this);
#ifdef USE_STREAMING
    auto viewMenuSelectStreaming = new QAction("Select Str&eaming View", this);
#endif
    // Connect actions from view menu.
    connect(viewMenuSelectMusic, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainMusicWidget);
        mainMusicWidget->initializeView();
    });
    connect(viewMenuSelectMovie, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainMovieWidget);
        mainMovieWidget->initializeView();
    });
#ifdef USE_STREAMING
    connect(viewMenuSelectStreaming, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainStreamingWidget);
        mainStreamingWidget->initializeView();
    });
#endif
    // Create view menu.
    auto viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(viewMenuSelectMusic);
    viewMenu->addAction(viewMenuSelectMovie);
#ifdef USE_STREAMING
    viewMenu->addAction(viewMenuSelectStreaming);
#endif
    // Create actions for help menu
    auto helpMenuAboutQt = new QAction("About Qt", this);
    auto helpMenuAboutQwt = new QAction("About Qwt", this);
    // Connect actions from view menu.
    connect(helpMenuAboutQt, &QAction::triggered, [=]() { QMessageBox::aboutQt(this, "About Qt"); });
    connect(helpMenuAboutQwt, &QAction::triggered, [=]() { QMessageBox::about(this, "About Qwt",
        // The text for Qwt taken from their website and license.
        "<p><b>Qwt - Qt Widgets for Technical Applications</b></p>"
        "<p>The Qwt library contains GUI Components and utility classes which are primarily "
        "useful for programs with a technical background. Beside a framework for 2D plots it "
        "provides scales, sliders, dials, compasses, thermometers, wheels and knobs to "
        "control or display values, arrays, or ranges of type double.</p>"
        "<p><i>xPlay is based in part on the work of the Qwt project (http://qwt.sf.net).</i></p>"); });
    // Create help menu.
    auto helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpMenuAboutQt);
    helpMenu->addAction(helpMenuAboutQwt);
}

void xApplication::configure() {
    xPlayerConfigurationDialog configurationDialog;
    configurationDialog.show();
    configurationDialog.exec();
}

void xApplication::databaseUpdateError() {
    if (xPlayerConfiguration::configuration()->getDatabaseIgnoreUpdateErrors()) {
        qDebug() << "Ignoring database update errors.";
        return;
    }
    QMessageBox::StandardButton result =
            QMessageBox::critical(this, "Database Error",
                                  "Unable to update the entries in the xPlay database.",
                                  QMessageBox::Ok | QMessageBox::Ignore, QMessageBox::Ignore);
    if (result == QMessageBox::Ignore) {
        xPlayerConfiguration::configuration()->setDatabaseIgnoreUpdateErrors(true);
    }
}
