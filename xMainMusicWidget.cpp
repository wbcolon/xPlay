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
#include <QCheckBox>
#include <random>

// Function addGroupBox has to be defined before the constructor due to the auto return.
auto xMainMusicWidget::addGroupBox(const QString& boxLabel) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, this);
    groupBox->setFlat(xPlayerUseFlatGroupBox);
    auto list = new QListWidget(groupBox);
    auto boxLayout = new QVBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}

xMainMusicWidget::xMainMusicWidget(xMusicPlayer* player, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicPlayer(player),
        albumSelectorMatch(),
        albumSelectorNotMatch(),
        playedTrack(0),
        useDatabaseMusicOverlay(true),
        databaseCutOff(0) {
    // Create and group boxes with embedded list widgets.
    auto [artistBox, artistList_] = addGroupBox(tr("Artists"));
    auto [albumBox, albumList_] = addGroupBox(tr("Albums"));
    auto [trackBox, trackList_] = addGroupBox(tr("Tracks"));
    auto [artistSelectorBox, artistSelectorList_] = addGroupBox(tr("Artist Selector"));
    playerWidget = new xPlayerMusicWidget(musicPlayer, this);
    // Sort entries in artist/album/track
    artistList = artistList_;
    artistList->setSortingEnabled(false);
    // auto artistFont = artistList->font();
    // artistFont.setPointSizeF(artistFont.pointSizeF()*1.3);
    // artistList->setFont(artistFont);
    albumList = albumList_;
    albumList->setSortingEnabled(true);
    trackList = trackList_;
    trackList->setSortingEnabled(true);
    trackList->setContextMenuPolicy(Qt::CustomContextMenu);
    // Queue list.
    auto queueBox = new QGroupBox(tr("Queue"), this);
    queueBox->setFlat(xPlayerUseFlatGroupBox);
    auto queueBoxLayout = new xPlayerLayout();
    queueList = new QListWidget(queueBox);
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
    // Setup artistSelector as horizontal list widget with no wrapping. Fix it's height.
    artistSelectorList = artistSelectorList_; // requires since we need to use the member variable;
    artistSelectorList->setViewMode(QListView::IconMode);
    artistSelectorList->setWrapping(false);
    artistSelectorList->setFixedHeight(static_cast<int>(QFontMetrics(QApplication::font()).height()*xPlayerSelectorHeightFontFactor));
    artistSelectorBox->setFixedHeight(artistSelectorBox->sizeHint().height());
    // Setup album selector.
    auto albumSelectorBox = new QGroupBox(tr("Album Selector"), this);
    albumSelectorBox->setFlat(xPlayerUseFlatGroupBox);
    auto albumSelectorLayout = new QVBoxLayout();
    // Create album selector widget.
    albumSelectorList = new xPlayerSelectorWidget(albumSelectorBox);
    albumSelectorLayout->addWidget(albumSelectorList);
    albumSelectorBox->setLayout(albumSelectorLayout);
    // Set heigh of album selector to height af artist selector.
    albumSelectorBox->setFixedHeight(artistSelectorBox->sizeHint().height());
    // Setup layout for main widget.
    auto mainWidgetLayout = new xPlayerLayout(this);
    mainWidgetLayout->setSpacing(xPlayerLayout::SmallSpace);
    mainWidgetLayout->addWidget(playerWidget, 0, 0, 1, 12);
    mainWidgetLayout->addWidget(artistBox, 1, 0, 7, 3);
    mainWidgetLayout->addWidget(albumBox, 1, 3, 7, 5);
    mainWidgetLayout->addWidget(trackBox, 1, 8, 7, 4);
    mainWidgetLayout->addWidget(artistSelectorBox, 8, 0, 1, 8);
    mainWidgetLayout->addWidget(albumSelectorBox, 8, 8, 1, 4);
    mainWidgetLayout->addWidget(queueBox, 0, 12, 9, 4);
    // Connect artist, album, track and selector widgets
    connect(artistList, &QListWidget::currentRowChanged, this, &xMainMusicWidget::selectArtist);
    connect(artistList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::queueArtist);
    connect(albumList, &QListWidget::currentRowChanged, this, &xMainMusicWidget::selectAlbum);
    connect(albumList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::queueAlbum);
    connect(trackList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::selectTrack);
    connect(artistSelectorList, &QListWidget::currentRowChanged, this, &xMainMusicWidget::selectArtistSelector);
    connect(artistSelectorList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::queueArtistSelector);
    connect(albumSelectorList, &xPlayerSelectorWidget::updatedSelectors, this, &xMainMusicWidget::selectAlbumSelector);
    // Connect main widget to music player
    connect(this, &xMainMusicWidget::queueTracks, musicPlayer, &xMusicPlayer::queueTracks);
    connect(this, &xMainMusicWidget::finishedQueueTracks, musicPlayer, &xMusicPlayer::finishedQueueTracks);
    connect(this, &xMainMusicWidget::dequeueTrack, musicPlayer, &xMusicPlayer::dequeTrack);
    // Connect player widget to main widget
    connect(playerWidget, &xPlayerMusicWidget::clearQueue, this, &xMainMusicWidget::clearQueue);
    // Connect queue to main widget
    connect(queueList, &QListWidget::itemDoubleClicked, this, &xMainMusicWidget::currentQueueTrackClicked);
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
    artistList->clear();
    albumList->clear();
    trackList->clear();
    // Not very efficient to use a filtered and the unfiltered list, but easier to read.
    for (const auto& artist : filterArtists(artists)) {
        artistList->addItem(artist);
    }
    // Update database overlay for artists.
    updatePlayedArtists();
}

void xMainMusicWidget::scannedAlbums(const QStringList& albums) {
    // Clear album and track lists
    albumList->clear();
    trackList->clear();
    unfilteredAlbums = albums;
    for (const auto& album : filterAlbums(albums)) {
        albumList->addItem(album);
    }
    // Update database overlay for albums.
    updatePlayedAlbums();
}

void xMainMusicWidget::scannedTracks(const QStringList& tracks) {
    // Clear only track list
    trackList->clear();
    for (const auto& track : tracks) {
        trackList->addItem(track);
    }
    // Update database overlay for tracks.
    updatePlayedTracks();
}

void xMainMusicWidget::scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,std::vector<QString>>>& albumTracks) {
    for (const auto& albumTrack : albumTracks) {
        // Skip over the albums that do not match our filter.
        if (!filterAlbum(albumTrack.first)) {
            continue;
        }
        for (const auto& track : albumTrack.second) {
            // Add to the playlist (queue)
            auto queueItem = new QListWidgetItem(track, queueList);
            // Add tooltip.
            queueItem->setToolTip(QString("%1 - %2").arg(artist).arg(albumTrack.first));
            queueList->addItem(queueItem);
        }
        emit queueTracks(artist, albumTrack.first, albumTrack.second);
    }
    emit finishedQueueTracks();
}

void xMainMusicWidget::scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString, std::vector<QString>>>>>& listTracks) {
    for (const auto& listTrack : listTracks) {
        for (const auto& albumTrack : listTrack.second) {
            // Skip over the albums that do not match our filter.
            if (!filterAlbum(albumTrack.first)) {
                continue;
            }
            for (const auto& track : albumTrack.second) {
                // Add to the playlist (queue)
                auto queueItem = new QListWidgetItem(track, queueList);
                // Add tooltip.
                queueItem->setToolTip(QString("%1 - %2").arg(listTrack.first).arg(albumTrack.first));
                queueList->addItem(queueItem);
            }
            emit queueTracks(listTrack.first, albumTrack.first, albumTrack.second);
        }
    }
    emit finishedQueueTracks();
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

QStringList xMainMusicWidget::filterAlbums(const QStringList& albums) {
    QStringList filtered;
    for (const auto& album : albums) {
        // Only add albums that pass our filters.
        if (filterAlbum(album)) {
            filtered.push_back(album);
        }
    }
    return filtered;
}

bool xMainMusicWidget::filterAlbum(const QString& album) {
    // Go through the not match selectors.
    for (const auto& notMatch : albumSelectorNotMatch) {
        if (album.contains(notMatch)) {
            return false;
        }
    }
    // Accept if we do not have any match selectors.
    if (albumSelectorMatch.isEmpty()) {
        return true;
    } else {
        // Go through the match selectors.
        for (const auto& match : albumSelectorMatch) {
            // Only has to match one.
            if (album.contains(match)) {
                return true;
            }
        }
    }
    return false;
}


void xMainMusicWidget::selectArtist(int artist) {
    // Check if artist index is valid.
    if ((artist >= 0) && (artist < artistList->count())) {
        // Retrieve selected artist name and trigger scanForArtist
        auto artistName = artistList->item(artist)->text();
        emit scanForArtist(artistName);
    }
}

void xMainMusicWidget::queueArtist(QListWidgetItem *artistItem) {
    // Retrieve index for the selected item and check if it's valid.
    auto artist = artistList->row(artistItem);
    if ((artist >= 0) && (artist < artistList->count())) {
        // Retrieve selected artist name and trigger scanAllAlbumsForArtist
        emit scanAllAlbumsForArtist(artistItem->text());
    }
}

void xMainMusicWidget::selectAlbum(int album) {
    // Check if album index is valid.
    if ((album >= 0) && (album < albumList->count())) {
        // Retrieve selected artist and album name and
        // trigger scanForArtistAndAlbum
        auto artistName = artistList->currentItem()->text();
        auto albumName = albumList->item(album)->text();
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
            auto queueItem = new QListWidgetItem(trackName, queueList);
            // Add tooltip.
            queueItem->setToolTip(QString("%1 - %2").arg(artistName).arg(albumName));
            queueList->addItem(queueItem);
        }
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackNames);
        // Finished queueing tracks.
        emit finishedQueueTracks();
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
        QList<QString> listArtists;
        // Call with stored list in order to update artist filtering.
        updateScannedArtists(unfilteredArtists);
        // Scan for all filtered artists.
        for (auto i = 0; i < artistList->count(); ++i) {
            listArtists.push_back(artistList->item(i)->text());
        }
        // Tigger scanAllAlbumsForListArtist with given list of artists.
        emit scanAllAlbumsForListArtists(listArtists);
    }
}

void xMainMusicWidget::selectAlbumSelector(const QStringList& match, const QStringList& notMatch) {
    albumSelectorMatch = match;
    albumSelectorNotMatch = notMatch;
    scannedAlbums(unfilteredAlbums);
}

void xMainMusicWidget::currentState(xMusicPlayer::State state) {
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
}

void xMainMusicWidget::currentQueueTrack(int index) {
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
        queueList->takeItem(track);
        emit dequeueTrack(track);
    }
}

void xMainMusicWidget::clearQueue() {
    // Clear playlist (queue).
    queueList->clear();
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
            artistList->item(i)->setIcon(QIcon());
        }
        // Clear album list.
        for (auto i = 0; i < albumList->count(); ++i) {
            albumList->item(i)->setIcon(QIcon());
        }
        // Clear track list.
        for (auto i = 0; i < trackList->count(); ++i) {
            trackList->item(i)->setIcon(QIcon());
            trackList->item(i)->setToolTip(QString());
        }
    }
}

void xMainMusicWidget::updatedMusicLibraryAlbumSelectors() {
    albumSelectorList->setSelectors(xPlayerConfiguration::configuration()->getMusicLibraryAlbumSelectorList());
    // Clear match and not match selectors.
    albumSelectorMatch.clear();
    albumSelectorNotMatch.clear();
    // If we have a list of albums, then we re-filter.
    if (!unfilteredAlbums.isEmpty()) {
        scannedAlbums(unfilteredAlbums);
    }
}

void xMainMusicWidget::selectSingleTrack(const QPoint& point) {
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
        auto queueItem = new QListWidgetItem(trackName, queueList);
        // Add tooltip.
        queueItem->setToolTip(QString("%1 - %2").arg(artistName).arg(albumName));
        queueList->addItem(queueItem);
        // Signal the set tracks to be queued by the music player.
        emit queueTracks(artistName, albumName, trackNames);
        // Signal finish adding tracks.
        emit finishedQueueTracks();
    }
}
void xMainMusicWidget::updatePlayedArtists() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMusicOverlay) {
        return;
    }
    auto playedArtists = xPlayerDatabase::database()->getPlayedArtists(databaseCutOff);
    for (auto i = 0; i < artistList->count(); ++i) {
        auto artistItem = artistList->item(i);
        auto artist = artistItem->text();
        // Clear icon.
        artistItem->setIcon(QIcon());
        for (const auto& playedArtist : playedArtists) {
            // Update icon and tooltip if movie already played.
            if (playedArtist == artist) {
                artistItem->setIcon(QIcon(":images/xplay-star.svg"));
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
        auto albumItem = albumList->item(i);
        auto album = albumItem->text();
        // Clear icon and tooltip.
        albumItem->setIcon(QIcon());
        for (const auto& playedAlbum : playedAlbums) {
            // Update icon and tooltip if movie already played.
            if (playedAlbum == album) {
                albumItem->setIcon(QIcon(":images/xplay-star.svg"));
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
    qDebug() << "ARTIST: " << artistItem;
    // No artist or album currently selected. We do not need to update.
    if ((!artistItem) || (!albumItem)) {
        return;
    }
    auto artist = artistItem->text();
    auto album = albumItem->text();
    auto playedMusicTracks = xPlayerDatabase::database()->getPlayedTracks(artist, album, databaseCutOff);
    for (auto i = 0; i < trackList->count(); ++i) {
        auto trackItem = trackList->item(i);
        auto track = trackItem->text();
        // Clear icon and tooltip.
        trackItem->setIcon(QIcon());
        trackItem->setToolTip(QString());
        for (auto playedMusicTrack = playedMusicTracks.begin(); playedMusicTrack != playedMusicTracks.end(); ++playedMusicTrack) {
            // Update icon and tooltip if track already played.
            if (std::get<0>(*playedMusicTrack) == track) {
                trackItem->setIcon(QIcon(":images/xplay-star.svg"));
                // Adjust tooltip to play count "once" vs "x times".
                auto playCount = std::get<1>(*playedMusicTrack);
                if (playCount > 1) {
                    trackItem->setToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                            arg(QDateTime::fromMSecsSinceEpoch(std::get<2>(*playedMusicTrack)).toString(Qt::DefaultLocaleLongDate)));
                } else {
                    trackItem->setToolTip(QString(tr("played once, last time on %1")).
                            arg(QDateTime::fromMSecsSinceEpoch(std::get<2>(*playedMusicTrack)).toString(Qt::DefaultLocaleLongDate)));
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
    auto artistItem = artistList->currentItem();
    // Update the artists.
    auto artistPlayedItems = artistList->findItems(artist, Qt::MatchExactly);
    for (auto& artistPlayedItem : artistPlayedItems) {
        artistPlayedItem->setIcon(QIcon(":images/xplay-star.svg"));
    }
    // If no artist selected or the artist does not match the selected
    // artists then we do not need to update the albums.
    if ((!artistItem) || (artistItem->text() != artist)) {
        return;
    }
    auto albumItem = albumList->currentItem();
    // Update the albums.
    auto albumPlayedItems = albumList->findItems(album, Qt::MatchExactly);
    for (auto& albumPlayedItem : albumPlayedItems) {
        albumPlayedItem->setIcon(QIcon(":images/xplay-star.svg"));
    }
    // If no album selected or the album does not match the selected
    // album then we do not need to update the tracks.
    if ((!albumItem) || (albumItem->text() != album)) {
        return;
    }
    auto trackPlayedItems = trackList->findItems(track, Qt::MatchExactly);
    for (auto& trackPlayedItem : trackPlayedItems) {
        trackPlayedItem->setIcon(QIcon(":images/xplay-star.svg"));
        if (playCount > 1) {
            trackPlayedItem->setToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::DefaultLocaleLongDate)));
        } else {
            trackPlayedItem->setToolTip(QString(tr("played once, last time on %1")).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::DefaultLocaleLongDate)));
        }
    }
}

void xMainMusicWidget::playlist(const std::vector<std::tuple<QString,QString,QString>>& entries) {
    // Clear the player widget.
    playerWidget->clear();
    // Clear queue.
    queueList->clear();
    for (const auto& entry : entries) {
        // Add to the playlist (queue)
        auto queueItem = new QListWidgetItem(std::get<2>(entry), queueList);
        // Add tooltip.
        queueItem->setToolTip(QString("%1 - %2").arg(std::get<0>(entry)).arg(std::get<1>(entry)));
        queueList->addItem(queueItem);
    }
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
