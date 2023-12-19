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
#include <QMetaType>

#include "xApplication.h"
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include "xPlayerConfigurationDialog.h"
#include "xPlayerDatabase.h"
#include "xPlayerConfig.h"
#include "xPlayerPulseAudioControls.h"
#include "xPlayerBluOSControl.h"

xApplication::xApplication(QWidget* parent, Qt::WindowFlags flags):
        QMainWindow(parent, flags),
        optionsMenuBars(nullptr),
        musicOptionsMenuBar(nullptr),
        musicOptionsVisualization(nullptr),
        movieOptionsMenuBar(nullptr),
        streamingOptionsMenuBar(nullptr),
        mobileSyncOptionsMenuBar(nullptr) {
    // Register Type
    qRegisterMetaType<xMusicLibraryTrackEntry>();
    qRegisterMetaType<xMusicLibraryTrackEntry*>();
    qRegisterMetaType<std::list<xMusicLibraryTrackEntry*>>();
    qRegisterMetaType<std::vector<xMusicLibraryTrackEntry*>>();
    qRegisterMetaType<xMusicLibraryAlbumEntry>();
    qRegisterMetaType<xMusicLibraryAlbumEntry*>();
    qRegisterMetaType<std::list<xMusicLibraryAlbumEntry*>>();
    qRegisterMetaType<std::vector<xMusicLibraryAlbumEntry*>>();
    qRegisterMetaType<xMusicLibraryArtistEntry>();
    qRegisterMetaType<xMusicLibraryArtistEntry*>();
    qRegisterMetaType<std::list<xMusicLibraryArtistEntry*>>();
    qRegisterMetaType<std::vector<xMusicLibraryArtistEntry*>>();
    qRegisterMetaType<xMovieLibraryEntry>();
    qRegisterMetaType<xMovieLibraryEntry*>();
    qRegisterMetaType<std::filesystem::path>();
    // Setup music and movie library.
    musicLibrary = new xMusicLibrary(this);
    movieLibrary = new xMovieLibrary(this);
    // Stack for different views.
    mainView = new QStackedWidget(this);
    // Setup players and main widgets
    musicPlayer = new xMusicPlayer(musicLibrary, mainView);
    moviePlayer = new xMoviePlayer(mainView);
    mainMusicWidget = new xMainMusicWidget(musicPlayer, musicLibrary, mainView);
    mainMovieWidget = new xMainMovieWidget(moviePlayer, mainView);
    mainMobileSyncWidget = new xMainMobileSyncWidget(musicLibrary, mainView);
#ifdef USE_STREAMING
    mainStreamingWidget = new xMainStreamingWidget(mainView);
#else
    mainStreamingWidget = nullptr;
#endif

    // Add to the stack widget
    mainView->addWidget(mainMusicWidget);
    mainView->addWidget(mainMovieWidget);
    mainView->addWidget(mainMobileSyncWidget);
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
            musicLibrary, SLOT(scanAllAlbumsForListArtists(QStringList,xMusicLibraryFilter)));
    // Results back to the main music widget.
    connect(musicLibrary, &xMusicLibrary::scanningError, this, &xApplication::scanningErrorMusicLibrary);
    connect(musicLibrary, &xMusicLibrary::scanningProgress, mainMusicWidget, &xMainMusicWidget::scanningProgress);
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
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryDirectory, [=]() {
        setMusicLibrary();
    });
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryBluOS, [=]() {
        setMusicLibrary();
    });
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedUseMusicLibraryBluOS, [=]() {
        setMusicLibrary();
    });
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieLibraryTagsAndDirectories,
            this, &xApplication::setMovieLibraryTagsAndDirectories);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelNetworkAddress,
            this, &xApplication::setRotelNetworkAddress);
    // Connect database.
    connect(xPlayerDatabase::database(), &xPlayerDatabase::databaseUpdateError,
            this, &xApplication::databaseUpdateError);
    // Connect pulseaudio controls.
    connect(xPlayerPulseAudioControls::controls(), &xPlayerPulseAudioControls::error,
            this, &xApplication::pulseAudioError);
    // Connect to movie player errors.
    connect(moviePlayer, &xMoviePlayer::moviePlayerError, this, &xApplication::moviePlayerError);
    // Signal configuration updates.
    xPlayerConfiguration::configuration()->updatedConfiguration();
    // Connect signals generated by dbus commands.
    connect(mainDbus, &xPlayerDBus::dbus_playPause, this, &xApplication::dbus_playPause);
    connect(mainDbus, &xPlayerDBus::dbus_stop, this, &xApplication::dbus_stop);
    connect(mainDbus, &xPlayerDBus::dbus_prev, this, &xApplication::dbus_prev);
    connect(mainDbus, &xPlayerDBus::dbus_next, this, &xApplication::dbus_next);
    connect(mainDbus, &xPlayerDBus::dbus_jump, this, &xApplication::dbus_jump);
    connect(mainDbus, &xPlayerDBus::dbus_fullWindow, this, &xApplication::dbus_fullWindow);
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
    musicLibrary->clear();
    // Disconnect rotel controls.
    xPlayerRotelControls::controls()->disconnect();
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
        // Implement toggle.
        moviePlayer->setFullWindowMode(!moviePlayer->getFullWindowMode());
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
    } else if (!view.compare("mobilesync", Qt::CaseInsensitive)) {
        mainView->setCurrentWidget(mainMobileSyncWidget);
        mainMobileSyncWidget->initializeView();
    } else if (!view.compare("streaming", Qt::CaseInsensitive)) {
#ifdef USE_STREAMING
        mainView->setCurrentWidget(mainStreamingWidget);
        mainStreamingWidget->initializeView();
#endif
    }
}

void xApplication::dbus_muteRotel() {
    if (mainDbusMutex.try_lock()) {
        // Mute the Rotel amp.
        auto rotelControls = xPlayerRotelControls::controls();
        rotelControls->setMuted(!rotelControls->isMuted());
        mainDbusMutex.unlock();
    }
}

void xApplication::dbus_changeRotelVolume(int delta) {
    if (mainDbusMutex.try_lock()) {
        // Set the volume for the Rotel amp.
        auto rotelControls = xPlayerRotelControls::controls();
        auto rotelVolume = rotelControls->getVolume();
        // Only adjust volume if getVolume was successful.
        if (rotelVolume >= 0) {
            rotelControls->setVolume(rotelVolume+delta);
        }
        mainDbusMutex.unlock();
    }
}

void xApplication::dbus_selectRotelSource(const QString& source) {
    if (mainDbusMutex.try_lock()) {
        // Set a new source for the Rotel amp.
        xPlayerRotelControls::controls()->setSource(source);
        mainDbusMutex.unlock();
    }
}

void xApplication::pulseAudioError(const QString& errorMsg) {
    QMessageBox::warning(this, "PulseAudio Error", errorMsg);
}

void xApplication::moviePlayerError(const QString& errorMsg) {
    QMessageBox::warning(this, "Movie Player Error", errorMsg);
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
    for (const auto& [level0, level1, level2] : entries) {
        unknownEntriesList->addItem(QString("%1/%2/%3").arg(level0, level1, level2));
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
    // Connect buttons to playlist actions and close afterward.
    connect(unknownDialogButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed, unknownDialog, &QDialog::accept);
    connect(unknownDialogButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, unknownDialog, &QDialog::reject);
    unknownDialog->resize(unknownDialog->sizeHint());
    // Set the minimum width as 3/2 of its height.
    unknownDialog->setMinimumWidth(unknownDialog->sizeHint().height()*3/2);
    return unknownDialog->exec();
}

void xApplication::setMusicLibrary(bool force) {
    QUrl musicLibraryUrl;
    if (xPlayerConfiguration::configuration()->useMusicLibraryBluOS()) {
        musicLibraryUrl = QUrl(xPlayerConfiguration::configuration()->getMusicLibraryBluOS());
    } else {
        musicLibraryUrl = QUrl::fromLocalFile(xPlayerConfiguration::configuration()->getMusicLibraryDirectory());
    }
    mainMusicWidget->clear();
    qInfo() << "Update music library path to " << musicLibraryUrl;
    musicLibrary->setUrl(musicLibraryUrl, force);
}

void xApplication::scanningErrorMusicLibrary() {
    QMessageBox::critical(this, "Music Library",
                          "Unable to scan the music error. Please check your configuration.");
}

void xApplication::setRotelNetworkAddress() {
    if (mainDbusMutex.try_lock()) {
        xPlayerRotelControls::controls()->disconnect();
        auto [rotelAddress,rotelPort] = xPlayerConfiguration::configuration()->getRotelNetworkAddress();
        xPlayerRotelControls::controls()->connect(rotelAddress, rotelPort);
        mainDbusMutex.unlock();
    }
}

void xApplication::setMovieLibraryTagsAndDirectories() {
    mainMovieWidget->clear();
    movieLibrary->setBaseDirectories(xPlayerConfiguration::configuration()->getMovieLibraryTagAndDirectoryPath());
}

void xApplication::checkMusicDatabase() {
    musicLibrary->scanForUnknownEntries(xPlayerDatabase::database()->getAllTracks());
}

void xApplication::checkMovieDatabase() {
    movieLibrary->scanForUnknownEntries(xPlayerDatabase::database()->getAllMovies());
}

void xApplication::reIndexMusicLibraryBluOS() {
    QMessageBox reIndexBox;
    reIndexBox.setText("ReIndexing BluOS Player Library");
    reIndexBox.setInformativeText("Scanned 0 tracks");
    reIndexBox.setStandardButtons(QMessageBox::Ok);
    connect(xPlayerBluOSControls::controls(), &xPlayerBluOSControls::playerReIndexing, [&reIndexBox](int noTracks) {
        if (noTracks > 0) {
            reIndexBox.buttons()[0]->setEnabled(false);
            reIndexBox.setInformativeText(QString("Scanned %1 tracks").arg(noTracks));
        } else {
            reIndexBox.buttons()[0]->setEnabled(true);
        }
    });
    // Call reindex command.
    xPlayerBluOSControls::controls()->reIndex();
    reIndexBox.exec();
    // Rescan library.
    setMusicLibrary(true);
}

void xApplication::createMenus() {
    // Create main menu with hamburger icon.
    auto mainMenu = menuBar()->addMenu(QIcon(":images/xplay-menu.svg") ,"");
    // Create actions for view menu.
    auto mainMenuConfigure = new QAction("&Configure", this);
    auto mainMenuMusic = new QAction("Select M&usic View", this);
    auto mainMenuMovie = new QAction("Select M&ovie View", this);
#ifdef USE_STREAMING
    auto mainMenuStreaming = new QAction("Select Str&eaming View", this);
#endif
    auto mainMenuMobileSync = new QAction("Select Mobile S&ync View", this);
    auto mainMenuAboutQt = new QAction("About Qt", this);
    auto mainMenuExit = new QAction(tr("&Exit"), this);

    // Add menu entries.
    mainMenu->addAction(mainMenuConfigure);
    mainMenu->addSeparator();
    mainMenu->addAction(mainMenuMusic);
    mainMenu->addAction(mainMenuMovie);
#ifdef USE_STREAMING
    mainMenu->addAction(mainMenuStreaming);
#endif
    mainMenu->addAction(mainMenuMobileSync);
    mainMenu->addSeparator();
    mainMenu->addAction(mainMenuAboutQt);
    mainMenu->addSeparator();
    mainMenu->addAction(mainMenuExit);

    // Connect actions to main menu.
    connect(mainMenuMusic, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainMusicWidget);
        mainMusicWidget->initializeView();
        optionsMenuBars->setCurrentWidget(musicOptionsMenuBar);
    });
    connect(mainMenuMovie, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainMovieWidget);
        mainMovieWidget->initializeView();
        optionsMenuBars->setCurrentWidget(movieOptionsMenuBar);
    });
#ifdef USE_STREAMING
    connect(mainMenuStreaming, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainStreamingWidget);
        mainStreamingWidget->initializeView();
        optionsMenuBars->setCurrentWidget(streamingOptionsMenuBar);
    });
#endif
    connect(mainMenuMobileSync, &QAction::triggered, [=]() {
        mainView->setCurrentWidget(mainMobileSyncWidget);
        mainMobileSyncWidget->initializeView();
        optionsMenuBars->setCurrentWidget(mobileSyncOptionsMenuBar);
    });
    connect(mainMenuConfigure, &QAction::triggered, this, &xApplication::configure);
    connect(mainMenuExit, &QAction::triggered, this, &xApplication::close);
    connect(mainMenuAboutQt, &QAction::triggered, [=]() { QMessageBox::aboutQt(this, "About Qt"); });

    // Create stacked options menu bars.
    optionsMenuBars = new QStackedWidget(this);
    // Hack: Add one pixel on the right side to avoid issues with dual (or more) head setup.
    optionsMenuBars->setContentsMargins(0, 0, 1, 0);
    menuBar()->setCornerWidget(optionsMenuBars, Qt::TopRightCorner);
    musicOptionsMenuBar = new QMenuBar(optionsMenuBars);
    movieOptionsMenuBar = new QMenuBar(optionsMenuBars);
    streamingOptionsMenuBar = new QMenuBar(optionsMenuBars);
    mobileSyncOptionsMenuBar = new QMenuBar(optionsMenuBars);
    optionsMenuBars->addWidget(musicOptionsMenuBar);
    optionsMenuBars->addWidget(movieOptionsMenuBar);
    optionsMenuBars->addWidget(streamingOptionsMenuBar);
    optionsMenuBars->addWidget(mobileSyncOptionsMenuBar);
    // Create options menus.
    createMusicOptionsMenus();
    createMovieOptionsMenus();
#ifdef USE_STREAMING
    createStreamingOptionsMenus();
#endif
    createMobileSyncOptionsMenus();
}

void xApplication::createMusicOptionsMenus() {
    // Options Menu for Music View
    musicOptionsMenuBar->setLayoutDirection(Qt::RightToLeft);
    auto musicOptionsMenu = musicOptionsMenuBar->addMenu(QIcon(":images/xplay-options.svg") ,"");
    auto musicOptionsRescanMusicLibrary = new QAction("Rescan Library", this);
    auto musicOptionsCheckDatabase = new QAction("Check Database", this);
    // Get current music library configuration.
    auto useMusicLibraryBluOS = xPlayerConfiguration::configuration()->useMusicLibraryBluOS();
    // Create actions for music view submenu.
    auto musicOptionsUseBluOSPlayer = new QAction("Use BluOS Player", this);
    musicOptionsUseBluOSPlayer->setCheckable(true);
    //musicOptionsUseBluOSPlayer->setShortcut(QKeySequence("Ctrl+Alt+B"));
    musicOptionsUseBluOSPlayer->setChecked(useMusicLibraryBluOS);
    auto musicOptionsReIndexBluOSPlayer = new QAction("ReIndex BluOS Player", this);
    musicOptionsReIndexBluOSPlayer->setEnabled(useMusicLibraryBluOS);
    auto musicOptionsSelectors = new QAction("Selectors", this);
    musicOptionsSelectors->setCheckable(true);
    //musicOptionsSelectors->setShortcut(QKeySequence("Ctrl+Alt+S"));
    musicOptionsSelectors->setChecked(xPlayerConfiguration::configuration()->getMusicViewSelectors());
    auto musicOptionsFilters = new QAction("Filters", this);
    musicOptionsFilters->setCheckable(true);
    //musicOptionsFilters->setShortcut(QKeySequence("Ctrl+Alt+F"));
    musicOptionsFilters->setChecked(xPlayerConfiguration::configuration()->getMusicViewFilters());
    musicOptionsVisualization = new QAction("Visualization", this);
    musicOptionsVisualization->setCheckable(true);
    //musicOptionsVisualization->setShortcut(QKeySequence("Ctrl+Alt+V"));
    musicOptionsVisualization->setChecked(xPlayerConfiguration::configuration()->getMusicViewVisualization());
    musicOptionsVisualization->setDisabled(xPlayerConfiguration::configuration()->useMusicLibraryBluOS());
    // Action group for visualization mode
    auto musicOptionsVisualizationMode = new QActionGroup(this);
    musicOptionsVisualizationMode->setExclusive(true);
    auto musicOptionsVisualizationSmall = new QAction("Small Window", musicOptionsVisualizationMode);
    musicOptionsVisualizationSmall->setCheckable(true);
    musicOptionsVisualizationSmall->setChecked(true);
    auto musicOptionsVisualizationCentral = new QAction("Central Window", musicOptionsVisualizationMode);
    musicOptionsVisualizationCentral->setCheckable(true);

    // Create music options menu.
    musicOptionsMenu->addAction(musicOptionsRescanMusicLibrary);
    musicOptionsMenu->addAction(musicOptionsCheckDatabase);
    musicOptionsMenu->addSeparator();
    musicOptionsMenu->addAction(musicOptionsUseBluOSPlayer);
    musicOptionsMenu->addAction(musicOptionsReIndexBluOSPlayer);
    musicOptionsMenu->addSeparator();
    musicOptionsMenu->addAction(musicOptionsSelectors);
    musicOptionsMenu->addAction(musicOptionsFilters);
    musicOptionsMenu->addAction(musicOptionsVisualization);
    auto musicOptionsVisualizationMenu = musicOptionsMenu->addMenu("Visualization Mode");
    musicOptionsVisualizationMenu->addAction(musicOptionsVisualizationSmall);
    musicOptionsVisualizationMenu->addAction(musicOptionsVisualizationCentral);
    // Select the proper visualization mode.
    if (xPlayerConfiguration::configuration()->getMusicViewVisualizationMode() == 0) {
        musicOptionsVisualizationSmall->setChecked(true);
    } else {
        musicOptionsVisualizationCentral->setChecked(true);
    }
    musicOptionsVisualizationMenu->setDisabled(xPlayerConfiguration::configuration()->getMusicViewVisualization());

    // Create connections.
    connect(musicOptionsRescanMusicLibrary, &QAction::triggered, [=]() {
        setMusicLibrary(true);
    });
    connect(musicOptionsCheckDatabase, &QAction::triggered, this, &xApplication::checkMusicDatabase);
    connect(musicOptionsUseBluOSPlayer, &QAction::triggered, mainMusicWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->useMusicLibraryBluOS(checked);
        // Disable visualization menu entry if BluOS is enabled.
        musicOptionsVisualization->setEnabled(!checked);
        // Enable reindex menu entry if BluOS is enabled.
        musicOptionsReIndexBluOSPlayer->setEnabled(checked);
    });
    connect(musicOptionsReIndexBluOSPlayer, &QAction::triggered, this, &xApplication::reIndexMusicLibraryBluOS);
    connect(musicOptionsSelectors, &QAction::triggered, mainMusicWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->setMusicViewSelectors(checked);
    });
    connect(musicOptionsFilters, &QAction::triggered, mainMusicWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->setMusicViewFilters(checked);
    });
    // Connect visualization signals.
    connect(musicOptionsVisualization, &QAction::triggered, mainMusicWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->setMusicViewVisualization(checked);
        // Mode can only be changed when visualization disabled.
        musicOptionsVisualizationMenu->setEnabled(!checked);
    });
    connect(musicOptionsVisualizationMode, &QActionGroup::triggered, [=](QAction* action) {
        if (action == musicOptionsVisualizationSmall) {
            xPlayerConfiguration::configuration()->setMusicViewVisualizationMode(0);
        } else {
            xPlayerConfiguration::configuration()->setMusicViewVisualizationMode(1);
        }
    });
    // Toggle the visualization view.
    connect(mainMusicWidget, &xMainMusicWidget::visualizationToggle, [=]() {
        auto toggleChecked = !musicOptionsVisualization->isChecked();
        musicOptionsVisualization->setChecked(toggleChecked);
        musicOptionsVisualizationMenu->setEnabled(!toggleChecked);
        xPlayerConfiguration::configuration()->setMusicViewVisualization(toggleChecked);
    });
    // Disable music visualization if ESC is pressed.
    connect(mainMusicWidget, &xMainMusicWidget::visualizationExiting, [=]() {
        musicOptionsVisualization->setChecked(false);
        musicOptionsVisualizationMenu->setEnabled(true);
        xPlayerConfiguration::configuration()->setMusicViewVisualization(false);
    });
    // Disable the visualization view in case of an error.
    connect(mainMusicWidget, &xMainMusicWidget::visualizationError, [=]() {
        musicOptionsVisualization->setChecked(false);
        musicOptionsVisualization->setEnabled(false);
        musicOptionsVisualizationMenu->setEnabled(false);
        xPlayerConfiguration::configuration()->setMusicViewVisualization(false);
    });
    // Connect menu entries to enable/disable them.
    connect(mainMobileSyncWidget, &xMainMobileSyncWidget::enableMusicLibraryScanning,
            musicOptionsRescanMusicLibrary, &QAction::setEnabled);
}

void xApplication::createMovieOptionsMenus() {
    // Options Menu for Movie View
    movieOptionsMenuBar->setLayoutDirection(Qt::RightToLeft);
    auto movieOptionsMenu = movieOptionsMenuBar->addMenu(QIcon(":images/xplay-options.svg") ,"");
    auto movieOptionsRescanLibrary = new QAction("Rescan Library", this);
    auto movieOptionsCheckDatabase = new QAction("Check Database", this);
    auto movieOptionsFilters = new QAction("Filters", this);
    movieOptionsFilters->setCheckable(true);
    //movieOptionsFilters->setShortcut(QKeySequence("Ctrl+Alt+M"));
    movieOptionsFilters->setChecked(xPlayerConfiguration::configuration()->getMovieViewFilters());

    // Create movie options menu.
    movieOptionsMenu->addAction(movieOptionsRescanLibrary);
    movieOptionsMenu->addAction(movieOptionsCheckDatabase);
    movieOptionsMenu->addSeparator();
    movieOptionsMenu->addAction(movieOptionsFilters);

    // Create connections.
    connect(movieOptionsRescanLibrary, &QAction::triggered, this, &xApplication::setMovieLibraryTagsAndDirectories);
    connect(movieOptionsCheckDatabase, &QAction::triggered, this, &xApplication::checkMovieDatabase);
    connect(movieOptionsFilters, &QAction::triggered, mainMovieWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->setMovieViewFilters(checked);
    });
}

void xApplication::createStreamingOptionsMenus() {
#ifdef USE_STREAMING
    // Options Menu for Streaming View
    streamingOptionsMenuBar->setLayoutDirection(Qt::RightToLeft);
    auto streamingOptionsMenu = streamingOptionsMenuBar->addMenu(QIcon(":images/xplay-options.svg") ,"");
    auto streamingOptionsSidebar = new QAction("Sidebar", this);
    streamingOptionsSidebar->setCheckable(true);
    //streamingOptionsSidebar->setShortcut(QKeySequence("Ctrl+Alt+B"));
    streamingOptionsSidebar->setChecked(xPlayerConfiguration::configuration()->getStreamingViewSidebar());
    auto streamingOptionsNavigation = new QAction("Navigation", this);
    streamingOptionsNavigation->setCheckable(true);
    //streamingOptionsNavigation->setShortcut(QKeySequence("Ctrl+Alt+N"));
    streamingOptionsNavigation->setChecked(xPlayerConfiguration::configuration()->getStreamingViewNavigation());

    // Create movie options menu.
    streamingOptionsMenu->addAction(streamingOptionsSidebar);
    streamingOptionsMenu->addAction(streamingOptionsNavigation);

    // Create connections.
    connect(streamingOptionsSidebar, &QAction::triggered, mainMusicWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->setStreamingViewSidebar(checked);
    });
    connect(streamingOptionsNavigation, &QAction::triggered, mainMusicWidget, [=](bool checked) {
        xPlayerConfiguration::configuration()->setStreamingViewNavigation(checked);
    });
#endif
}

void xApplication::createMobileSyncOptionsMenus() {
    // Currently no options menu for mobile sync view.
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
