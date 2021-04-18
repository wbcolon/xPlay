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

#include <QGroupBox>
#include <QListWidget>
#include <QListView>
#include <QFontMetrics>
#include <QApplication>
#include <QRandomGenerator>
#include <QDateTime>
#include <QCheckBox>
#include <QButtonGroup>
#include <random>

// Function addGroupBox has to be defined before the constructor due to the auto return.
auto xMainMusicWidget::addGroupBox(const QString& boxLabel) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, this);
    groupBox->setFlat(xPlayerUseFlatGroupBox);
    auto list = new xPlayerListWidget(groupBox);
    auto boxLayout = new QVBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}

xMainMusicWidget::xMainMusicWidget(xMusicPlayer* player, xMusicLibrary* library, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicPlayer(player),
        musicLibrary(library),
        albumSelectorMatch(),
        albumSelectorNotMatch(),
        playedTrack(0),
        useDatabaseMusicOverlay(true),
        databaseCutOff(0),
        currentArtist(),
        currentAlbum() {
    // Create and group boxes with embedded list widgets.
    auto [artistBox, artistList_] = addGroupBox(tr("Artists"));
    auto [albumBox, albumList_] = addGroupBox(tr("Albums"));
    auto [trackBox_, trackList_] = addGroupBox(tr("Tracks"));
    playerWidget = new xPlayerMusicWidget(musicPlayer, this);
    // Sort entries in artist/album/track
    artistList = artistList_;
    artistList->enableSorting(false);
    // auto artistFont = artistList->font();
    // artistFont.setPointSizeF(artistFont.pointSizeF()*1.3);
    // artistList->setFont(artistFont);
    albumList = albumList_;
    albumList->enableSorting(true);
    trackList = trackList_;
    trackList->enableSorting(true);
    trackList->setContextMenuPolicy(Qt::CustomContextMenu);
    trackBox = trackBox_;
    // Queue list.
    queueBox = new QGroupBox(tr("Queue"), this);
    queueBox->setFlat(xPlayerUseFlatGroupBox);
    auto queueBoxLayout = new xPlayerLayout();
    queueList = new xPlayerListWidget(queueBox);
    queueList->setContextMenuPolicy(Qt::CustomContextMenu);
    auto queueShuffleCheck = new QCheckBox(tr("Shuffle Mode"), queueBox);
    // Playlist menu.
    auto queuePlaylistButton = new QPushButton("Playlist", queueBox);
    queuePlaylistButton->setFlat(false);
    queueBoxLayout->addWidget(queueList, 0, 0, 8, 3);
    queueBoxLayout->addRowSpacer(8, xPlayerLayout::MediumSpace);
    queueBoxLayout->addWidget(queueShuffleCheck, 9, 0);
    queueBoxLayout->addWidget(queuePlaylistButton, 9, 2);
    queueBox->setLayout(queueBoxLayout);
    // Selector Tabs.
    auto selectorTabs = new QTabWidget(this);
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
    // Album selector and Search.
    albumSelectorList = new xPlayerMusicAlbumSelectorWidget(selectorTabs);
    searchSelector = new xPlayerMusicSearchWidget(selectorTabs);
    selectorTabs->addTab(artistSelectorTab, tr("Artist Selector"));
    selectorTabs->addTab(albumSelectorList, tr("Album Selector"));
    selectorTabs->addTab(searchSelector, tr("Search"));
    selectorTabs->setFixedHeight(artistSelectorList->height()+selectorTabs->tabBar()->sizeHint().height());
    // Setup layout for main widget.
    auto mainWidgetLayout = new xPlayerLayout(this);
    mainWidgetLayout->setSpacing(xPlayerLayout::SmallSpace);
    mainWidgetLayout->addWidget(playerWidget, 0, 0, 1, 12);
    mainWidgetLayout->addWidget(artistBox, 1, 0, 7, 3);
    mainWidgetLayout->addWidget(albumBox, 1, 3, 7, 5);
    mainWidgetLayout->addWidget(trackBox, 1, 8, 7, 4);
    mainWidgetLayout->addWidget(selectorTabs, 8, 0, 1, 12);
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
    connect(albumSelectorList, &xPlayerMusicAlbumSelectorWidget::clearAllSelectors, this, &xMainMusicWidget::clearAlbumSelectors);
    connect(searchSelector, &xPlayerMusicSearchWidget::updateFilter, this, &xMainMusicWidget::updateSearchSelectorFilter);
    connect(searchSelector, &xPlayerMusicSearchWidget::clearFilter, this, &xMainMusicWidget::clearSearchSelectorFilter);
    // Connect main widget to music player
    connect(this, &xMainMusicWidget::queueTracks, musicPlayer, &xMusicPlayer::queueTracks);
    connect(this, &xMainMusicWidget::finishedQueueTracks, musicPlayer, &xMusicPlayer::finishedQueueTracks);
    connect(this, &xMainMusicWidget::dequeueTrack, musicPlayer, &xMusicPlayer::dequeTrack);
    // Connect player widget to main widget
    connect(playerWidget, &xPlayerMusicWidget::clearQueue, this, &xMainMusicWidget::clearQueue);
    // Connect queue to main widget
    connect(queueList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::currentQueueTrackClicked);
    connect(queueList, &xPlayerListWidget::totalTime, this, &xMainMusicWidget::updateQueueTotalTime);
    // Connect shuffle mode.
    connect(queueShuffleCheck, &QCheckBox::clicked, musicPlayer, &xMusicPlayer::setShuffleMode);
    connect(queueShuffleCheck, &QCheckBox::clicked, queueList, &QListWidget::setDisabled);
    // Connect queue playlist menu.
    connect(queuePlaylistButton, &QPushButton::pressed, this, &xMainMusicWidget::playlistMenu);
    // Right click.
    connect(trackList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::selectSingleTrack);
    connect(queueList, &QListWidget::customContextMenuRequested, this, &xMainMusicWidget::currentQueueTrackRemoved);
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
    trackList->clearItems();
    // Not very efficient to use a filtered and the unfiltered list, but easier to read.
    for (const auto& artist : filterArtists(artists)) {
        artistList->addItemWidget(artist);
    }
    // Update database overlay for artists.
    updatePlayedArtists();
}

void xMainMusicWidget::scannedAlbums(const QStringList& albums) {
    // Clear album and track lists
    albumList->clearItems();
    trackList->clearItems();
    //unfilteredAlbums = albums;
    albumList->addItemWidgets(albums);
    // Update database overlay for albums.
    updatePlayedAlbums();
}

void xMainMusicWidget::scannedTracks(const std::list<xMusicFile*>& tracks) {
    // Clear only track list
    trackList->clearItems();
    for (const auto& track : tracks) {
        trackList->addItemWidget(track);
    }
    trackList->updateItems();
    // Update database overlay for tracks.
    updatePlayedTracks();
}

void xMainMusicWidget::scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,
                                             std::vector<xMusicFile*>>>& albumTracks) {
    for (const auto& albumTrack : albumTracks) {
        for (const auto& track : albumTrack.second) {
            // Add to the playlist (queue)
            queueList->addItemWidget(track, QString("%1 - %2").arg(artist, albumTrack.first));
        }
        emit queueTracks(artist, albumTrack.first, albumTrack.second);
    }
    emit finishedQueueTracks();
    // Update items.
    queueList->updateItems();
}

void xMainMusicWidget::scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString,
                                                        std::vector<xMusicFile*>>>>>& listTracks) {
    for (const auto& listTrack : listTracks) {
        for (const auto& albumTrack : listTrack.second) {
            for (const auto& track : albumTrack.second) {
                // Add to the playlist (queue)
                queueList->addItemWidget(track, QString("%1 - %2").arg(listTrack.first, albumTrack.first));
            }
            emit queueTracks(listTrack.first, albumTrack.first, albumTrack.second);
        }
    }
    emit finishedQueueTracks();
    // Update items.
    queueList->updateItems();
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
        emit scanAllAlbumsForArtist(artistList->itemWidget(artist)->text(), musicLibraryFilter);
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
        emit finishedQueueTracks();
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
        // Scan for all filtered artists.
        for (auto i = 0; i < artistList->count(); ++i) {
            listArtists.push_back(artistList->itemWidget(i)->text());
        }
        // Trigger scanAllAlbumsForListArtist with given list of artists.
        emit scanAllAlbumsForListArtists(listArtists, musicLibraryFilter);
    }
}

void xMainMusicWidget::selectAlbumSelectors(const QStringList& match, const QStringList& notMatch) {
    // We need to the matching selectors since search may add to the album matches.
    albumSelectorMatch = match;
    albumSelectorNotMatch = notMatch;
    musicLibraryFilter.setAlbumMatch(albumSelectorMatch, albumSelectorNotMatch);
    musicLibraryFilter.addSearchMatch(searchSelector->getMatch());
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

void xMainMusicWidget::clearAlbumSelectors() {
    musicLibraryFilter.clearMatch();
    musicLibraryFilter.addSearchMatch(searchSelector->getMatch());
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::clearSearchSelectorFilter() {
    musicLibraryFilter.clearMatch();
    musicLibraryFilter.addSearchMatch(searchSelector->getMatch());
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::updateSearchSelectorFilter(const std::tuple<QString,QString,QString>& match) {
    musicLibraryFilter.setAlbumMatch(albumSelectorMatch, albumSelectorNotMatch);
    musicLibraryFilter.addSearchMatch(match);
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

void xMainMusicWidget::currentQueueTrackClicked(QListWidgetItem* trackItem) {
    auto track = queueList->row(trackItem);
    if ((track >= 0) && (track< queueList->count())) {
        currentQueueTrack(track);
        musicPlayer->play(track);
    }
}

void xMainMusicWidget::currentQueueTrackRemoved(const QPoint& point) {
    // Currently unused
    Q_UNUSED(point)
    // Retrieve currently selected element
    auto track = queueList->currentRow();
    if ((track >= 0) && (track< queueList->count())) {
        queueList->takeItemWidget(track);
        emit dequeueTrack(track);
    }
}

void xMainMusicWidget::clearQueue() {
    // Clear playlist (queue).
    queueList->clearItems();
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

void xMainMusicWidget::updatedMusicLibraryAlbumSelectors() {
    albumSelectorList->setSelectors(xPlayerConfiguration::configuration()->getMusicLibraryAlbumSelectorList());
    // Clear match and not match selectors.
    musicLibraryFilter.clearMatch();
    // Add search matches.
    musicLibraryFilter.addSearchMatch(searchSelector->getMatch());
    // Rescan.
    emit scan(musicLibraryFilter);
}

void xMainMusicWidget::selectSingleTrack(const QPoint& point) {
    // Currently unused
    Q_UNUSED(point)
    // Retrieve currently selected element
    auto track = trackList->currentRow();
    if ((track >= 0) && (track< trackList->count())) {
        // Retrieve selected artist and album name.
        auto artistName = artistList->currentItemWidget()->text();
        auto albumName = albumList->currentItemWidget()->text();
        // Retrieve only selected track
        std::vector<xMusicFile*> trackObjects;
        QString trackName = trackList->itemWidget(track)->text();
        auto trackObject = musicLibrary->getMusicFile(artistName, albumName, trackName);
        if (trackObject == nullptr) {
            qCritical() << "xMainMusicWidget: unable to find music file object in library for "
                        << artistName+"/"+albumName+"/"+trackName;
            return;
        }
        trackObjects.push_back(trackObject);
        // Add to the playlist (queue)
        queueList->addItemWidget(trackObject,QString("%1 - %2").arg(artistName, albumName));
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackObjects);
        // Signal finish adding tracks.
        emit finishedQueueTracks();
        // Update items.
        queueList->updateItems();
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
    for (auto i = 0; i < trackList->count(); ++i) {
        auto trackItem = trackList->itemWidget(i);
        auto track = trackItem->text();
        // Clear icon and tooltip.
        trackItem->removeIcon();
        trackItem->removeToolTip();
        for (auto playedMusicTrack = playedMusicTracks.begin(); playedMusicTrack != playedMusicTracks.end(); ++playedMusicTrack) {
            // Update icon and tooltip if track already played.
            if (std::get<0>(*playedMusicTrack) == track) {
                trackItem->setIcon(":images/xplay-star.svg");
                // Adjust tooltip to play count "once" vs "x times".
                auto playCount = std::get<1>(*playedMusicTrack);
                if (playCount > 1) {
                    trackItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                            arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(std::get<2>(*playedMusicTrack))).toString(Qt::DefaultLocaleLongDate)));
                } else {
                    trackItem->addToolTip(QString(tr("played once, last time on %1")).
                            arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(std::get<2>(*playedMusicTrack))).toString(Qt::DefaultLocaleLongDate)));
                }
                // Remove element to speed up search in the next iteration.
                playedMusicTracks.erase(playedMusicTrack);
                // End update if no more tracks need to be marked.
                if (playedMusicTracks.isEmpty()) {
                    return;
                }
                // Break loop and move on to the next movie item.
                break;
            }
        }
    }
}

void xMainMusicWidget::updatePlayedTrack(const QString& artist, const QString& album,
                                         const QString& track, int playCount, quint64 timeStamp) {
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
                    arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timeStamp)).toString(Qt::DefaultLocaleLongDate)));
        } else {
            trackPlayedItem->addToolTip(QString(tr("played once, last time on %1")).
                    arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timeStamp)).toString(Qt::DefaultLocaleLongDate)));
        }
    }
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

void xMainMusicWidget::playlistMenu() {
    auto playlistNames = xPlayerDatabase::database()->getMusicPlaylists();
    auto playlistDialog = new xPlayerPlaylistDialog(playlistNames, this);
    connect(playlistDialog, &xPlayerPlaylistDialog::savePlaylist, musicPlayer, &xMusicPlayer::saveQueueToPlaylist);
    connect(playlistDialog, &xPlayerPlaylistDialog::openPlaylist, musicPlayer, &xMusicPlayer::loadQueueFromPlaylist);
    connect(playlistDialog, &xPlayerPlaylistDialog::removePlaylist,
            [=](const QString& name) { xPlayerDatabase::database()->removeMusicPlaylist(name); });
    playlistDialog->exec();
}
