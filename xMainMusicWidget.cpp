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
#include "xMusicFile.h"
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
#include <QFontMetrics>
#include <QApplication>
#include <QRandomGenerator>
#include <QDateTime>
#include <QCheckBox>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <random>

// Function addListWidgetGroupBox has to be defined before the constructor due to the auto return.
auto xMainMusicWidget::addListWidgetGroupBox(const QString& boxLabel, QWidget* parent) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, parent);
    groupBox->setFlat(xPlayerUseFlatGroupBox);
    auto list = new xPlayerListWidget(groupBox);
    auto boxLayout = new QVBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}

auto xMainMusicWidget::addLineEditGroupBox(const QString& boxLabel, QWidget* parent) {
    // Create a QGroupBox with the given label and embed a QLineEdit.
    auto groupBox = new QGroupBox(boxLabel, parent);
    groupBox->setFlat(xPlayerUseFlatGroupBox);
    auto lineEdit = new QLineEdit(groupBox);
    auto boxLayout = new QVBoxLayout();
    boxLayout->addWidget(lineEdit);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox, lineEdit);
}


xMainMusicWidget::xMainMusicWidget(xMusicPlayer* player, xMusicLibrary* library, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicPlayer(player),
        musicLibrary(library),
        playedTrack(0),
        useDatabaseMusicOverlay(true),
        databaseCutOff(0),
        databaseUpdateThread(nullptr),
        currentArtist(),
        currentAlbum() {

    musicStacked = new QStackedWidget(this);
    // Create and group boxes with embedded list widgets.
    musicListView = new QWidget(musicStacked);
    auto [artistBox, artistList_] = addListWidgetGroupBox(tr("Artists"), musicListView);
    auto [albumBox, albumList_] = addListWidgetGroupBox(tr("Albums"), musicListView);
    auto [trackBox_, trackList_] = addListWidgetGroupBox(tr("Tracks"), musicListView);
    // Sort entries in artist/album/track
    artistList = artistList_;
    artistList->enableSorting(false);
    artistList->setContextMenuPolicy(Qt::CustomContextMenu);
    artistList->setMinimumWidth(xPlayerArtistListMinimumWidth);
    albumList = albumList_;
    albumList->enableSorting(true);
    albumList->setMinimumWidth(xPlayerAlbumListMinimumWidth);
    trackList = trackList_;
    trackList->enableSorting(true);
    trackList->setContextMenuPolicy(Qt::CustomContextMenu);
    trackList->setMinimumWidth(xPlayerTracksListMinimumWidth);
    trackBox = trackBox_;
    // Selector Tabs.
    selectorTabs = new QTabWidget(musicListView);
    selectorTabs->setStyleSheet("QTabWidget::pane { border: none; }");
    // Artist selector.
    auto artistSelectorTab = new QWidget(selectorTabs);
    auto artistSelectorLayout = new xPlayerLayout();
    artistSelectorLayout->setContentsMargins(0, 0, 0, 0);
    artistSelectorLayout->setSpacing(0);
    artistSelectorList = new QListWidget(artistSelectorTab);
    artistSelectorList->setViewMode(QListView::IconMode);
    artistSelectorList->setWrapping(false);
    artistSelectorLayout->addWidget(artistSelectorList, 0, 0);
    artistSelectorLayout->addColumnSpacer(1, xPlayerLayout::SmallSpace);
    artistSelectorTab->setLayout(artistSelectorLayout);
    artistSelectorTab->setFixedHeight(static_cast<int>(QFontMetrics(QApplication::font()).height()*xPlayerSelectorHeightFontFactor));
    // Local filters.
    auto [artistFilterBox_, artistFilterLineEdit_] = addLineEditGroupBox(tr("Filter Artists"), musicListView);
    auto [albumFilterBox_, albumFilterLineEdit_] = addLineEditGroupBox(tr("Filter Albums"), musicListView);
    auto [trackFilterBox_, trackFilterLineEdit_] = addLineEditGroupBox(tr("Filter Tracks"), musicListView);
    artistFilterBox = artistFilterBox_;
    artistFilterLineEdit = artistFilterLineEdit_;
    albumFilterBox = albumFilterBox_;
    albumFilterLineEdit = albumFilterLineEdit_;
    trackFilterBox = trackFilterBox_;
    trackFilterLineEdit = trackFilterLineEdit_;
    // Album selector and Search.
    albumSelectorList = new xPlayerMusicAlbumSelectorWidget(selectorTabs);
    searchSelector = new xPlayerMusicSearchWidget(selectorTabs);
    selectorTabs->addTab(artistSelectorTab, tr("Artist Selector"));
    selectorTabs->addTab(albumSelectorList, tr("Album Selector"));
    selectorTabs->addTab(searchSelector, tr("Search"));
    selectorTabs->setFixedHeight(artistSelectorList->height()+selectorTabs->tabBar()->sizeHint().height());
    // Setup layout for list view.
    auto musicListViewLayout = new xPlayerLayout(musicListView);
    musicListViewLayout->setContentsMargins(0, 0, 0, 0);
    musicListViewLayout->setSpacing(xPlayerLayout::SmallSpace);
    musicListViewLayout->addWidget(artistBox, 0, 0, 9, 3);
    musicListViewLayout->addWidget(artistFilterBox, 9, 0, 1, 3);
    musicListViewLayout->addWidget(albumBox, 0, 3, 9, 5);
    musicListViewLayout->addWidget(albumFilterBox, 9, 3, 1, 5);
    musicListViewLayout->addWidget(trackBox, 0, 8, 9, 4);
    musicListViewLayout->addWidget(trackFilterBox, 9, 8, 1, 4);
    musicListViewLayout->addWidget(selectorTabs, 10, 0, 1, 12);

    musicInfoView = new xPlayerArtistInfo(musicStacked);
    musicStacked->addWidget(musicListView);
    musicStacked->addWidget(musicInfoView);
    musicStacked->setCurrentWidget(musicListView);
    // Player widget.
    playerWidget = new xPlayerMusicWidget(musicPlayer, this);
    // Queue list.
    queueBox = new QGroupBox(tr("Queue"), this);
    queueBox->setFlat(xPlayerUseFlatGroupBox);
    auto queueBoxLayout = new xPlayerLayout();
    queueList = new xPlayerListWidget(queueBox);
    queueList->setContextMenuPolicy(Qt::CustomContextMenu);
    queueList->setLayoutMode(QListView::Batched);
    queueList->setDragDropMode(QListWidget::InternalMove);
    queueList->setMinimumWidth(xPlayerQueueListMinimumWidth);
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
    queueBox->setLayout(queueBoxLayout);
    // Setup layout for main widget.
    auto mainWidgetLayout = new xPlayerLayout(this);
    mainWidgetLayout->setSpacing(xPlayerLayout::SmallSpace);
    mainWidgetLayout->addWidget(playerWidget, 0, 0, 1, 12);
    mainWidgetLayout->addWidget(musicStacked, 1, 0, 8, 12);
    mainWidgetLayout->addWidget(queueBox, 0, 12, 9, 4);
    // Connect artist, album, track and selector widgets
    connect(artistList, &QListWidget::currentRowChanged, this, &xMainMusicWidget::selectArtist);
    connect(artistList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::queueArtist);
    connect(albumList, &QListWidget::currentRowChanged, this, &xMainMusicWidget::selectAlbum);
    connect(albumList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::queueAlbum);
    connect(trackList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::selectTrack);
    connect(trackList, &xPlayerListWidget::totalTime, this, &xMainMusicWidget::updateTracksTotalTime);
    connect(artistSelectorList, &QListWidget::currentRowChanged, this, &xMainMusicWidget::selectArtistSelector);
    connect(artistSelectorList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::queueArtistSelector);
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
    connect(musicInfoView, &xPlayerArtistInfo::close, this, [=]() { musicStacked->setCurrentWidget(musicListView); });
    // Connect main widget to music player
    connect(this, &xMainMusicWidget::queueTracks, musicPlayer, &xMusicPlayer::queueTracks);
    connect(this, &xMainMusicWidget::finishedQueueTracks, musicPlayer, &xMusicPlayer::finishedQueueTracks);
    connect(this, &xMainMusicWidget::dequeueTrack, musicPlayer, &xMusicPlayer::dequeTrack);
    // Connect player widget to main widget
    connect(playerWidget, &xPlayerMusicWidget::clearQueue, this, &xMainMusicWidget::clearQueue);
    // Connect queue to main widget
    connect(queueList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::currentQueueTrackDoubleClicked);
    connect(queueList, &QListWidget::itemClicked, this, &xMainMusicWidget::currentQueueTrackCtrlClicked);
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
    connect(artistList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentArtistContextMenu);
    connect(trackList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentTrackRightClicked);
    connect(queueList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentQueueTrackRightClicked);
    // Connect music player to main widget for queue update.
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xMainMusicWidget::currentState);
    // Connect update for current track. Update queue, database and database overlay.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xMainMusicWidget::currentTrack);
    // Connect update of playlist.
    connect(musicPlayer, &xMusicPlayer::playlist, this, &xMainMusicWidget::playlist);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMusicOverlay,
            this, &xMainMusicWidget::updatedDatabaseMusicOverlay);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryAlbumSelectors,
            this, &xMainMusicWidget::updatedMusicLibraryAlbumSelectors);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicViewSelectors,
            this, &xMainMusicWidget::updatedMusicViewSelectors);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicViewFilters,
            this, &xMainMusicWidget::updatedMusicViewFilters);

#if 0
    // Part of the alternate implementation for inserting chunks of tracks into the queue.
    connect(this, &xMainMusicWidget::scanAllAlbumsForListArtistsIterate,
            this, &xMainMusicWidget::scannedAllAlbumsForListArtistsWorker);
#endif
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
    scannedArtists(QStringList());
}

void xMainMusicWidget::scannedArtists(const QStringList& artists) {
    std::set<QString> selectors;
    unfilteredArtists = artists;
    // Use unfiltered list for selectors update
    for (const auto& artist : artists) {
        selectors.insert(artist.left(1));
    }
    // Update the selector based upon the added artists
    updateScannedArtistsSelectors(selectors);
    // Update the artists.
    updateScannedArtists(artists);
}

void xMainMusicWidget::updateScannedArtists(const QStringList& artists) {
    // Clear artist, album and track lists
    artistList->clearItems();
    albumList->clearItems();
    clearTrackList();
    // Not very efficient to use a filtered and the unfiltered list, but easier to read.
    filteredArtists = filterArtists(artists);
    for (const auto& artist : filteredArtists) {
        artistList->addItemWidget(artist);
    }
    // Update database overlay for artists.
    updatePlayedArtists();
}

void xMainMusicWidget::scannedAlbums(const QStringList& albums) {
    // Check for running update thread.
    updateDatabaseFinished();
    // Clear album and track lists
    albumList->clearItems();
    clearTrackList();
    albumList->addItemWidgets(albums);
    // Offload database update into thread.
    databaseUpdateThread = QThread::create([this]() {
        // Update database overlay for albums.
        updatePlayedAlbums();
    });
    databaseUpdateThread->start();
}

void xMainMusicWidget::scannedTracks(const std::list<xMusicFile*>& tracks) {
    // Check for running update thread.
    updateDatabaseFinished();
    // Clear only track list and stop potential running update thread.
    clearTrackList();
    for (const auto& track : tracks) {
        trackList->addItemWidget(track);
    }
    // Offload database update into thread.
    databaseUpdateThread = QThread::create([this]() {
        // Update database overlay for tracks. Run in separate thread.
        updatePlayedTracks();
        // Update track times.
        trackList->updateItems();
    });
    databaseUpdateThread->start();
}

void xMainMusicWidget::scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,
                                             std::vector<xMusicFile*>>>& albumTracks) {
    for (const auto& albumTrack : albumTracks) {
        queueList->addItemWidgets(albumTrack.second, QString("%1 - %2").arg(artist, albumTrack.first));
        emit queueTracks(artist, albumTrack.first, albumTrack.second);
    }
    emit finishedQueueTracks(true);
    // Update items.
    queueList->updateItems();
    // Restore the waiting cursor.
    QApplication::restoreOverrideCursor();
}

void xMainMusicWidget::scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString,
                                                        std::vector<xMusicFile*>>>>>& listTracks) {

    // Compute the number of files to be inserted.
    int maxListTracks = 0;
    for (const auto& listElem : listTracks) {
        for (const auto& albumTrack : listElem.second) {
            maxListTracks += static_cast<int>(albumTrack.second.size());
        }
    }
    qDebug() << "scannedListArtistsAllAlbumTracks: maxFiles: " << maxListTracks;

    if (maxListTracks > xPlayerQueueCriticalNumberEntries) {
        QApplication::restoreOverrideCursor();
        auto result = QMessageBox::warning(this, tr("Queue"),
                                           tr("Do you want to add %1 tracks to the queue?\n"
                                              "This process may takes several minutes and it cannot be aborted.").
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
            queueList->addItemWidgets(albumTrack.second, QString("%1 - %2").arg(listElem.first, albumTrack.first));
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

#if 0
    // Alternative implementation that does not block the queue during update.
    // Issue. Blocking occurs later as with approach above, when items are visualized.
    artistList->setUpdatesEnabled(false);
    albumList->setUpdatesEnabled(false);
    trackList->setUpdatesEnabled(false);
    queueList->setUpdatesEnabled(false);

    queueProgress->setRange(0, maxListTracks);
    queueProgress->setVisible(true);

    emit scanAllAlbumsForListArtistsIterate(listTracks, listTracks.begin(), 0, maxListTracks);
#endif
}

void xMainMusicWidget::scannedAllAlbumsForListArtistsWorker(
        const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>& listTracks,
        const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>::const_iterator& listTracksIterator,
        int currentFiles, int maxFiles) {

    if (listTracksIterator != listTracks.end()) {
        // Update progress bar including format.
        queueProgress->setFormat(QString("%1 - %p%").arg(listTracksIterator->first));
        queueProgress->setValue(currentFiles);
        for (const auto& albumTrack : listTracksIterator->second) {
            qDebug() << "scannedListArtistsAllAlbumTracks: " << listTracksIterator->first << "," << albumTrack.first;
            // Update queue list UI.
            queueList->addItemWidgets(albumTrack.second, QString("%1 - %2").arg(listTracksIterator->first, albumTrack.first));
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

void xMainMusicWidget::updateScannedArtistsSelectors(const std::set<QString> &selectors) {
    // Update artist selectors list widget.
    artistSelectorList->clear();
    artistSelectorList->addItem(tr("none"));
    artistSelectorList->addItem(tr("random"));
    for (const auto& as : selectors) {
        artistSelectorList->addItem(as);
    }
}

QStringList xMainMusicWidget::filterArtists(const QStringList& artists) {
    // Check if a selector is selected. We sort the list if necessary.
    QStringList filtered;
    if (artistSelectorList->currentItem()) {
        QString selected = artistSelectorList->currentItem()->text();
        // Do not filter if we have selector "none" selected.
        if (selected.compare(tr("none"), Qt::CaseInsensitive) == 0) {
            filtered = artists;
            filtered.sort();
            return filtered;
        }
        // Do not filter if we have selector "random" selected. Just randomize list.
        if (selected.compare(tr("random"), Qt::CaseInsensitive) == 0) {
            filtered = artists;
            std::shuffle(filtered.begin(), filtered.end(), std::mt19937(std::random_device()()));
            return filtered;
        }
        // Go through list of artists and only add the ones to the filtered
        // list that start with the selector character.
        for (const auto& artist : artists) {
            if (artist.startsWith(selected)) {
                filtered.push_back(artist);
            }
        }
        // Return filtered list of artists.
        filtered.sort();
        return filtered;
    }
    // No artist selector enabled. Return sorted list of artists.
    filtered = artists;
    filtered.sort();
    return filtered;
}

void xMainMusicWidget::selectArtist(int artist) {
    // Check if artist index is valid.
    if ((artist >= 0) && (artist < artistList->count())) {
        // Retrieve selected artist name and trigger scanForArtist
        auto artistName = artistList->itemWidget(artist)->text();
        emit scanForArtist(artistName, musicLibraryFilter);
    }
}

void xMainMusicWidget::queueArtist(QListWidgetItem *artistItem) {
    // Retrieve index for the selected item and check if it's valid.
    auto artist = artistList->row(artistItem);
    if ((artist >= 0) && (artist < artistList->count())) {
        // Retrieve selected artist name and trigger scanAllAlbumsForArtist
        QApplication::setOverrideCursor(Qt::WaitCursor);
        emit scanAllAlbumsForArtist(artistList->itemWidget(artist)->text(), musicLibraryFilter);
    }
}

void xMainMusicWidget::currentArtistContextMenu(const QPoint& point) {
    auto artistItem = artistList->itemWidgetAt(point);
    if (artistItem) {
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
                    artistList->setCurrentWidgetItem(artistTransitions[i].first);
                });
            }
        }
        artistMenu.exec(artistList->mapToGlobal(point));
    }
}

void xMainMusicWidget::selectAlbum(int album) {
    // Check if album index is valid.
    if ((album >= 0) && (album < albumList->count())) {
        // Retrieve selected artist and album name and
        // trigger scanForArtistAndAlbum
        auto artistName = artistList->currentItemWidget()->text();
        auto albumName = albumList->itemWidget(album)->text();
        emit scanForArtistAndAlbum(artistName, albumName);
    }
}

void xMainMusicWidget::queueAlbum(QListWidgetItem* albumItem) {
    // Emulate behavior of selecting album and clicking on the first track.
    if (albumItem == albumList->currentItem()) {
        if (trackList->count() > 0) {
            selectTrack(trackList->item(0));
        }
    }
}

void xMainMusicWidget::selectTrack(QListWidgetItem* trackItem) {
    // Retrieve index for the selected item and check if it's valid.
    auto track = trackList->row(trackItem);
    if ((track >= 0) && (track< trackList->count())) {
        // Retrieve selected artist and album name.
        auto artistName = artistList->currentItemWidget()->text();
        auto albumName = albumList->currentItemWidget()->text();
        // Retrieve all track names from the selected index to
        // the end of the album.
        std::vector<xMusicFile*> trackObjects;
        QString trackName;
        for (auto i = track; i < trackList->count(); ++i) {
            trackName = trackList->itemWidget(i)->text();
            auto trackObject = musicLibrary->getMusicFile(artistName, albumName, trackName);
            if (trackObject != nullptr) {
                trackObjects.push_back(trackObject);
            }
            // Add to the playlist (queue)
            queueList->addItemWidget(trackObject, QString("%1 - %2").arg(artistName, albumName));
        }
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackObjects);
        // Finished queueing tracks.
        emit finishedQueueTracks(true);
        // Update items.
        queueList->updateItems();
    }
}

void xMainMusicWidget::selectArtistSelector(int selector) {
    // Check if index is valid.
    if ((selector >= 0) && (selector < artistSelectorList->count())) {
        // Call with stored list in order to update artist filtering.
        updateScannedArtists(unfilteredArtists);
    }
}

void xMainMusicWidget::queueArtistSelector(QListWidgetItem* selectorItem) {
    // Currently unused
    auto selector = artistSelectorList->row(selectorItem);
    if ((selector >= 0) && (selector < artistSelectorList->count())) {
        // Do not queue artist selector if random is selected, just randomize again.
        if (selectorItem->text().compare("random", Qt::CaseInsensitive) == 0) {
            selectArtistSelector(selector);
            return;
        }
        QStringList listArtists;
        // Call with stored list in order to update artist filtering.
        updateScannedArtists(unfilteredArtists);
        // Trigger scanAllAlbumsForListArtist with given list of artists.
        QApplication::setOverrideCursor(Qt::WaitCursor);
        emit scanAllAlbumsForListArtists(filteredArtists, musicLibraryFilter);
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
    // Update the icon for the played track based on the state of the music player.
    auto currentTrack = queueList->itemWidget(playedTrack);
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
                                    int bitrate, int sampleRate, int bitsPerSample) {
    Q_UNUSED(bitrate)
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
}

void xMainMusicWidget::currentQueueTrack(int index) {
    // Remove play icon from old track
    auto oldPlayedItem = queueList->itemWidget(playedTrack);
    if (oldPlayedItem) {
        oldPlayedItem->removeIcon();
    }
    // Update played track.
    playedTrack = index;
    // Set play icon only for currently played one.
    auto playedItem = queueList->itemWidget(playedTrack);
    if (playedItem) {
        playedItem->setIcon(":/images/xplay-play.svg");
    }
    queueList->setCurrentRow(index);
}

void xMainMusicWidget::currentQueueTrackCtrlClicked(QListWidgetItem* trackItem) {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        auto track = queueList->row(trackItem);
        if ((track >= 0) && (track< queueList->count())) {
            // Access the information stored in the track.
            auto musicFile = queueList->itemWidget(track)->musicFile();
            // Try to select the artist.
            if (artistList->setCurrentWidgetItem(musicFile->getArtist())) {
                // Try to select the album.
                albumList->setCurrentWidgetItem(musicFile->getAlbum());
            }
        }
    }
}

void xMainMusicWidget::currentQueueTrackDoubleClicked(QListWidgetItem* trackItem) {
    auto track = queueList->row(trackItem);
    if ((track >= 0) && (track< queueList->count())) {
        currentQueueTrack(track);
        musicPlayer->play(track);
    }
}

void xMainMusicWidget::currentQueueTrackRightClicked(const QPoint& point) {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        // Retrieve currently selected element
        auto track = queueList->currentRow();
        if ((track >= 0) && (track< queueList->count())) {
            queueList->takeItemWidget(track);
            emit dequeueTrack(track);
        }
    } else {
        tagPopupMenu(queueList, point);
    }
}

void xMainMusicWidget::clearQueue() {
    // Clear playlist (queue).
    queueList->clearItems();
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

void xMainMusicWidget::updatedDatabaseMusicOverlay() {
    useDatabaseMusicOverlay = xPlayerConfiguration::configuration()->getDatabaseMusicOverlay();
    databaseCutOff = xPlayerConfiguration::configuration()->getDatabaseCutOff();
    // Wait for database update thread.
    updateDatabaseFinished();
    if (useDatabaseMusicOverlay) {
        updatePlayedArtists();
        updatePlayedAlbums();
        updatePlayedTracks();
    } else {
        // Clear artist list.
        for (auto i = 0; i < artistList->count(); ++i) {
            artistList->itemWidget(i)->removeIcon();
            artistList->itemWidget(i)->removeToolTip();
        }
        // Clear album list.
        for (auto i = 0; i < albumList->count(); ++i) {
            albumList->itemWidget(i)->removeIcon();
            albumList->itemWidget(i)->removeToolTip();
        }
        // Clear track list.
        for (auto i = 0; i < trackList->count(); ++i) {
            trackList->itemWidget(i)->removeIcon();
            trackList->itemWidget(i)->removeToolTip();
        }
    }
}

void xMainMusicWidget::updateDatabaseFinished() {
    // Check for running update thread.
    if ((databaseUpdateThread) && (databaseUpdateThread->isRunning())) {
        databaseUpdateThread->requestInterruption();
        databaseUpdateThread->wait();
        delete databaseUpdateThread;
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
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        // Retrieve currently selected element
        auto track = trackList->currentRow();
        if ((track >= 0) && (track < trackList->count())) {
            // Retrieve selected artist and album name.
            auto artistName = artistList->currentItemWidget()->text();
            auto albumName = albumList->currentItemWidget()->text();
            // Retrieve only selected track
            std::vector<xMusicFile *> trackObjects;
            QString trackName = trackList->itemWidget(track)->text();
            auto trackObject = musicLibrary->getMusicFile(artistName, albumName, trackName);
            if (trackObject == nullptr) {
                qCritical() << "xMainMusicWidget: unable to find music file object in library for "
                            << artistName + "/" + albumName + "/" + trackName;
                return;
            }
            trackObjects.push_back(trackObject);
            // Add to the playlist (queue)
            queueList->addItemWidget(trackObject, QString("%1 - %2").arg(artistName, albumName));
            // Signal the set tracks to be queued by the music player.
            emit queueTracks(artistName, albumName, trackObjects);
            // Signal finish adding tracks.
            emit finishedQueueTracks(true);
            // Update items.
            queueList->updateItems();
        }
    } else {
        tagPopupMenu(trackList, point);
    }
}

// Little helper function.
auto msToTimeString = [](qint64 total) {
    auto hours = total/3600000;
    auto minutes = (total/60000)%60;
    auto seconds = (total/1000)%60;
    // More than one hour
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
    }
};

void xMainMusicWidget::updateTracksTotalTime(qint64 total) {
    if (total > 0) {
        trackBox->setTitle(QString("Tracks - %1").arg(msToTimeString(total)));
    } else {
        trackBox->setTitle("Tracks");
    }
}

void xMainMusicWidget::updateQueueTotalTime(qint64 total) {
    if (total > 0) {
        queueBox->setTitle(QString("Queue - %1 Songs - %2").arg(queueList->count()).arg(msToTimeString(total)));
    } else {
        queueBox->setTitle("Queue");
    }
}

void xMainMusicWidget::updatePlayedArtists() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMusicOverlay) {
        return;
    }
    auto playedArtists = xPlayerDatabase::database()->getPlayedArtists(databaseCutOff);
    for (auto i = 0; i < artistList->count(); ++i) {
        auto artistItem = artistList->itemWidget(i);
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
    auto artistItem = artistList->currentItemWidget();
    if (!artistItem) {
        return;
    }
    auto artist = artistItem->text();
    auto playedAlbums = xPlayerDatabase::database()->getPlayedAlbums(artist, databaseCutOff);
    for (auto i = 0; i < albumList->count(); ++i) {
        // Check if another update scheduled.
        if ((databaseUpdateThread) && (databaseUpdateThread->isInterruptionRequested())) {
            return;
        }
        auto albumItem = albumList->itemWidget(i);
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
    auto artistItem = artistList->currentItemWidget();
    auto albumItem = albumList->currentItemWidget();
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
        // Check if another update scheduled.
        if ((databaseUpdateThread) && (databaseUpdateThread->isInterruptionRequested())) {
            return;
        }
        auto trackItem = trackList->itemWidget(i);
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
    auto artistItem = artistList->currentItemWidget();
    // Update the artists.
    auto artistPlayedItems = artistList->findItemWidgets(artist);
    for (auto& artistPlayedItem : artistPlayedItems) {
        artistPlayedItem->setIcon(":images/xplay-star.svg");
    }
    // If no artist selected or the artist does not match the selected
    // artists then we do not need to update the albums.
    if ((!artistItem) || (artistItem->text() != artist)) {
        return;
    }
    auto albumItem = albumList->currentItemWidget();
    // Update the albums.
    auto albumPlayedItems = albumList->findItemWidgets(album);
    for (auto& albumPlayedItem : albumPlayedItems) {
        albumPlayedItem->setIcon(":images/xplay-star.svg");
    }
    // If no album selected or the album does not match the selected
    // album then we do not need to update the tracks.
    if ((!albumItem) || (albumItem->text() != album)) {
        return;
    }
    auto trackPlayedItems = trackList->findItemWidgets(track);
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
        auto trackObject = musicLibrary->getMusicFile(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry));
        if (trackObject) {
            queueList->addItemWidget(trackObject, QString("%1 - %2").arg(std::get<0>(entry), std::get<1>(entry)));
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

void xMainMusicWidget::tagPopupMenu(xPlayerListWidget* list, const QPoint& point) {
    auto item = list->itemWidgetAt(point);
    if (!item) {
        // No item at the current position. No need for the tag menu.
        return;
    }
    auto menuTags = xPlayerConfiguration::configuration()->getMusicLibraryTags();
    if (menuTags.isEmpty()) {
        // No tags defined.
        return;
    }
    // Track info.
    auto artist = item->musicFile()->getArtist();
    auto album = item->musicFile()->getAlbum();
    auto trackname = item->musicFile()->getTrackName();
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
