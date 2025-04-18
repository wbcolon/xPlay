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
#include "xMainMusicWidget.h"
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"
#include "xPlayerMusicSearchWidget.h"
#include "xPlayerMusicWidget.h"
#include "xPlayerDatabase.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include "xPlayerPlaylistDialog.h"
#include "xPlayerTagsDialog.h"

#include <QGroupBox>
#include <QListWidget>
#include <QListView>
#include <QSplitter>
#include <QApplication>
#include <QRandomGenerator>
#include <QDateTime>
#include <QCheckBox>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <random>
#include <iterator>


// Function addListWidgetGroupBox has to be defined before the constructor due to the auto return.
auto xMainMusicWidget::addListWidgetGroupBox(const QString& boxLabel, bool displayTime, QWidget* parent) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, parent);
    groupBox->setFlat(xPlayer::UseFlatGroupBox);
    auto list = new xPlayerListWidget(groupBox, displayTime);
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    auto boxLayout = new QVBoxLayout(groupBox);
    boxLayout->addWidget(list);
    return std::make_pair(groupBox,list);
}

auto xMainMusicWidget::addLineEditGroupBox(const QString& boxLabel, QWidget* parent) {
    // Create a QGroupBox with the given label and embed a QLineEdit.
    auto groupBox = new QGroupBox(boxLabel, parent);
    groupBox->setFlat(xPlayer::UseFlatGroupBox);
    auto lineEdit = new QLineEdit(groupBox);
    auto boxLayout = new QVBoxLayout(groupBox);
    boxLayout->addWidget(lineEdit);
    return std::make_pair(groupBox, lineEdit);
}


xMainMusicWidget::xMainMusicWidget(xMusicPlayer* player, xMusicLibrary* library, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicVisualizationEnabled(false),
        musicVisualizationMode(0),
        musicVisualizationFullWindow(false),
        musicPlayer(player),
        musicLibrary(library),
        playedTrack(0),
        useDatabaseMusicOverlay(true),
        databaseCutOff(0),
        currentArtist(),
        currentAlbum(),
        currentTrackName(),
        currentArtistSelector(),
        useSortingLatest(false) {

    auto mainWidgetSplitter = new QSplitter(this);
    mainWidgetSplitter->setOrientation(Qt::Horizontal);
    // Widget for music player and stacked widget.
    auto musicPlayerStackedWidget = new QWidget(mainWidgetSplitter);
    auto musicPlayerStackedLayout = new QVBoxLayout(musicPlayerStackedWidget);
    // Stacked widget for lists and artist info.
    musicStacked = new QStackedWidget(musicPlayerStackedWidget);
    // Create and group boxes with embedded list widgets.
    musicListView = new QWidget(musicStacked);
    // Splitter for artist, album and track lists.
    auto artistAlbumTrackSplitter = new QSplitter(musicListView);
    artistAlbumTrackSplitter->setOrientation(Qt::Horizontal);
    // Widgets with list and filter.
    auto artistWidget = new QWidget(artistAlbumTrackSplitter);
    auto artistLayout = new QVBoxLayout(artistWidget);
    auto albumWidget = new QWidget(artistAlbumTrackSplitter);
    auto albumLayout = new QVBoxLayout(albumWidget);
    auto trackWidget = new QWidget(artistAlbumTrackSplitter);
    auto trackLayout = new QVBoxLayout(trackWidget);
    // Create boxes.
    auto [artistBox_, artistList_] = addListWidgetGroupBox(tr("Artists"), false, artistWidget);
    auto [albumBox, albumList_] = addListWidgetGroupBox(tr("Albums"), false, albumWidget);
    auto [trackBox_, trackList_] = addListWidgetGroupBox(tr("Tracks"), true, trackWidget);
    // Sort entries in artist/album/track
    artistList = artistList_;
    artistList->enableSorting(false);
    artistList->setMinimumWidth(xPlayer::ArtistListMinimumWidth);
    artistBox = artistBox_;
    albumList = albumList_;
    albumList->enableSorting(false);
    albumList->setMinimumWidth(xPlayer::AlbumListMinimumWidth);
    trackList = trackList_;
    trackList->enableSorting(true);
    trackList->setMinimumWidth(xPlayer::TracksListMinimumWidth);
    trackBox = trackBox_;
    // Local filters.
    auto [artistFilterBox_, artistFilterLineEdit_] = addLineEditGroupBox(tr("Filter Artists"), artistWidget);
    auto [albumFilterBox_, albumFilterLineEdit_] = addLineEditGroupBox(tr("Filter Albums"), albumWidget);
    auto [trackFilterBox_, trackFilterLineEdit_] = addLineEditGroupBox(tr("Filter Tracks"), trackWidget);
    artistFilterBox = artistFilterBox_;
    artistFilterLineEdit = artistFilterLineEdit_;
    albumFilterBox = albumFilterBox_;
    albumFilterLineEdit = albumFilterLineEdit_;
    trackFilterBox = trackFilterBox_;
    trackFilterLineEdit = trackFilterLineEdit_;
    // Add box and filter to widget
    artistLayout->addWidget(artistBox);
    artistLayout->addWidget(artistFilterBox);
    albumLayout->addWidget(albumBox);
    albumLayout->addWidget(albumFilterBox);
    trackLayout->addWidget(trackBox);
    trackLayout->addWidget(trackFilterBox);
    // Add widgets to splitter.
    artistAlbumTrackSplitter->addWidget(artistWidget);
    artistAlbumTrackSplitter->addWidget(albumWidget);
    artistAlbumTrackSplitter->addWidget(trackWidget);
    artistAlbumTrackSplitter->setCollapsible(0, false);
    artistAlbumTrackSplitter->setStretchFactor(0, 1);
    artistAlbumTrackSplitter->setCollapsible(1, false);
    artistAlbumTrackSplitter->setStretchFactor(1, 1);
    artistAlbumTrackSplitter->setCollapsible(2, false);
    artistAlbumTrackSplitter->setStretchFactor(2, 1);
    // Selector Tabs.
    selectorTabs = new QTabWidget(musicListView);
    selectorTabs->setStyleSheet("QTabWidget::pane { border: none; }");
    // Artist, album selector and Search.
    artistSelectorList = new xPlayerMusicArtistSelectorWidget(selectorTabs);
    albumSelectorList = new xPlayerMusicAlbumSelectorWidget(selectorTabs);
    searchSelector = new xPlayerMusicSearchWidget(selectorTabs);
    selectorTabs->addTab(artistSelectorList, tr("Artist Selector"));
    selectorTabs->addTab(albumSelectorList, tr("Album Selector"));
    selectorTabs->addTab(searchSelector, tr("Search"));
    selectorTabs->setFixedHeight(artistSelectorList->height()+selectorTabs->tabBar()->sizeHint().height());
    // Setup layout for list view.
    auto musicListViewLayout = new xPlayerLayout(musicListView);
    musicListViewLayout->setContentsMargins(0, 0, 0, 0);
    musicListViewLayout->setSpacing(xPlayerLayout::SmallSpace);
    musicListViewLayout->addWidget(artistAlbumTrackSplitter, 0, 0, 10, 12);
    musicListViewLayout->addWidget(selectorTabs, 10, 0, 1, 12);
    // Player widget.
    playerWidget = new xPlayerMusicWidget(musicPlayer, musicPlayerStackedWidget);
    musicInfoView = new xPlayerArtistInfo(musicStacked);
    musicStacked->addWidget(musicListView);
    musicStacked->addWidget(musicInfoView);
    musicStacked->setCurrentWidget(musicListView);
    musicPlayerStackedLayout->addWidget(playerWidget, 1);
    musicPlayerStackedLayout->addWidget(musicStacked, 5);
    // Queue list.
    queueBox = new QGroupBox(tr("Queue"), mainWidgetSplitter);
    queueBox->setFlat(xPlayer::UseFlatGroupBox);
    queueBoxLayout = new xPlayerLayout(queueBox);
    queueList = new xPlayerListWidget(queueBox, true);
    queueList->setContextMenuPolicy(Qt::CustomContextMenu);
    queueList->setDragDropMode(QTreeWidget::InternalMove);
    queueList->setSelectionMode(QTreeWidget::SingleSelection);
    queueList->setMinimumWidth(xPlayer::QueueListMinimumWidth);
    auto queueShuffleCheck = new QCheckBox(tr("Shuffle Mode"), queueBox);
    // Tags menu.
    auto queueTagsButton = new QPushButton(tr("Tags"), queueBox);
    // Playlist menu.
    auto queuePlaylistButton = new QPushButton(tr("Playlist"), queueBox);
    queueProgress = new QProgressBar(queueBox);
    queueProgress->setVisible(false);
    queuePlaylistButton->setFlat(false);
    queueBoxLayout->addWidget(queueList, 0, 0, 7, 4);
    queueBoxLayout->addRowSpacer(7, xPlayerLayout::MediumSpace);
    queueBoxLayout->addWidget(queueProgress, 8, 0, 1, 4);
    queueBoxLayout->addWidget(queueShuffleCheck, 9, 0);
    queueBoxLayout->addWidget(queueTagsButton, 9, 2);
    queueBoxLayout->addWidget(queuePlaylistButton, 9, 3);
    // Does the music player support visualization.
    if (musicPlayer->supportsVisualization()) {
        musicVisualizationWidget = new xPlayerVisualizationWidget(queueBox);
        // Connect music visualization view
        connect(musicPlayer, &xMusicPlayer::visualizationStereo,
                musicVisualizationWidget, &xPlayerVisualizationWidget::visualizationStereo);
        connect(musicPlayer, &xMusicPlayer::currentTrackPlayed, this, &xMainMusicWidget::updateWindowTitle);
        connect(playerWidget, &xPlayerMusicWidget::mouseDoubleClicked, this, &xMainMusicWidget::visualizationToggle);
        connect(musicVisualizationWidget, &xPlayerVisualizationWidget::visualizationFullWindow,
                this, &xMainMusicWidget::updateMusicViewVisualizationFullWindow);
        connect(musicVisualizationWidget, &xPlayerVisualizationWidget::visualizationExiting,
                this, &xMainMusicWidget::visualizationExiting);
        connect(musicVisualizationWidget, &xPlayerVisualizationWidget::visualizationError,
                this, &xMainMusicWidget::visualizationError);
        // Add it initially to the stacked widget.
        musicStacked->addWidget(musicVisualizationWidget);
    } else {
        // No visualization.
        musicVisualizationWidget = nullptr;
    }
    queueBoxLayout->setColumnStretch(1, 3);
    // Setup splitter, queue should stretch a little more than the rest.
    mainWidgetSplitter->addWidget(musicPlayerStackedWidget);
    mainWidgetSplitter->addWidget(queueBox);
    mainWidgetSplitter->setCollapsible(0, false);
    mainWidgetSplitter->setStretchFactor(0, 7);
    mainWidgetSplitter->setCollapsible(1, false);
    mainWidgetSplitter->setStretchFactor(1, 8);
    // Setup layout for main widget.
    auto mainWidgetLayout = new xPlayerLayout(this);
    mainWidgetLayout->setSpacing(xPlayerLayout::SmallSpace);
    mainWidgetLayout->addWidget(mainWidgetSplitter);
    // Connect artist, album, track and selector widgets
    connect(artistList, &xPlayerListWidget::currentListIndexChanged, this, &xMainMusicWidget::selectArtist);
    connect(artistList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMusicWidget::queueArtistItem);
    connect(albumList, &xPlayerListWidget::currentListIndexChanged, this, &xMainMusicWidget::selectAlbum);
    connect(albumList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMusicWidget::queueAlbumItem);
    connect(trackList, &xPlayerListWidget::listItemDoubleClicked, [=](xPlayerListWidgetItem* trackItem) {
        // Double click adds current and remaining to queue.
        queueTrackItem(trackItem, true);
    });
    connect(trackList, &xPlayerListWidget::totalTime, this, &xMainMusicWidget::updateTracksTotalTime);
    connect(artistSelectorList, &xPlayerMusicArtistSelectorWidget::selector, this, &xMainMusicWidget::selectArtistSelector);
    connect(artistSelectorList, &xPlayerMusicArtistSelectorWidget::selectorCtrlHovered, this, &xMainMusicWidget::jumpArtistSelector);
    connect(artistSelectorList, &xPlayerMusicArtistSelectorWidget::selectorDoubleClicked, this, &xMainMusicWidget::queueArtistSelector);
    connect(artistSelectorList, &xPlayerMusicArtistSelectorWidget::sortingLatest, this, &xMainMusicWidget::selectSortingLatest);
    connect(albumSelectorList, &xPlayerMusicAlbumSelectorWidget::updatedSelectors, this,
            &xMainMusicWidget::selectAlbumSelectors);
    connect(albumSelectorList, &xPlayerMusicAlbumSelectorWidget::updatedDatabaseSelectors, this,
            &xMainMusicWidget::selectAlbumDatabaseSelectors);
    connect(albumSelectorList, &xPlayerMusicAlbumSelectorWidget::clearDatabaseSelectors,
            this, &xMainMusicWidget::clearAlbumDatabaseSelectors);
    connect(albumSelectorList, &xPlayerMusicAlbumSelectorWidget::clearAllSelectors, this,
            &xMainMusicWidget::clearAllAlbumSelectors);
    connect(searchSelector, &xPlayerMusicSearchWidget::updateFilter, this, &xMainMusicWidget::updateSearchSelectorFilter);
    connect(searchSelector, &xPlayerMusicSearchWidget::clearFilter, this, &xMainMusicWidget::clearSearchSelectorFilter);
    // Connect local filters.
    connect(artistFilterLineEdit, &QLineEdit::textChanged, artistList, &xPlayerListWidget::updateFilter);
    connect(albumFilterLineEdit, &QLineEdit::textChanged, albumList, &xPlayerListWidget::updateFilter);
    connect(trackFilterLineEdit, &QLineEdit::textChanged, trackList, &xPlayerListWidget::updateFilter);
    // Connect artist info view
    connect(musicInfoView, &xPlayerArtistInfo::close, this, [=]() {
        musicStacked->setCurrentWidget(musicListView);
        if (musicVisualizationWidget) {
            // Reset reduced framerate mode for visualization.
            musicVisualizationWidget->setReducedFrameRate(xPlayer::VisualizationNoDrop);
            updateVisualizationView(musicPlayer->isPlaying());
        }
    });
    // Connect main widget to music player
    connect(this, &xMainMusicWidget::queueTracks, musicPlayer, &xMusicPlayer::queueTracks);
    connect(this, &xMainMusicWidget::finishedQueueTracks, musicPlayer, &xMusicPlayer::finishedQueueTracks);
    connect(this, &xMainMusicWidget::dequeueTrack, musicPlayer, &xMusicPlayer::dequeueTrack);
    // Connect player widget to main widget
    connect(playerWidget, &xPlayerMusicWidget::clearQueue, this, &xMainMusicWidget::clearQueue);
    // Connect queue to main widget
    connect(queueList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMusicWidget::currentQueueTrackDoubleClicked);
    connect(queueList, &xPlayerListWidget::listItemClicked, this, &xMainMusicWidget::currentQueueTrackCtrlClicked);
    connect(queueList, &xPlayerListWidget::totalTime, this, &xMainMusicWidget::updateQueueTotalTime);
    connect(queueList, &xPlayerListWidget::dragDrop, musicPlayer, &xMusicPlayer::moveQueueTracks);
    // Connect shuffle mode.
    connect(queueShuffleCheck, &QCheckBox::clicked, musicPlayer, &xMusicPlayer::setShuffleMode);
    connect(queueShuffleCheck, &QCheckBox::clicked, queueList, &QListWidget::setDisabled);
    // Connect queue tags dialog.
    connect(queueTagsButton, &QPushButton::pressed, this, &xMainMusicWidget::showTagsDialog);
    // Connect queue playlist dialog.
    connect(queuePlaylistButton, &QPushButton::pressed, this, &xMainMusicWidget::showPlaylistDialog);
    // Right click.
    connect(artistList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentArtistRightClicked);
    connect(albumList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentAlbumRightClicked);
    connect(trackList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentTrackRightClicked);
    connect(queueList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentQueueTrackRightClicked);
    // Connect music player to main widget for queue update.
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xMainMusicWidget::currentState);
    // Connect update for current track. Update queue, database and database overlay.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xMainMusicWidget::currentTrack);
    connect(musicPlayer, &xMusicPlayer::updatePlayedTrack, this, &xMainMusicWidget::updatePlayedTrack);
    // Connect update of playlist.
    connect(musicPlayer, &xMusicPlayer::playlist, this, &xMainMusicWidget::playlist);
    // Connect to shuffe mode.
    connect(musicPlayer, &xMusicPlayer::allowShuffleMode, [=](bool enable) {
        if (!enable) {
            queueShuffleCheck->setChecked(false);
            musicPlayer->setShuffleMode(false);
            queueList->setEnabled(true);
        }
        queueShuffleCheck->setEnabled(enable);
    });
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMusicOverlay,
            this, &xMainMusicWidget::updatedDatabaseMusicOverlay);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryAlbumSelectors,
            this, &xMainMusicWidget::updatedMusicLibraryAlbumSelectors);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicViewSelectors,
            this, &xMainMusicWidget::updatedMusicViewSelectors);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicViewFilters,
            this, &xMainMusicWidget::updatedMusicViewFilters);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicViewVisualization,
            this, &xMainMusicWidget::updatedMusicViewVisualization);
    // No need to connect to updatedMusicViewVisualizationMode since mode can only be changed if visualization is off.
}

void xMainMusicWidget::initializeView() {
    emit showWindowTitle(QApplication::applicationName());
    emit showMenuBar(true);
}

void xMainMusicWidget::clear() {
    // Clear queue.
    musicPlayer->clearQueue();
    playerWidget->clear();
    clearQueue();
    // Clear artist (including selector), album and track lists.
    artistSelectorList->clear();
    scannedArtists({});
    // Clear album and search selector widgets.
    albumSelectorList->clear();
    searchSelector->clear();
    // Disable selector tab on clear. Library needs to be scanned again.
    selectorTabs->setEnabled(false);
    selectorTabs->setCurrentIndex(0);
    // Clear Sorting Latest.
    useSortingLatest = false;
}

void xMainMusicWidget::scanningProgress(int percent) {
    if ((percent > 0) && (percent < 100)) {
        artistBox->setTitle(tr("Artists")+QString(" - scanning %1%").arg(percent));
    } else {
        artistBox->setTitle(tr("Artists"));
    }
}

void xMainMusicWidget::scannedArtists(const std::vector<xMusicLibraryArtistEntry*>& artists) {
    std::set<QString> selectors;
    // Save unfiltered list.
    unfilteredArtists = artists;
    // Use unfiltered list for selectors update
    for (auto artist : artists) {
        // Convert selectors to lower-case.
        selectors.insert(artist->getArtistName().left(1).toLower());
    }
    // Update the selector based upon the added artists.
    artistSelectorList->updateSelectors(selectors);
    // Update the artists.
    updateScannedArtists(artists);
}

void xMainMusicWidget::updateScannedArtists(const std::vector<xMusicLibraryArtistEntry*>& artists) {
    // Clear artist, album and track lists
    artistList->clearItems();
    albumList->clearItems();
    clearTrackList();
    // Not very efficient to use a filtered and the unfiltered list, but easier to read.
    filteredArtists = filterArtists(artists);
    for (auto artist : filteredArtists) {
        artistList->addListItem(artist);
    }
    // Update database overlay for artists.
    updatePlayedArtists();
    // Enable the selector tab if any artists are in the list
    if (!filteredArtists.empty()) {
        selectorTabs->setEnabled(true);
    }
}

void xMainMusicWidget::scannedAlbums(const std::vector<xMusicLibraryAlbumEntry*>& albums) {
    auto sortedAlbums = albums;
    sortEntries(sortedAlbums);
    // Clear album and track lists
    albumList->clearItems();
    clearTrackList();
    for (auto album : sortedAlbums) {
        albumList->addListItem(album);
    }
    // Update database overlay for albums.
    updatePlayedAlbums();
}

void xMainMusicWidget::scannedTracks(const std::vector<xMusicLibraryTrackEntry*>& tracks) {
    // Clear only track list and stop potential running update thread.
    clearTrackList();
    for (auto track : tracks) {
        trackList->addListItem(track);
    }
    // Update database overlay for tracks. Run in separate thread.
    updatePlayedTracks();
    // Update track times.
    trackList->updateItems();
}

void xMainMusicWidget::scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,
                                             std::vector<xMusicLibraryTrackEntry*>>>& albumTracks) {
    for (const auto& albumTrack : albumTracks) {
        queueList->addListItems(albumTrack.second, QString("%1 - %2").arg(artist, albumTrack.first));
        emit queueTracks(artist, albumTrack.first, albumTrack.second);
    }
    emit finishedQueueTracks(true);
    // Update items.
    queueList->updateItems();
    // Restore the waiting cursor.
    QApplication::restoreOverrideCursor();
}

void xMainMusicWidget::scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString,
                                                        std::vector<xMusicLibraryTrackEntry*>>>>>& listTracks) {
    // Compute the number of files to be inserted.
    int maxListTracks = 0;
    for (const auto& listElem : listTracks) {
        for (const auto& albumTrack : listElem.second) {
            maxListTracks += static_cast<int>(albumTrack.second.size());
        }
    }
    qDebug() << "scannedListArtistsAllAlbumTracks: maxFiles: " << maxListTracks;

    if (maxListTracks > xPlayer::QueueCriticalNumberEntries) {
        QApplication::restoreOverrideCursor();
        auto result = QMessageBox::warning(this, tr("Queue"),
                                           tr("Do you want to add %1 tracks to the queue?\n"
                                              "This process may take a little longer and it cannot be aborted.").
                                                   arg(maxListTracks),  QMessageBox::Cancel | QMessageBox::Ok,
                                           QMessageBox::Ok);
        if (result == QMessageBox::Cancel) {
            return;
        }
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    int currentListTracks = 0;
    // Disable updates for other lists while queueing.
    artistList->setUpdatesEnabled(false);
    albumList->setUpdatesEnabled(false);
    trackList->setUpdatesEnabled(false);
    queueList->setUpdatesEnabled(false);
    // Setup and show the progress bar.
    queueProgress->setRange(0, maxListTracks);
    queueProgress->setVisible(true);
    for (const auto& listElem : listTracks) {
        // Update progress bar including format.
        queueProgress->setFormat(QString("%1 - %p%").arg(listElem.first));
        queueProgress->setValue(currentListTracks);
        for (const auto& albumTrack : listElem.second) {
            qDebug() << "scannedListArtistsAllAlbumTracks: " << listElem.first << "," << albumTrack.first;
            // Update queue list UI.
            queueList->addListItems(albumTrack.second, QString("%1 - %2").arg(listElem.first, albumTrack.first));
            // Queue items.
            emit queueTracks(listElem.first, albumTrack.first, albumTrack.second);
            // Update progress bar.
            currentListTracks += static_cast<int>(albumTrack.second.size());
            queueProgress->setValue(currentListTracks);
        }
    }
    // Restore updates on the lists.
    artistList->setUpdatesEnabled(true);
    albumList->setUpdatesEnabled(true);
    trackList->setUpdatesEnabled(true);
    queueList->setUpdatesEnabled(true);
    // Update UI and finish queuing.
    queueList->updateItems();
    emit finishedQueueTracks(false);
    // Hide the progress bar again.
    queueProgress->setVisible(false);
    // Restore the waiting cursor.
    QApplication::restoreOverrideCursor();
}

void xMainMusicWidget::scannedAllAlbumsForListArtistsWorker(
        const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>>>& listTracks,
        const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>>>::const_iterator& listTracksIterator,
        int currentFiles, int maxFiles) {

    if (listTracksIterator != listTracks.end()) {
        // Update progress bar including format.
        queueProgress->setFormat(QString("%1 - %p%").arg(listTracksIterator->first));
        queueProgress->setValue(currentFiles);
        for (const auto& albumTrack : listTracksIterator->second) {
            qDebug() << "scannedListArtistsAllAlbumTracks: " << listTracksIterator->first << "," << albumTrack.first;
            // Update queue list UI.
            queueList->addListItems(albumTrack.second,
                                    QString("%1 - %2").arg(listTracksIterator->first, albumTrack.first));
            // Queue items.
            emit queueTracks(listTracksIterator->first, albumTrack.first, albumTrack.second);
            // Update progress bar.
            currentFiles += static_cast<int>(albumTrack.second.size());
            queueProgress->setValue(currentFiles);
        }
        QTimer::singleShot(50, [=]() {
            auto nextTracksInterator = listTracksIterator;
            ++nextTracksInterator;
            emit scanAllAlbumsForListArtistsIterate(listTracks, nextTracksInterator, currentFiles, maxFiles);
        });
    } else {
        // Restore updates on the lists.
        artistList->setUpdatesEnabled(true);
        albumList->setUpdatesEnabled(true);
        trackList->setUpdatesEnabled(true);
        queueList->setUpdatesEnabled(true);
        // Update UI and finish queuing.
        queueList->updateItems();
        emit finishedQueueTracks(false);
        // Hide the progress bar again.
        queueProgress->setVisible(false);
        // Restore the waiting cursor.
        QApplication::restoreOverrideCursor();
    }
}

std::vector<xMusicLibraryArtistEntry*> xMainMusicWidget::filterArtists(const std::vector<xMusicLibraryArtistEntry*>& artists) {
    // Check if a selector is selected. We sort the list if necessary.
    std::vector<xMusicLibraryArtistEntry*> filtered;
    if (!currentArtistSelector.isEmpty()) {
        // Do not filter if we have selector "none" currentArtistSelector.
        if (currentArtistSelector.compare(tr("none"), Qt::CaseInsensitive) == 0) {
            filtered = artists;
            sortEntries(filtered);
            return filtered;
        }
        // Do not filter if we have selector "random" currentArtistSelector. Just randomize list.
        if (currentArtistSelector.compare(tr("random"), Qt::CaseInsensitive) == 0) {
            filtered = artists;
            // Not a very efficient way to shuffle the entries...
            std::vector<xMusicLibraryArtistEntry*> shuffleFiltered(artists.begin(), artists.end());
            std::shuffle(shuffleFiltered.begin(), shuffleFiltered.end(), std::mt19937(std::random_device()()));
            std::copy(shuffleFiltered.begin(), shuffleFiltered.end(), filtered.begin());
            return filtered;
        }
        // Go through list of artists and only add the ones to the filtered
        // list that start with the selector character.
        for (auto artist : artists) {
            // Convert to lower-case before checking against artist selector.
            if (artist->getArtistName().toLower().startsWith(currentArtistSelector)) {
                filtered.push_back(artist);
            }
        }
        sortEntries(filtered);
        return filtered;
    }
    // No artist selector enabled. Return sorted list of artists.
    filtered = artists;
    sortEntries(filtered);
    return filtered;
}

void xMainMusicWidget::sortEntries(std::vector<xMusicLibraryAlbumEntry*>& albums) const {
    if (useSortingLatest) {
        std::sort(albums.begin(), albums.end(), [](xMusicLibraryAlbumEntry* a, xMusicLibraryAlbumEntry* b) {
            return a->getLastWritten() > b->getLastWritten();
        });
    } else {
        std::sort(albums.begin(), albums.end(), [](xMusicLibraryAlbumEntry* a, xMusicLibraryAlbumEntry* b) {
            return a->getAlbumName().toLower() < b->getAlbumName().toLower();
        });
    }
}

void xMainMusicWidget::sortEntries(std::vector<xMusicLibraryArtistEntry*>& artists) const {
    if (useSortingLatest) {
        std::sort(artists.begin(), artists.end(), [](xMusicLibraryArtistEntry* a, xMusicLibraryArtistEntry* b) {
            return a->getLastWritten() > b->getLastWritten();
        });
    } else {
        std::sort(artists.begin(), artists.end(), [](xMusicLibraryArtistEntry* a, xMusicLibraryArtistEntry* b) {
            return a->getArtistName().toLower() < b->getArtistName().toLower();
        });
    }
}

bool xMainMusicWidget::sortListItems(xPlayerListWidgetItem* a, xPlayerListWidgetItem* b) const {
    if (useSortingLatest) {
        if (a->artistEntry() && b->artistEntry()) {
            return a->artistEntry()->getLastWritten() > b->artistEntry()->getLastWritten();
        }
        if (a->albumEntry() && b->albumEntry()) {
            return a->albumEntry()->getLastWritten() > b->albumEntry()->getLastWritten();
        }
        // tracks entries are always sorted according to their name even if useSortingLatest is set.
        return a->text().toLower() < b->text().toLower();
    } else {
        return a->text().toLower() < b->text().toLower();
    }
}

void xMainMusicWidget::selectArtist(int index) {
    // Check if artist listIndex is valid.
    if ((index >= 0) && (index < artistList->count())) {
        // Retrieve selected artist name and trigger scanForArtist
        emit scanForArtist(artistList->listItem(index)->text(), musicLibraryFilter);
    }
}

void xMainMusicWidget::queueArtistItem(xPlayerListWidgetItem* artistItem) {
    // Do we allow queue tracks.
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Retrieve listIndex for the selected listItem and check if it's valid.
    auto index = artistList->listIndex(artistItem);
    if ((index >= 0) && (index < artistList->count())) {
        // Retrieve selected listIndex name and trigger scanAllAlbumsForArtist
        QApplication::setOverrideCursor(Qt::WaitCursor);
        emit scanAllAlbumsForArtist(artistList->listItem(index)->text(), musicLibraryFilter);
    }
}

void xMainMusicWidget::currentArtistRightClicked(const QPoint& point) {
    auto artistItem = artistList->itemAt(point);
    if (artistItem) {
        auto renamingAllowed = isRenamingAllowed();
        auto queueTracksAllowed = musicPlayer->isQueueTracksAllowed();
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // Are we allowed to rename?
            if (renamingAllowed) {
                renameArtist(artistItem);
            }
        } else {
            QMenu artistMenu;
            // Add section for artist info website.
            artistMenu.addSection(tr("Info"));
            artistMenu.addAction(tr("Artist Website"), this, [=] () {
                musicStacked->setCurrentWidget(musicInfoView);
                musicInfoView->show(artistItem->text());
                if (musicVisualizationWidget) {
                    // Enable the reduced framerate mode for visualization.
                    // Required for usable website browsing.
                    musicVisualizationWidget->setReducedFrameRate(xPlayer::VisualizationDropRate);
                }
            });
            // Add section for similar artist based on recorded transitions.
            auto artistTransitions = xPlayerDatabase::database()->getArtistTransitions(artistItem->text());
            if (!artistTransitions.empty()) {
                artistMenu.addSection(tr("Similar Artists"));
                for (size_t i = 0; (i < artistTransitions.size()) && (i < 10); ++i) {
                    artistMenu.addAction(artistTransitions[i].first, [=]() {
                        artistList->setCurrentItem(artistTransitions[i].first);
                    });
                }
            }
            if ((renamingAllowed) || (queueTracksAllowed)) {
                // Add section for operations only if at least one of them is applicable.
                artistMenu.addSection(tr("Commands"));
                if (queueTracksAllowed) {
                    artistMenu.addAction(tr("Queue Artist"), this, [=] () { queueArtistItem(artistItem); });
                }
                if (renamingAllowed) {
                    artistMenu.addAction(tr("Rename"), this, [=] () { renameArtist(artistItem); });
                }
            }
            artistMenu.exec(artistList->mapToGlobal(point));
        }
    }
}

void xMainMusicWidget::selectAlbum(int index) {
    // Check if index listIndex is valid.
    if ((index >= 0) && (index < albumList->count())) {
        // Retrieve selected artist and listIndex name and
        // trigger scanForArtistAndAlbum
        auto artist = artistList->currentItem()->text();
        auto album = albumList->listItem(index)->text();
        emit scanForArtistAndAlbum(artist, album);
    }
}

void xMainMusicWidget::queueAlbumItem(xPlayerListWidgetItem* albumItem) {
    // Do we allow queue tracks?
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Emulate behavior of selecting album and clicking on the first track.
    if (albumItem == albumList->currentItem()) {
        if (trackList->count() > 0) {
            queueTrackItem(trackList->listItem(0), true);
        }
    }
}

void xMainMusicWidget::currentAlbumRightClicked(const QPoint& point) {
    auto albumItem = albumList->itemAt(point);
    if (albumItem) {
        auto renamingAllowed = isRenamingAllowed();
        auto queueTracksAllowed = musicPlayer->isQueueTracksAllowed();
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // Are we allowed to rename?
            if (isRenamingAllowed()) {
                renameAlbum(albumItem);
            }
        } else {
            if ((renamingAllowed) || (queueTracksAllowed)) {
                // Add section for operations only if at least one of them is applicable.
                QMenu albumMenu;
                albumMenu.addSection(tr("Commands"));
                if (queueTracksAllowed) {
                    albumMenu.addAction(tr("Queue Album"), this, [=]() { queueAlbumItem(albumItem); });
                }
                if (renamingAllowed) {
                    albumMenu.addAction(tr("Rename"), this, [=]() { renameAlbum(albumItem); });
                }
                albumMenu.exec(albumList->mapToGlobal(point));
            }
        }
    }
}

void xMainMusicWidget::dequeueTrackItem(xPlayerListWidgetItem* trackItem, bool removeRemaining) {
    Q_ASSERT(trackItem != nullptr);
    // Do we allow dequeue tracks?
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Retrieve currently selected element
    auto trackIndex = queueList->listIndex(trackItem);
    if ((trackIndex >= 0) && (trackIndex < queueList->count())) {
        auto trackCount = (removeRemaining) ? queueList->count()-1 : trackIndex;
        for (auto i = trackCount; i >= trackIndex; --i) {
            queueList->takeListItem(i);
            // Did we remove the last queued track?
            if (queueList->count() > 0) {
                emit dequeueTrack(i);
            } else {
                musicPlayer->clearQueue();
                playerWidget->clear();
                break;
            }
        }
    }
}

void xMainMusicWidget::queueTrackItem(xPlayerListWidgetItem* trackItem, bool addRemaining) {
    Q_ASSERT(trackItem != nullptr);
    // Do we allow queue tracks?
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Retrieve listIndex for the selected listItem and check if it's valid.
    auto trackIndex = trackList->listIndex(trackItem);
    if ((trackIndex >= 0) && (trackIndex < trackList->count())) {
        // Retrieve selected artist and album name.
        auto artistName = artistList->currentItem()->text();
        auto albumName = albumList->currentItem()->text();
        // Retrieve all track names from the selected listIndex to
        // the end of the album.
        std::vector<xMusicLibraryTrackEntry*> trackObjects;
        QString trackName;
        auto trackCount = (addRemaining) ? trackList->count() : trackIndex+1;
        for (auto i = trackIndex; i < trackCount; ++i) {
            trackName = trackList->listItem(i)->text();
            auto trackObject = musicLibrary->getTrackEntry(artistName, albumName, trackName);
            if (trackObject != nullptr) {
                trackObjects.push_back(trackObject);
            }
            // Add to the playlist (queue)
            queueList->addListItem(trackObject, QString("%1 - %2").arg(artistName, albumName));
        }
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackObjects);
        // Finished queueing tracks.
        emit finishedQueueTracks(true);
        // Update items.
        queueList->updateItems();
    }
}

void xMainMusicWidget::selectArtistSelector(const QString& selector) {
    // Check if the selector is valid.
    if (!selector.isEmpty()) {
        currentArtistSelector = selector;
        // Call with stored list in order to update artist filtering.
        updateScannedArtists(unfilteredArtists);
    }
}

void xMainMusicWidget::jumpArtistSelector(const QString& selector) {
    // Check if the selector is valid.
    if (!selector.isEmpty()) {
        // Find the first selector matching artist.
        for (int index=0; index < artistList->count(); ++index) {
            if (artistList->listItem(index)->text().toLower().startsWith(selector)) {
                artistList->scrollToIndex(index);
                return;
            }
        }
    }
}

void xMainMusicWidget::selectSortingLatest(bool enabled) {
    useSortingLatest = enabled;
    updateScannedArtists(unfilteredArtists);
}

void xMainMusicWidget::queueArtistSelector(const QString& selector) {
    // Do we allow queue tracks.
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Disable queue functionality of selectors for remote libraries.
    if ((!selector.isEmpty()) && (musicLibrary->isLocal())) {
        currentArtistSelector = selector;
        // Call with stored list in order to update artist filtering.
        updateScannedArtists(unfilteredArtists);
        // Trigger scanAllAlbumsForListArtist with given list of artists.
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QStringList filteredArtistNames;
        for (auto filteredArtist : filteredArtists) {
            filteredArtistNames.push_back(filteredArtist->getArtistName());
        }
        emit scanAllAlbumsForListArtists(filteredArtistNames, musicLibraryFilter);
    }
}

void xMainMusicWidget::selectAlbumSelectors(const QStringList& match, const QStringList& notMatch) {
    // We need to the matching selectors since search may add to the album matches.
    musicLibraryFilter.setAlbumMatch(match, notMatch);
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::selectAlbumDatabaseSelectors(const std::map<QString, std::set<QString>>& databaseMatch,
                                                    bool databaseNotMatch) {
    musicLibraryFilter.setDatabaseMatch(databaseMatch, databaseNotMatch);
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::clearAlbumDatabaseSelectors() {
    musicLibraryFilter.clearDatabaseMatch();
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::clearAllAlbumSelectors() {
    musicLibraryFilter.clearAlbumMatch();
    musicLibraryFilter.clearDatabaseMatch();
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::clearSearchSelectorFilter() {
    musicLibraryFilter.clearSearchMatch();
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::updateSearchSelectorFilter(const std::tuple<QString,QString,QString>& match) {
    musicLibraryFilter.setSearchMatch(match);
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::currentState(xMusicPlayer::State state) {
    // Update visualization.
    updateVisualizationView(state == xMusicPlayer::PlayingState);
    // Update the icon for the played track based on the state of the music player.
    auto currentTrack = queueList->listItem(playedTrack);
    if (!currentTrack) {
        return;
    }
    switch (state) {
        case xMusicPlayer::PlayingState: {
            currentTrack->setIcon(":/images/xplay-play.svg");
        } break;
        case xMusicPlayer::PauseState: {
            currentTrack->setIcon(":/images/xplay-pause.svg");
        } break;
        case xMusicPlayer::StopState: {
            currentTrack->setIcon(":/images/xplay-stop.svg");
        } break;
        default: {
            currentTrack->removeIcon();
        } break;
    }
}

void xMainMusicWidget::currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                                    int bitrate, int sampleRate, int bitsPerSample, const QString& quality) {
    // Some parameters are currently unused.
    Q_UNUSED(sampleRate)
    Q_UNUSED(bitsPerSample)
    Q_UNUSED(bitrate)
    Q_UNUSED(quality)
    // Update queue
    currentQueueTrack(index);
    currentArtist = artist;
    currentAlbum = album;
    currentTrackName = track;
    // Update visualization title.
    if (musicPlayer->getVisualization()) {
        musicVisualizationWidget->showTitle(QString("%1 - %2 - %3").arg(track, artist, album));
    }
}

void xMainMusicWidget::currentQueueTrack(int index) {
    // Remove play icon from old track
    auto oldPlayedItem = queueList->listItem(playedTrack);
    if (oldPlayedItem) {
        oldPlayedItem->removeIcon();
    }
    // Update played track.
    playedTrack = index;
    // Set play icon only for currently played one.
    auto playedItem = queueList->listItem(playedTrack);
    if (playedItem) {
        playedItem->setIcon(":/images/xplay-play.svg");
    }
    queueList->setCurrentListIndex(index);
}

void xMainMusicWidget::currentQueueTrackCtrlClicked(xPlayerListWidgetItem* trackItem) {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        auto track = queueList->listIndex(trackItem);
        if ((track >= 0) && (track< queueList->count())) {
            // Access the information stored in the track.
            auto trackEntry = queueList->listItem(track)->trackEntry();
            // Try to select the artist.
            if (artistList->setCurrentItem(trackEntry->getArtistName())) {
                // Try to select the album.
                albumList->setCurrentItem(trackEntry->getAlbumName());
            }
        }
    }
}

void xMainMusicWidget::currentQueueTrackDoubleClicked(xPlayerListWidgetItem* trackItem) {
    auto track = queueList->listIndex(trackItem);
    if ((track >= 0) && (track< queueList->count())) {
        currentQueueTrack(track);
        musicPlayer->play(track);
    }
}

void xMainMusicWidget::currentQueueTrackRightClicked(const QPoint& point) {
    auto trackItem = queueList->itemAt(point);
    if (trackItem) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            dequeueTrackItem(trackItem, false);
        } else {
            QMenu menu;
            addTrackTagMenu(trackItem, &menu);
            addTrackDequeueMenu(trackItem, &menu);
            menu.exec(queueList->mapToGlobal(point));
        }
    }
}

void xMainMusicWidget::clearQueue() {
    // Clear playlist (queue).
    queueList->clearItems();
    // Stop music player.
    musicPlayer->stop();
}

void xMainMusicWidget::updatedMusicViewFilters() {
    auto visible = xPlayerConfiguration::configuration()->getMusicViewFilters();
    artistFilterBox->setVisible(visible);
    albumFilterBox->setVisible(visible);
    trackFilterBox->setVisible(visible);
    // Clear local filters if they are not visible
    if (!visible) {
        artistFilterLineEdit->clear();
        albumFilterLineEdit->clear();
        trackFilterLineEdit->clear();
    }
}

void xMainMusicWidget::updatedMusicViewSelectors() {
    auto visible = xPlayerConfiguration::configuration()->getMusicViewSelectors();
    selectorTabs->setVisible(visible);
    // Clear selectors if not visible
    if (!visible) {
        artistSelectorList->clear();
        albumSelectorList->clear();
        searchSelector->clear();
    }
}

void xMainMusicWidget::updatedMusicViewVisualization() {
    if (musicPlayer->supportsVisualization()) {
        musicVisualizationEnabled = xPlayerConfiguration::configuration()->getMusicViewVisualization();
        musicVisualizationMode = xPlayerConfiguration::configuration()->getMusicViewVisualizationMode();
        musicPlayer->setVisualization(musicVisualizationEnabled);
        if (musicVisualizationEnabled) {
            if (musicVisualizationMode == 0) {
                musicStacked->removeWidget(musicVisualizationWidget);
                // Attach to queue box parent
                musicVisualizationWidget->setParent(queueBox);
                musicVisualizationWidget->setVisible(true);
                // Integrate into queue box layout.
                queueBoxLayout->addRowSpacer(10, xPlayerLayout::SmallSpace);
                queueBoxLayout->addWidget(musicVisualizationWidget, 11, 0, 2, 4);
            } else {
                // Attach to stacked widget parent
                musicVisualizationWidget->setParent(musicStacked);
                musicStacked->addWidget(musicVisualizationWidget);
                // Switch over to the visualization if in a playing state and currently on the list view.
                if ((musicStacked->currentWidget() == musicListView) && (musicPlayer->isPlaying())) {
                    musicStacked->setCurrentWidget(musicVisualizationWidget);
                }
            }
            // Adjust visual part if necessary.
            updateVisualizationView(musicPlayer->isPlaying());
        } else {
            if (musicVisualizationMode == 0) {
                musicVisualizationWidget->setVisible(false);
                // Remove from queue box layout.
                queueBoxLayout->addRowSpacer(10, xPlayerLayout::NoSpace);
                queueBoxLayout->removeWidget(musicVisualizationWidget);
            } else {
                if (musicStacked->currentWidget() != musicInfoView) {
                    musicStacked->setCurrentWidget(musicListView);
                }
            }
        }
        updateMusicViewVisualizationFullWindow(musicVisualizationFullWindow);
    }
}

void xMainMusicWidget::updateMusicViewVisualizationFullWindow(bool enabled) {
    // Only full window mode for central window mode.
    if (musicVisualizationEnabled) {
        // Toggle mode only for central window.
        if (musicVisualizationMode == 1) {
            musicVisualizationFullWindow = enabled;
        } else {
            musicVisualizationFullWindow = false;
        }
        // Hide/show queue box and player widget if full window mode is enabled/disabled
        if (musicVisualizationFullWindow) {
            queueBox->setVisible(false);
            playerWidget->setVisible(false);
            // Hide menu bar.
            emit showMenuBar(false);
            emit showWindowTitle(QString("%1 - %2 - %3").arg(currentTrackName, currentArtist, currentAlbum));
            return;
        }
    }
    // Disabled music visualization or disabled full window mode.
    queueBox->setVisible(true);
    playerWidget->setVisible(true);
    // Show menu bar and reset window title.
    emit showWindowTitle(QApplication::applicationName());
    emit showMenuBar(true);
}

void xMainMusicWidget::updatedDatabaseMusicOverlay() {
    useDatabaseMusicOverlay = xPlayerConfiguration::configuration()->getDatabaseMusicOverlay();
    databaseCutOff = xPlayerConfiguration::configuration()->getDatabaseCutOff();
    if (useDatabaseMusicOverlay) {
        updatePlayedArtists();
        updatePlayedAlbums();
        updatePlayedTracks();
    } else {
        // Clear artist list.
        for (auto i = 0; i < artistList->count(); ++i) {
            artistList->listItem(i)->removeIcon();
            artistList->listItem(i)->removeToolTip();
        }
        // Clear album list.
        for (auto i = 0; i < albumList->count(); ++i) {
            albumList->listItem(i)->removeIcon();
            albumList->listItem(i)->removeToolTip();
        }
        // Clear track list.
        for (auto i = 0; i < trackList->count(); ++i) {
            trackList->listItem(i)->removeIcon();
            trackList->listItem(i)->removeToolTip();
        }
    }
}

void xMainMusicWidget::updatedMusicLibraryAlbumSelectors() {
    albumSelectorList->setSelectors(xPlayerConfiguration::configuration()->getMusicLibraryAlbumSelectorList());
    // Clear match and not match selectors.
    musicLibraryFilter.clearAlbumMatch();
    // Rescan.
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::currentTrackRightClicked(const QPoint& point) {
    auto trackItem = trackList->itemAt(point);
    if (trackItem) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
            queueTrackItem(trackItem, false);
        } else if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // Are we allowed to rename?
            if (isRenamingAllowed()) {
                renameTrack(trackItem);
            }
        } else {
            QMenu menu;
            addTrackTagMenu(trackItem, &menu);
            addTrackQueueMenu(trackItem, &menu);
            menu.exec(trackList->mapToGlobal(point));
        }
    }
}

void xMainMusicWidget::updateTracksTotalTime(qint64 total) {
    if (total > 0) {
        trackBox->setTitle(QString("Tracks - %1").arg(xPlayer::millisecondsToShortTimeFormat(total)));
    } else {
        trackBox->setTitle("Tracks");
    }
}

void xMainMusicWidget::updateQueueTotalTime(qint64 total) {
    if (total > 0) {
        queueBox->setTitle(QString("Queue - %1 Songs - %2").arg(queueList->count()).
                arg(xPlayer::millisecondsToShortTimeFormat(total)));
    } else {
        queueBox->setTitle("Queue");
    }
}

void xMainMusicWidget::updateVisualizationView(bool playing) {
    if (musicVisualizationEnabled) {
        // Switch back-and-forth between visualization and list view only if either of the views is selected.
        if (playing) {
            if (musicVisualizationMode == 0) {
                musicVisualizationWidget->setVisible(true);
                // Integrate into queue box layout.
                queueBoxLayout->addRowSpacer(10, xPlayerLayout::SmallSpace);
                queueBoxLayout->addWidget(musicVisualizationWidget, 11, 0, 2, 4);
            } else {
                if (musicStacked->currentWidget() == musicListView) {
                    musicStacked->setCurrentWidget(musicVisualizationWidget);
                }
            }
        } else {
            if (musicVisualizationMode == 0) {
                musicVisualizationWidget->setVisible(false);
                // Remove from queue box layout.
                queueBoxLayout->addRowSpacer(10, xPlayerLayout::NoSpace);
                queueBoxLayout->removeWidget(musicVisualizationWidget);
            } else {
                if (musicStacked->currentWidget() == musicVisualizationWidget) {
                    musicStacked->setCurrentWidget(musicListView);
                }
            }
        }
    }
}

void xMainMusicWidget::updateWindowTitle(qint64 time) {
    if ((musicPlayer->getVisualization()) && (musicVisualizationFullWindow)) {
        emit showWindowTitle(QString("%1 - %2 - %3 (%4)").arg(currentTrackName, currentArtist, currentAlbum,
                                                              xPlayer::millisecondsToTimeFormat(time, false)));
    }
}

void xMainMusicWidget::updatePlayedArtists() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMusicOverlay) {
        return;
    }
    auto playedArtists = xPlayerDatabase::database()->getPlayedArtists(databaseCutOff);
    for (auto i = 0; i < artistList->count(); ++i) {
        auto artistItem = artistList->listItem(i);
        auto artist = artistItem->text();
        // Clear icon.
        artistItem->removeIcon();
        for (const auto& [playedArtist, playCount] : playedArtists) {
            // Update icon and tooltip if movie already played.
            if (playedArtist == artist) {
                artistItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
                break;
            }
        }
    }
}

void xMainMusicWidget::updatePlayedAlbums() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMusicOverlay) {
        return;
    }
    auto artistItem = artistList->currentItem();
    if (!artistItem) {
        return;
    }
    auto artist = artistItem->text();
    auto playedAlbums = xPlayerDatabase::database()->getPlayedAlbums(artist, databaseCutOff);
    for (auto i = 0; i < albumList->count(); ++i) {
        auto albumItem = albumList->listItem(i);
        auto album = albumItem->text();
        // Clear icon and tooltip.
        albumItem->removeIcon();
        for (const auto& [playedAlbum, playCount] : playedAlbums) {
            // Update icon and tooltip if movie already played.
            if (playedAlbum == album) {
                albumItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
                break;
            }
        }
    }
}

void xMainMusicWidget::updatePlayedTracks() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMusicOverlay) {
        return;
    }
    auto artistItem = artistList->currentItem();
    auto albumItem = albumList->currentItem();
    // No artist or album currently selected. We do not need to update.
    if ((!artistItem) || (!albumItem)) {
        return;
    }
    auto artist = artistItem->text();
    auto album = albumItem->text();
    auto playedMusicTracks = xPlayerDatabase::database()->getPlayedTracks(artist, album, databaseCutOff);
    // Both lists are sorted. We therefore update in one walk-through.
    auto playedMusicTrack = playedMusicTracks.begin();
    for (auto i = 0; (i < trackList->count()) && (playedMusicTrack != playedMusicTracks.end()); ++i) {
        auto trackItem = trackList->listItem(i);
        auto track = trackItem->text();
        auto [currentTrack, playCount, timeStamp] = *playedMusicTrack;
        if (currentTrack == track) {
            // Use the proper icon for the given play count.
            trackItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
            // Adjust tooltip to play count "once" vs "x times".
            if (playCount > 1) {
                trackItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                        arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
            } else {
                trackItem->addToolTip(QString(tr("played once, last time on %1")).
                        arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
            }
            // Move to the next element in the list.
            ++playedMusicTrack;
        } else {
            // Clear icon and tooltip.
            trackItem->removeIcon();
            trackItem->removeToolTip();
        }
    }
}

void xMainMusicWidget::updatePlayedTrack(const QString& artist, const QString& album,
                                         const QString& track, int playCount, qint64 timeStamp) {
    // Only update if the database overlay is enabled.
    if (!useDatabaseMusicOverlay) {
        return;
    }
    auto artistItem = artistList->currentItem();
    // Update the artists.
    auto artistPlayedItems = artistList->findListItems(artist);
    auto artistPlayCount = xPlayerDatabase::database()->getMaxPlayCount(artist, QString(), QString(), databaseCutOff);
    for (auto& artistPlayedItem : artistPlayedItems) {
        artistPlayedItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(artistPlayCount));
    }
    // If no artist selected or the artist does not match the selected
    // artists then we do not need to update the albums.
    if ((!artistItem) || (artistItem->text() != artist)) {
        return;
    }
    auto albumItem = albumList->currentItem();
    auto albumPlayCount = xPlayerDatabase::database()->getMaxPlayCount(artist, album, QString(), databaseCutOff);
    // Update the albums.
    auto albumPlayedItems = albumList->findListItems(album);
    for (auto& albumPlayedItem : albumPlayedItems) {
        albumPlayedItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(albumPlayCount));
    }
    // If no album selected or the album does not match the selected
    // album then we do not need to update the tracks.
    if ((!albumItem) || (albumItem->text() != album)) {
        return;
    }
    auto trackPlayedItems = trackList->findListItems(track);
    for (auto& trackPlayedItem : trackPlayedItems) {
        trackPlayedItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
        if (playCount > 1) {
            trackPlayedItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
        } else {
            trackPlayedItem->addToolTip(QString(tr("played once, last time on %1")).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
        }
    }
}

void xMainMusicWidget::clearTrackList() {
    // Clear only track list
    trackList->clearItems();
}

void xMainMusicWidget::playlist(const std::vector<std::tuple<QString,QString,QString>>& entries) {
    // Clear the player widget.
    playerWidget->clear();
    // Clear queue.
    queueList->clearItems();
    for (const auto& [artist, album, trackName] : entries) {
        // Add to the playlist (queue)
        auto trackObject = musicLibrary->getTrackEntry(artist, album, trackName);
        if (trackObject) {
            queueList->addListItem(trackObject, QString("%1 - %2").arg(artist, album));
        }
    }
    // Update items.
    queueList->updateItems();
}

void xMainMusicWidget::showPlaylistDialog() {
    auto playlistNames = xPlayerDatabase::database()->getMusicPlaylists();
    auto playlistDialog = new xPlayerPlaylistDialog(playlistNames, this);
    connect(playlistDialog, &xPlayerPlaylistDialog::savePlaylist, musicPlayer, &xMusicPlayer::saveQueueToPlaylist);
    connect(playlistDialog, &xPlayerPlaylistDialog::openPlaylist, musicPlayer, &xMusicPlayer::loadQueueFromPlaylist);
    connect(playlistDialog, &xPlayerPlaylistDialog::removePlaylist,
            [=](const QString& name) { xPlayerDatabase::database()->removeMusicPlaylist(name); });
    playlistDialog->exec();
}

void xMainMusicWidget::showTagsDialog() {
    auto tags = xPlayerConfiguration::configuration()->getMusicLibraryTags();
    if (!tags.empty()) {
        auto tagsDialog = new xPlayerTagsDialog(tags, this);
        connect(tagsDialog, &xPlayerTagsDialog::loadFromTag, musicPlayer, &xMusicPlayer::loadQueueFromTag);
        //connect(tagsDialog, &xPlayerTagsDialog::removeFromTag, this, &xMainMusicWidget::removeTagFromQueue);
        tagsDialog->exec();
    }
}

QString xMainMusicWidget::showRenameDialog(const QString& renameType, const QString& text) {
    // Setup rename dialog.
    auto renameDialog = new QDialog(this);
    auto currentTextInput = new QLineEdit(renameDialog);
    currentTextInput->setText(text);
    currentTextInput->setReadOnly(true);
    auto renameTextInput = new QLineEdit(renameDialog);
    renameTextInput->setText(text);
    auto renameDialogButtons = new QDialogButtonBox(QDialogButtonBox::Reset | QDialogButtonBox::Apply | QDialogButtonBox::Discard, renameDialog);
    auto renameLayout = new xPlayerLayout(renameDialog);
    renameLayout->addWidget(new QLabel(QString("Current %1").arg(renameType), renameDialog), 0, 0);
    renameLayout->addWidget(currentTextInput, 1, 0);
    renameLayout->addRowSpacer(2, xPlayerLayout::MediumSpace);
    renameLayout->addWidget(new QLabel(QString("New %1").arg(renameType), renameDialog), 3, 0);
    renameLayout->addWidget(renameTextInput, 4, 0);
    renameLayout->addRowSpacer(5, xPlayerLayout::LargeSpace);
    renameLayout->addWidget(renameDialogButtons, 6, 0);
    renameDialog->setWindowTitle(QString("Rename %1").arg(renameType));
    renameDialog->setMinimumWidth(xPlayer::DialogMinimumWidth);
    // Connect buttons.
    connect(renameDialogButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed, renameDialog, &QDialog::reject);
    connect(renameDialogButtons->button(QDialogButtonBox::Apply), &QPushButton::pressed, renameDialog, &QDialog::accept);
    connect(renameDialogButtons->button(QDialogButtonBox::Reset), &QPushButton::pressed, [renameTextInput,text]() {
        renameTextInput->setText(text);
    });
    if (renameDialog->exec() == QDialog::Accepted) {
        return renameTextInput->text();
    } else {
        return text;
    }
}

void xMainMusicWidget::addTrackQueueMenu(xPlayerListWidgetItem* trackItem, QMenu* menu) {
    auto renamingAllowed = isRenamingAllowed();
    auto queueTracksAllowed = musicPlayer->isQueueTracksAllowed();
    if ((renamingAllowed) || (queueTracksAllowed)) {
        // Add section for operations only if at least one of them is applicable.
        menu->addSection(tr("Commands"));
        if (queueTracksAllowed) {
            menu->addAction(tr("Queue Track"), this, [=]() { queueTrackItem(trackItem, false); });
            menu->addAction(tr("Queue Remaining Tracks"), this, [=]() { queueTrackItem(trackItem, true); });
            menu->addAction(tr("Queue All Tracks"), this, [=]() { queueTrackItem(trackList->listItem(0), true); });
        }
        if (renamingAllowed) {
            menu->addAction(tr("Rename"), this, [=]() { renameTrack(trackItem); });
        }
    }
}

void xMainMusicWidget::addTrackDequeueMenu(xPlayerListWidgetItem* trackItem, QMenu* menu) {
    auto queueTracksAllowed = musicPlayer->isQueueTracksAllowed();
    if (queueTracksAllowed) {
        // Add section for operations only if at least one of them is applicable.
        menu->addSection(tr("Commands"));
        menu->addAction(tr("Dequeue Track"), this, [=]() { dequeueTrackItem(trackItem, false); });
        menu->addAction(tr("Dequeue Remaining Tracks"), this, [=]() { dequeueTrackItem(trackItem, true); });
        menu->addAction(tr("Clear Queue"), this, [=]() {
            // Clear queue list entries.
            clearQueue();
            // Clear queued entries for music player.
            musicPlayer->clearQueue();
            // Clear player widget.
            playerWidget->clear();
        });
    }
}

void xMainMusicWidget::addTrackTagMenu(xPlayerListWidgetItem* trackItem, QMenu* menu) {
    auto menuTags = xPlayerConfiguration::configuration()->getMusicLibraryTags();
    if (menuTags.isEmpty()) {
        // No tags defined.
        return;
    }
    // Track info.
    auto artist = trackItem->trackEntry()->getArtistName();
    auto album = trackItem->trackEntry()->getAlbumName();
    auto trackName = trackItem->trackEntry()->getTrackName();
    // Read current tags for track.
    auto tags = xPlayerDatabase::database()->getTags(artist, album, trackName);
    // Get all available tags.
    // Create menu.
    menu->addSection(tr("Tags"));
    for (const auto& menuTag : menuTags) {
        auto action = new QAction(menuTag);
        action->setCheckable(true);
        // Adjust checked state of menu entry.
        action->setChecked(tags.contains(menuTag));
        // Add the tag if the menu entry is selected, remove it otherwise.
        connect(action, &QAction::triggered, [=](bool checked) {
            if (checked) {
                xPlayerDatabase::database()->addTag(artist, album, trackName, menuTag);
            } else {
                xPlayerDatabase::database()->removeTag(artist, album, trackName, menuTag);
            }
        });
        menu->addAction(action);
    }
    menu->addSeparator();
    menu->addAction(tr("Remove All Tags"), [=]() {
        xPlayerDatabase::database()->removeAllTags(artist, album, trackName);
    });

}

bool xMainMusicWidget::isRenamingAllowed() {
    // Renaming is only allowed for local libraries if the queue is empty and nothing is playing.
    return ((musicLibrary->isLocal()) && (!musicPlayer->isPlaying()) && (queueList->count() == 0));
}

void xMainMusicWidget::renameArtist(xPlayerListWidgetItem* artistItem) {
    auto artistName = artistItem->artistEntry()->getArtistName();
    auto newArtistName = showRenameDialog("Artist", artistName);
    // Do we need to rename the artist?
    if (artistName != newArtistName) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (artistItem->artistEntry()->rename(newArtistName)) {
            // Refresh artist list and selector list.
            scannedArtists(unfilteredArtists);
            // Update database.
            xPlayerDatabase::database()->renameMusicFiles(artistName, newArtistName);
        } else {
            qCritical() << "Unable to rename artist entry: " << newArtistName;
        }
        QApplication::restoreOverrideCursor();
    }
}

void xMainMusicWidget::renameAlbum(xPlayerListWidgetItem *albumItem) {
    auto albumName = albumItem->albumEntry()->getAlbumName();
    auto newAlbumName = showRenameDialog("Album", albumName);
    // Do we need to rename the album?
    if (albumName != newAlbumName) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (albumItem->albumEntry()->rename(newAlbumName)) {
            albumItem->updateText();
            albumList->refreshItems([this](auto a, auto b) { return sortListItems(a, b); });
            // Update database entry.
            xPlayerDatabase::database()->renameMusicFiles(albumItem->albumEntry()->getArtistName(),
                                                          albumName, newAlbumName);
        } else {
            qCritical() << "Unable to rename album entry: " << newAlbumName;
        }
        QApplication::restoreOverrideCursor();
    }
}

void xMainMusicWidget::renameTrack(xPlayerListWidgetItem* trackItem) {
    auto trackName = trackItem->trackEntry()->getTrackName();
    auto newTrackName = showRenameDialog("Track Name", trackName);
    // Do we need to rename the track name?
    if (trackName != newTrackName) {
        if (trackItem->trackEntry()->rename(newTrackName)) {
            trackItem->updateText();
            trackList->refreshItems([this](auto a, auto b) { return sortListItems(a, b); });
            // Update database entry.
            xPlayerDatabase::database()->renameMusicFile(trackItem->trackEntry()->getArtistName(),
                                                         trackItem->trackEntry()->getAlbumName(),
                                                         trackName, newTrackName);
        } else {
            qCritical() << "Unable to rename artist entry: " << newTrackName;
        }
    }
}
