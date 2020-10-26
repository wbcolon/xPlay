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
#include "xMainWidget.h"
#include "xPlayerWidget.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QListWidget>
#include <QListView>
#include <QFontMetrics>
#include <QApplication>

// Fuction addGroupBox has to be defined before the constructor due to the auto return.
auto xMainWidget::addGroupBox(const QString& boxLabel) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, this);
    auto list = new QListWidget(groupBox);
    auto boxLayout = new QHBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}

xMainWidget::xMainWidget(xMusicPlayer* player, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicPlayer(player),
        playedTrack(0) {
    // Create and group boxes with embedded list widgets.
    auto [artistBox, artistList_] = addGroupBox(tr("Artists"));
    auto [albumBox, albumList_] = addGroupBox(tr("Albums"));
    auto [trackBox, trackList_] = addGroupBox(tr("Tracks"));
    auto [queueBox, queueList_] = addGroupBox(tr("Queue"));
    auto [artistSelectorBox, artistSelectorList_] = addGroupBox(tr("ArtistSelector"));
    auto playerBox = new QGroupBox(tr("Player"), this);
    auto playerLayout = new QHBoxLayout();
    playerWidget = new xPlayerWidget(musicPlayer, this);
    playerLayout->addWidget(playerWidget);
    playerBox->setLayout(playerLayout);
    // Sort entries in artist/album/track
    artistList = artistList_;
    artistList->setSortingEnabled(true);
    albumList = albumList_;
    albumList->setSortingEnabled(true);
    trackList = trackList_;
    trackList->setSortingEnabled(true);
    trackList->setContextMenuPolicy(Qt::CustomContextMenu);
    queueList = queueList_;
    queueList->setContextMenuPolicy(Qt::CustomContextMenu);
    // Setup artistSelector as horizontal list widget with no wrapping. Fix it's height.
    artistSelectorList = artistSelectorList_; // requires since we need to use the member variable;
    artistSelectorList->setViewMode(QListView::IconMode);
    artistSelectorList->setWrapping(false);
    artistSelectorList->setFixedHeight(QFontMetrics(QApplication::font()).height()*3/2);
    artistSelectorBox->setFixedHeight(artistSelectorBox->sizeHint().height());
    // Setup layout for main widget.
    auto mainWidgetLayout = new QGridLayout(this);
    mainWidgetLayout->setSpacing(10);
    mainWidgetLayout->addWidget(playerBox, 0, 0, 1, 3);
    mainWidgetLayout->addWidget(artistBox, 1, 0, 8, 1);
    mainWidgetLayout->addWidget(albumBox, 1, 1, 7, 1);
    mainWidgetLayout->addWidget(trackBox, 1, 2, 7, 1);
    mainWidgetLayout->addWidget(artistSelectorBox, 8, 1, 1, 2);
    mainWidgetLayout->addWidget(queueBox, 0, 3, 9, 1);
    // Connect artist, album, track and selector widgets
    connect(artistList, &QListWidget::currentRowChanged, this, &xMainWidget::selectArtist);
    connect(albumList, &QListWidget::currentRowChanged, this, &xMainWidget::selectAlbum);
    connect(trackList, &QListWidget::itemDoubleClicked, this, &xMainWidget::selectTrack);
    connect(artistSelectorList, &QListWidget::currentRowChanged, this, &xMainWidget::selectArtistSelector);
    // Connect main widget to music player
    connect(this, &xMainWidget::queueTracks, musicPlayer, &xMusicPlayer::queueTracks);
    connect(this, &xMainWidget::dequeueTrack, musicPlayer, &xMusicPlayer::dequeTrack);
    // Connect player widget to main widget
    connect(playerWidget, &xPlayerWidget::clearQueue, this, &xMainWidget::clearQueue);
    // Connect queue to main widget
    connect(playerWidget, &xPlayerWidget::currentQueueTrack, this, &xMainWidget::currentQueueTrack);
    connect(queueList, &QListWidget::itemDoubleClicked, this, &xMainWidget::currentQueueTrackClicked);
    // Right click.
    connect(trackList, &QListWidget::customContextMenuRequested, this, &xMainWidget::selectSingleTrack);
    connect(queueList, &QListWidget::customContextMenuRequested, this, &xMainWidget::currentQueueTrackRemoved);
    // Connect music player to main widget for queue update.
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xMainWidget::currentState);
}

void xMainWidget::connectRotel(const QString& address, int port) {
    // Tell the player widget to connect.
    playerWidget->connectRotel(address, port);
}

void xMainWidget::scannedArtists(const QStringList& artists) {
    std::set<QString> selectors;
    unfilteredArtists = artists;
    // Clear artist, album and track lists
    artistList->clear();
    albumList->clear();
    trackList->clear();
    // Not very efficient to use a filtered and the unfiltered list, but easier to read.
    for (const auto& artist : filterArtists(artists)) {
        artistList->addItem(artist);
    }
    // Use unfiltered list for selectors update
    for (const auto& artist : artists) {
        selectors.insert(artist.left(1));
    }
    // Update the selector based upon the added artists
    scannedArtistsSelectors(selectors);
}

void xMainWidget::scannedAlbums(const QStringList& albums) {
    // Clear album and track lists
    albumList->clear();
    trackList->clear();
    for (const auto& album : albums) {
        albumList->addItem(album);
    }
}

void xMainWidget::scannedTracks(const QStringList& tracks) {
   // Clear only track list
   trackList->clear();
   for (const auto& track : tracks) {
       trackList->addItem(track);
   }
}

void xMainWidget::scannedArtistsSelectors(const std::set<QString> &selectors) {
    // Update artist selectors list widget.
    artistSelectorList->clear();
    artistSelectorList->addItem(tr("none"));
    for (const auto& as : selectors) {
        artistSelectorList->addItem(as);
    }
}

QStringList xMainWidget::filterArtists(const QStringList& artists) {
    // Check if a selector is selected.
    if (artistSelectorList->currentItem()) {
        QStringList filtered;
        QString selected = artistSelectorList->currentItem()->text();
        // Do not filter if we have selector "none" selected.
        if (selected.startsWith(tr("none"))) {
            return artists;
        }
        // Go through list of artists and only add the ones to the filtered
        // list that start with the selector character.
        for (const auto& artist : artists) {
            if (artist.startsWith(selected)) {
                filtered.push_back(artist);
            }
        }
        // Return filtered list of artists.
        return filtered;
    }
    return artists;
}

void xMainWidget::selectArtist(int artist) {
    // Check if artist index is valid.
    if ((artist >= 0) && (artist < artistList->count())) {
        // Retrieve selected artist name and trigger scanForArtist
        auto artistName = artistList->item(artist)->text();
        emit scanForArtist(artistName);
    }
}

void xMainWidget::selectAlbum(int album) {
    // Check if album index is valid.
    if ((album >= 0) && (album < albumList->count())) {
        // Retrieve selected artist and album name and
        // trigger scanForArtistAndAlbum
        auto artistName = artistList->currentItem()->text();
        auto albumName = albumList->item(album)->text();
        emit scanForArtistAndAlbum(artistName, albumName);
    }
}

void xMainWidget::selectTrack(QListWidgetItem* trackItem) {
    // Retrieve index for the selected item and check if it's valid.
    auto track = trackList->row(trackItem);
    if ((track >= 0) && (track< trackList->count())) {
        // Retrieve selected artist and album name.
        auto artistName = artistList->currentItem()->text();
        auto albumName = albumList->currentItem()->text();
        // Retrieve all track names from the selected index to
        // the end of the album.
        std::vector<QString> trackNames;
        QString trackName;
        for (auto i = track; i < trackList->count(); ++i) {
            trackName = trackList->item(i)->text();
            trackNames.push_back(trackName);
            // Add to the playlist (queue)
            queueList->addItem(trackName);
        }
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackNames);
    }
}

void xMainWidget::selectArtistSelector(int selector) {
    // Check if index is valid.
    if ((selector >= 0) && (selector < artistSelectorList->count())) {
        // Call with stored list in order to update artist filtering.
        scannedArtists(unfilteredArtists);
    }
}

void xMainWidget::currentState(xMusicPlayer::State state) {
    // Update the icon for the played track based on the state of the music player.
    auto currentTrack = queueList->item(playedTrack);
    if (!currentTrack) {
        return;
    }
    switch (state) {
        case xMusicPlayer::PlayingState: {
            currentTrack->setIcon(QIcon(":/images/xplay-play.svg"));
        } break;
        case xMusicPlayer::PauseState: {
            currentTrack->setIcon(QIcon(":/images/xplay-pause.svg"));
        } break;
        case xMusicPlayer::StopState: {
            currentTrack->setIcon(QIcon(":/images/xplay-stop.svg"));
        } break;
        default: {
            currentTrack->setIcon(QIcon());
        } break;
    }
}

void xMainWidget::currentQueueTrack(int index) {
    queueList->setCurrentRow(index);
    // Remove play icon from old track
    auto oldPlayedItem = queueList->item(playedTrack);
    if (oldPlayedItem) {
        oldPlayedItem->setIcon(QIcon());
    }
    // Update played track.
    playedTrack = index;
    // Set play icon only for currently played one.
    auto playedItem = queueList->item(playedTrack);
    if (playedItem) {
        playedItem->setIcon(QIcon(":/images/xplay-play.svg"));
    }
}

void xMainWidget::currentQueueTrackClicked(QListWidgetItem* trackItem) {
    auto track = queueList->row(trackItem);
    if ((track >= 0) && (track< queueList->count())) {
        currentQueueTrack(track);
        emit musicPlayer->play(track);
    }
}

void xMainWidget::currentQueueTrackRemoved(const QPoint& point) {
    // Currently unused
    Q_UNUSED(point)
    // Retrieve currently selected element
    auto track = queueList->currentRow();
    if ((track >= 0) && (track< queueList->count())) {
        queueList->takeItem(track);
        emit dequeueTrack(track);
    }
}

void xMainWidget::clearQueue() {
    // Clear playlist (queue).
    queueList->clear();
}

void xMainWidget::selectSingleTrack(const QPoint& point) {
    // Currently unused
    Q_UNUSED(point)
    // Retrieve currently selected element
    auto track = trackList->currentRow();
    if ((track >= 0) && (track< trackList->count())) {
        // Retrieve selected artist and album name.
        auto artistName = artistList->currentItem()->text();
        auto albumName = albumList->currentItem()->text();
        // Retrieve only selected track
        std::vector<QString> trackNames;
        QString trackName = trackList->item(track)->text();
        trackNames.push_back(trackName);
        // Add to the playlist (queue)
        queueList->addItem(trackName);
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackNames);
    }
}
