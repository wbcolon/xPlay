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
    // Does the music player support visualization.
    if (xMusicPlayer::supportsVisualization()) {
        musicVisualizationWidget = new xPlayerVisualizationWidget(musicStacked);
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
        musicStacked->addWidget(musicVisualizationWidget);
    }
    musicStacked->setCurrentWidget(musicListView);
    musicPlayerStackedLayout->addWidget(playerWidget);
    musicPlayerStackedLayout->addWidget(musicStacked);
    // Queue list.
    queueBox = new QGroupBox(tr("Queue"), mainWidgetSplitter);
    queueBox->setFlat(xPlayer::UseFlatGroupBox);
    auto queueBoxLayout = new xPlayerLayout(queueBox);
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
    connect(artistList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMusicWidget::queueArtist);
    connect(albumList, &xPlayerListWidget::currentListIndexChanged, this, &xMainMusicWidget::selectAlbum);
    connect(albumList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMusicWidget::queueAlbum);
    connect(trackList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMusicWidget::selectTrack);
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
        updateVisualizationView(musicPlayer->isPlaying());
    });
    // Connect main widget to music player
    connect(this, &xMainMusicWidget::queueTracks, musicPlayer, &xMusicPlayer::queueTracks);
    connect(this, &xMainMusicWidget::finishedQueueTracks, musicPlayer, &xMusicPlayer::finishedQueueTracks);
    connect(this, &xMainMusicWidget::dequeueTrack, musicPlayer, &xMusicPlayer::dequeTrack);
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

void xMainMusicWidget::queueArtist(xPlayerListWidgetItem* artistItem) {
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
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // Are we allowed to rename?
            if (!isRenamingAllowed()) {
                return;
            }
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
        } else {
            QMenu artistMenu;
            // Add section for artist info website.
            artistMenu.addSection(tr("Artist Info"));
            artistMenu.addAction(tr("Link To Website"), this, [=] () {
                musicStacked->setCurrentWidget(musicInfoView);
                musicInfoView->show(artistItem->text());
            });
            // Add section for artist transitions.
            auto artistTransitions = xPlayerDatabase::database()->getArtistTransitions(artistItem->text());
            if (!artistTransitions.empty()) {
                artistMenu.addSection(tr("Other Artists"));
                for (size_t i = 0; (i < artistTransitions.size()) && (i < 10); ++i) {
                    artistMenu.addAction(artistTransitions[i].first, [=]() {
                        artistList->setCurrentItem(artistTransitions[i].first);
                    });
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

void xMainMusicWidget::queueAlbum(xPlayerListWidgetItem* albumItem) {
    // Do we allow queue tracks.
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Emulate behavior of selecting album and clicking on the first track.
    if (albumItem == albumList->currentItem()) {
        if (trackList->count() > 0) {
            selectTrack(trackList->listItem(0));
        }
    }
}

void xMainMusicWidget::currentAlbumRightClicked(const QPoint& point) {
    auto albumItem = albumList->itemAt(point);
    if (albumItem) {
        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // Are we allowed to rename?
            if (!isRenamingAllowed()) {
                return;
            }
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
    }
}

void xMainMusicWidget::selectTrack(xPlayerListWidgetItem* trackItem) {
    // Do we allow queue tracks.
    if (!musicPlayer->isQueueTracksAllowed()) {
        return;
    }
    // Retrieve listIndex for the selected listItem and check if it's valid.
    auto track = trackList->listIndex(trackItem);
    if ((track >= 0) && (track< trackList->count())) {
        // Retrieve selected artist and album name.
        auto artistName = artistList->currentItem()->text();
        auto albumName = albumList->currentItem()->text();
        // Retrieve all track names from the selected listIndex to
        // the end of the album.
        std::vector<xMusicLibraryTrackEntry*> trackObjects;
        QString trackName;
        for (auto i = track; i < trackList->count(); ++i) {
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
    // Currently unused
    if (!selector.isEmpty()) {
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
    Q_UNUSED(bitrate)
    Q_UNUSED(quality)
    // Update queue
    currentQueueTrack(index);
    // Update database.
    auto result = xPlayerDatabase::database()->updateMusicFile(artist, album, track, sampleRate, bitsPerSample);
    if (result.second > 0) {
        // Update database overlay.
        updatePlayedTrack(artist, album, track, result.first, result.second);
    }
    // Update transitions
    if (((!currentArtist.isEmpty()) && (!currentAlbum.isEmpty())) &&
        ((currentArtist != artist) || (currentAlbum != album))) {
        auto transition = xPlayerDatabase::database()->updateTransition(currentArtist, currentAlbum, artist, album,
                                                                        musicPlayer->getShuffleMode());
        // Currently unused.
        Q_UNUSED(transition)
    }
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
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        // Retrieve currently selected element
        auto track = queueList->currentListIndex();
        if ((track >= 0) && (track< queueList->count())) {
            queueList->takeListItem(track);
            // Did we remove the last queued track?
            if (queueList->count() > 0) {
                emit dequeueTrack(track);
            } else {
                musicPlayer->clearQueue();
                playerWidget->clear();
            }
        }
    } else {
        tagPopupMenu(queueList, point);
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
    if (xMusicPlayer::supportsVisualization()) {
        musicVisualizationEnabled = xPlayerConfiguration::configuration()->getMusicViewVisualization();
        musicPlayer->setVisualization(musicVisualizationEnabled);
        if (musicVisualizationEnabled) {
            // Switch over to the visualization if in a playing state and currently on the list view.
            if ((musicStacked->currentWidget() == musicListView) && (musicPlayer->isPlaying())) {
                musicStacked->setCurrentWidget(musicVisualizationWidget);
            }
        } else {
            if (musicStacked->currentWidget() != musicInfoView) {
                musicStacked->setCurrentWidget(musicListView);
            }
        }
        updateMusicViewVisualizationFullWindow(musicVisualizationFullWindow);
    }
}

void xMainMusicWidget::updateMusicViewVisualizationFullWindow(bool enabled) {
    if (musicVisualizationEnabled) {
        // Toggle mode.
        musicVisualizationFullWindow = enabled;
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
            // Retrieve selected artist and album name.
            auto artistName = trackItem->trackEntry()->getArtistName();
            auto albumName = trackItem->trackEntry()->getAlbumName();
            // Retrieve only selected track
            std::vector<xMusicLibraryTrackEntry*> trackObjects;
            QString trackName = trackItem->trackEntry()->getTrackName();
            auto trackObject = musicLibrary->getTrackEntry(artistName, albumName, trackName);
            if (trackObject == nullptr) {
                qCritical() << "xMainMusicWidget: unable to find music file object in library for "
                            << artistName + "/" + albumName + "/" + trackName;
                return;
            }
            trackObjects.push_back(trackObject);
            // Add to the playlist (queue)
            queueList->addListItem(trackObject, QString("%1 - %2").arg(artistName, albumName));
            // Signal the set tracks to be queued by the music player.
            emit queueTracks(artistName, albumName, trackObjects);
            // Signal finish adding tracks.
            emit finishedQueueTracks(true);
            // Update items.
            queueList->updateItems();
        } else if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            // Are we allowed to rename?
            if (!isRenamingAllowed()) {
                return;
            }
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
        } else {
            tagPopupMenu(trackList, point);
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
            if (musicStacked->currentWidget() == musicListView) {
                musicStacked->setCurrentWidget(musicVisualizationWidget);
            }
        } else {
            if (musicStacked->currentWidget() == musicVisualizationWidget) {
                musicStacked->setCurrentWidget(musicListView);
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
        for (const auto& playedArtist : playedArtists) {
            // Update icon and tooltip if movie already played.
            if (playedArtist == artist) {
                artistItem->setIcon(":images/xplay-star.svg");
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
        for (const auto& playedAlbum : playedAlbums) {
            // Update icon and tooltip if movie already played.
            if (playedAlbum == album) {
                albumItem->setIcon(":images/xplay-star.svg");
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
        if (std::get<0>(*playedMusicTrack) == track) {
            trackItem->setIcon(":images/xplay-star.svg");
            // Adjust tooltip to play count "once" vs "x times".
            auto playCount = std::get<1>(*playedMusicTrack);
            if (playCount > 1) {
                trackItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                        arg(QDateTime::fromMSecsSinceEpoch(std::get<2>(*playedMusicTrack)).toString(
                        Qt::DefaultLocaleLongDate)));
            } else {
                trackItem->addToolTip(QString(tr("played once, last time on %1")).
                        arg(QDateTime::fromMSecsSinceEpoch(std::get<2>(*playedMusicTrack)).toString(
                        Qt::DefaultLocaleLongDate)));
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
    for (auto& artistPlayedItem : artistPlayedItems) {
        artistPlayedItem->setIcon(":images/xplay-star.svg");
    }
    // If no artist selected or the artist does not match the selected
    // artists then we do not need to update the albums.
    if ((!artistItem) || (artistItem->text() != artist)) {
        return;
    }
    auto albumItem = albumList->currentItem();
    // Update the albums.
    auto albumPlayedItems = albumList->findListItems(album);
    for (auto& albumPlayedItem : albumPlayedItems) {
        albumPlayedItem->setIcon(":images/xplay-star.svg");
    }
    // If no album selected or the album does not match the selected
    // album then we do not need to update the tracks.
    if ((!albumItem) || (albumItem->text() != album)) {
        return;
    }
    auto trackPlayedItems = trackList->findListItems(track);
    for (auto& trackPlayedItem : trackPlayedItems) {
        trackPlayedItem->setIcon(":images/xplay-star.svg");
        if (playCount > 1) {
            trackPlayedItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::DefaultLocaleLongDate)));
        } else {
            trackPlayedItem->addToolTip(QString(tr("played once, last time on %1")).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::DefaultLocaleLongDate)));
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
    for (const auto& entry : entries) {
        // Add to the playlist (queue)
        auto trackObject = musicLibrary->getTrackEntry(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry));
        if (trackObject) {
            queueList->addListItem(trackObject, QString("%1 - %2").arg(std::get<0>(entry), std::get<1>(entry)));
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

void xMainMusicWidget::tagPopupMenu(xPlayerListWidget* list, const QPoint& point) {
    auto item = list->itemAt(point);
    if (!item) {
        // No listItem at the current position. No need for the tag menu.
        return;
    }
    auto menuTags = xPlayerConfiguration::configuration()->getMusicLibraryTags();
    if (menuTags.isEmpty()) {
        // No tags defined.
        return;
    }
    // Track info.
    auto artist = item->trackEntry()->getArtistName();
    auto album = item->trackEntry()->getAlbumName();
    auto trackname = item->trackEntry()->getTrackName();
    // Read current tags for track.
    auto tags = xPlayerDatabase::database()->getTags(artist, album, trackname);
    // Get all available tags.
    // Create menu.
    QMenu menu;
    menu.addSection(tr("Tags"));
    for (const auto& menuTag : menuTags) {
        auto action = new QAction(menuTag);
        action->setCheckable(true);
        // Adjust checked state of menu entry.
        action->setChecked(tags.contains(menuTag));
        // Add the tag if the menu entry is selected, remove it otherwise.
        connect(action, &QAction::triggered, [=](bool checked) {
            if (checked) {
                xPlayerDatabase::database()->addTag(artist, album, trackname, menuTag);
            } else {
                xPlayerDatabase::database()->removeTag(artist, album, trackname, menuTag);
            }
        });
        menu.addAction(action);
    }
    menu.addSeparator();
    menu.addAction(tr("Remove All Tags"), [=]() {
        xPlayerDatabase::database()->removeAllTags(artist, album, trackname);
    });
    menu.exec(list->mapToGlobal(point));
}

bool xMainMusicWidget::isRenamingAllowed() {
    return ((!musicPlayer->isPlaying()) && (queueList->count() == 0));
}
