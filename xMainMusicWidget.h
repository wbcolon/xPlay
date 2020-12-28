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
#ifndef __XMAINWIDGET_H__
#define __XMAINWIDGET_H__

#include "xMusicPlayer.h"
#include "xPlayerMusicWidget.h"

#include <QListWidget>
#include <QString>

#include <vector>
#include <list>
#include <set>

class xMainMusicWidget: public QWidget {
    Q_OBJECT

public:
    explicit xMainMusicWidget(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xMainMusicWidget() override = default;
    /**
     * Perform initial commands required when switching to this view.
     */
    void initializeView();
    /**
     * Clear artist, album, tracks and queue.
     */
    void clear();

signals:
    /*
     * Signals used for communication with music player widget and the music player.
     * The corresponding slots are implemented in the music player widget or music
     * player classes.
     */
    /**
     * Signal emitted to notify main widget to update the window title.
     *
     * @param title the new window title as string.
     */
    void showWindowTitle(const QString& title);
    /**
     * Signal emitted to notify main widget to show/hide the menu bar.
     *
     * @param menu show menu bar if true, hide otherwise.
     */
    void showMenuBar(bool menu);
    /**
     * Signal to scan albums for an artist.
     *
     * @param artist name of the artist to scan for.
     */
    void scanForArtist(const QString& artist);
    /**
     * Signal to scan tracks for an artist/album
     *
     * @param artist the artist name for the scan.
     * @param album the album name for the scan.
     */
    void scanForArtistAndAlbum(const QString& artist, const QString& album);
    /**
     * Signal to scan all albums and tracks for the given artist.
     *
     * @param artist the artist name for which we can all albums and tracks.
     */
    void scanAllAlbumsForArtist(const QString& artist);
    /**
     * Signal a set of tracks to be queued in the playlist.
     *
     * @param artist the artist name for the tracks.
     * @param album the album name for the tracks.
     * @param tracks ordered vector of track names.
     */
    void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks);
    /**
     * Signal the dequeue of a single track
     *
     * @param index position of the track in the playlist.
     */
    void dequeueTrack(int index);
    /**
     * Signal that a specific track of the playlist will be played.
     *
     * @param index the position of the track in the playlist.
     */
    void play(int index);

public slots:
    /**
     * Receive the result of music library scan for artists.
     *
     * Update the QListWidget for list of artists. The widget itself sorts
     * all added entries. Clear the QListWidget's for the list of albums and
     * list of tracks.
     *
     * @param artists unordered list of artist names.
     */
    void scannedArtists(const QStringList& artists);
    /**
     * Receive the result of the album scan for a given artist.
     *
     * Update the QListWidget for the list of albums. The widget itself sorts
     * all added entries. Clear the QListWidget for the list of tracks.
     *
     * @param albums unordered list of album names.
     */
    void scannedAlbums(const QStringList& albums);
    /**
     * Receive the result of the track scan for a given artist/album
     *
     * Update the QListWidget for the list of tracks. The widget itself sorts
     * all added entries.
     *
     * @param tracks unordered list of track names.
     */
    void scannedTracks(const QStringList& tracks);
    /**
     * Receive the result of the all album and track scan for a given artist
     *
     * @param albumTracks sorted list of pairs of album/list of track names to be queued.
     */
    void scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,std::vector<QString>>>& albumTracks);

private slots:
    /**
     * Scan for albums of the selected artist.
     *
     * Function is triggered whenever a rows in the list of artists is
     * selected. The function then retrieves the artist name and signals
     * a scanForArtists.
     *
     * @param artist index of the artist in the QListWidget.
     */
    void selectArtist(int artist);
    /**
     * Queue all albums for the currently selected artists.
     *
     * Function is triggered whenever an artist out of the list of artists
     * is double-clicked. The artist name is retrieved and all its albums
     * are queued.
     *
     * @param artistItem pointer to the currently selected row.
     */
    void queueArtist(QListWidgetItem* artistItem);
    /**
     * Scan for tracks of the selected artist/album.
     *
     * Function is triggered whenever a rows in the list of albums is
     * selected. The function then retrieves the selected artist name and
     * the selected album name. It then signals a scanForArtistAndAlbum.
     *
     * @param artist index of the artist in the QListWidget.
     */
    void selectAlbum(int album);
    /**
     * Append tracks to the playlist (queue).
     *
     * Function is triggered by a double click to an entry in the list of
     * tracks. The position of the selected track is determined and the track
     * and the following tracks will be added to the queue.
     *
     * @param trackItem pointer to the currently selected row.
     */
    void selectTrack(QListWidgetItem* trackItem);
    /**
     * Append single track to the playlist (queue)
     *
     * Function is triggered by a right click to an entry in the list of
     * tracks. Only the track currently selected (at this position) is added.
     * (@see selectTrack)
     *
     * @param point position of the right-click.
     */
    void selectSingleTrack(const QPoint& point);
    /**
     * Filter the list of artists.
     *
     * Function is triggered whenever an element out of the selectors is selected.
     * The selector is retrieved and used to filter the list of artists.
     *
     * @param selector position of the first character used to filter artists.
     */
    void selectArtistSelector(int selector);
    /**
     * Queue all albums for the currently all artists starting with the selector.
     *
     * Function is triggered whenever a selector out of the list of selectors
     * is double-clicked. All artists are retrieved and all their albums
     * are queued. A double-click on "none" will queue the entire library.
     *
     * @param selectorItem pointer to the currently selected row.
     */
    void queueArtistSelector(QListWidgetItem* selectorItem);
    /**
     * Update the player UI based on the player state.
     *
     * @param state the current state of the player.
     */
    void currentState(xMusicPlayer::State state);
    /**
     * Update the queue based on the currently played song.
     *
     * @param index of the track currently played.
     */
    void currentQueueTrack(int index);
    /**
     * Update the database, database overlay and queue based on the currently played song.
     *
     * @param index the position of the current track in the playlist
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track the name of the current track.
     * @param bitrate the bitrate in kb/sec.
     * @param sampleRate the sample rate in Hz.
     */
    void currentTrack(int index, const QString& artist, const QString& album, const QString& track,
                      int bitrate, int sampleRate, int bitsPerSample);
    /**
     * Select the current track in the playlist (queue).
     *
     * @param trackItem pointer to the double-clicked item.
     */
    void currentQueueTrackClicked(QListWidgetItem* trackItem);
    /**
     *  Remove the current track from the playlist (queue).
     *
     * @param point position of the right-click.
     */
    void currentQueueTrackRemoved(const QPoint& point);
    /**
     * Clear the list of queued tracks.
     */
    void clearQueue();
    /**
     * Update the music database overlay on configuration changes.
     */
    void updatedDatabaseMusicOverlay();

private:
    /**
     * Update the database overlay for played artists.
     */
    void updatePlayedArtists();
    /**
     * Update the database overlay for played albums.
     */
    void updatePlayedAlbums();
    /**
     * Update the database overlay for played tracks and add tooltips.
     */
    void updatePlayedTracks();
    /**
     * Update the database overlay for currently played track and add tooltips.
     *
     * @param artist the artist for the currently played track.
     * @param album the album for the currently played track.
     * @param track the track number and name for the currently played track.
     * @param playCount the play count for the currently played track.
     * @param timeStamp the last played time stamp in milli seconds for the currently played track.
     */
    void updatePlayedTrack(const QString& artist, const QString& album, const QString& track, int playCount, quint64 timeStamp);
    /**
     * Update the list of artists.
     *
     * @param artists the list of all unfiltered artists.
     */
    void updateScannedArtists(const QStringList& artists);
    /**
     * Update the list of selectors (and add "none").
     *
     * @param selectors a set of characters (as QString) used for filtering artists.
     */
    void updateScannedArtistsSelectors(const std::set<QString>& selectors);
    /**
     * Filter the list of artists based on the selected selector.
     *
     * @param artists unfiltered list of artists.
     * @return filtered list of artists that start with the selector string.
     */
    [[nodiscard]] QStringList filterArtists(const QStringList& artists);
    /**
     * Helper function creating a QGroupBox with an QListWidget.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @return pair of pointer to the created QGroupBox and QListWidget.
     */
    auto addGroupBox(const QString& boxLabel);

    xMusicPlayer* musicPlayer;
    xPlayerMusicWidget* playerWidget;
    QListWidget* artistList;
    QListWidget* albumList;
    QListWidget* trackList;
    QListWidget* artistSelectorList;
    QListWidget* queueList;
    int playedTrack;
    bool useDatabaseMusicOverlay;
    quint64 databaseCutOff;
    /**
     * Store the current list of unfiltered artists for later filtering.
     */
    QStringList unfilteredArtists;
};

#endif
