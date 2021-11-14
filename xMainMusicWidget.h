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
#include "xMusicLibrary.h"
#include "xPlayerMusicWidget.h"
#include "xPlayerMusicSearchWidget.h"
#include "xPlayerMusicArtistSelectorWidget.h"
#include "xPlayerMusicAlbumSelectorWidget.h"
#include "xPlayerListWidget.h"
#include "xPlayerArtistInfo.h"
#include "xMusicDirectory.h"

#include <QGroupBox>
#include <QString>
#include <QThread>
#include <QStackedWidget>
#include <QToolButton>

#include <vector>
#include <list>
#include <set>
#include <QProgressBar>

class xMainMusicWidget: public QWidget {
    Q_OBJECT

public:
    explicit xMainMusicWidget(xMusicPlayer* player, xMusicLibrary* library,
                              QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
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
     * Signal to scan and filter for artists.
     *
     * @param filter the filter to be applied.
     */
    void scan(const xMusicLibraryFilter& filter);
    /**
     * Signal to scan and filter albums for an artist.
     *
     * @param artist the artist to scan for.
     * @param filter the filter to be applied.
     */
    void scanForArtist(const xMusicDirectory& artist, const xMusicLibraryFilter& filter);
    /**
     * Signal to scan tracks for an artist/album
     *
     * @param artist the artist for the scan.
     * @param album the album for the scan.
     */
    void scanForArtistAndAlbum(const xMusicDirectory& artist, const xMusicDirectory& album);
    /**
     * Signal to scan and filter all albums and tracks for the given artist.
     *
     * @param artist the artist name for which we can all albums and tracks.
     * @param filter the filter to be applied.
     */
    void scanAllAlbumsForArtist(const xMusicDirectory& artist, const xMusicLibraryFilter& filter);
    /**
     * Signal to scan and filter all albums and tracks for a given list of artists.
     *
     * @param listArtist the list of artists name for which we scan all albums and tracks.
     * @param filter the filter to be applied.
     */
    void scanAllAlbumsForListArtists(const std::list<xMusicDirectory>& listArtist, const xMusicLibraryFilter& filter);
    /**
     * Signal a set of tracks to be queued in the playlist.
     *
     * @param artist the artist name for the tracks.
     * @param album the album name for the tracks.
     * @param tracks ordered vector of track objects.
     */
    void queueTracks(const QString& artist, const QString& album, const std::vector<xMusicFile*>& tracks);
    /**
     * Indicate end of queueing tracks and hand over to the actual player.
     */
    void finishedQueueTracks(bool autoPlay);
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
    /**
     * Signal used to update queue with larger list in stages.
     *
     * @param listTracks list of pair of album and list of track objects (sorted) for a list of artists.
     * @param listTracksIterator iterator to the list element to be added.
     * @param currentListTracks current number of tracks inserted.
     * @param maxListTracks maximal number of tracks inserted overall.
     */
    void scanAllAlbumsForListArtistsIterate(const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>& listTracks,
                                            const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>::const_iterator& listTracksIterator,
                                            int currentListTracks, int maxListTracks);

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
    void scannedArtists(const std::list<xMusicDirectory>& artists);
    /**
     * Receive the result of the album scan for a given artist.
     *
     * Update the QListWidget for the list of albums. The widget itself sorts
     * all added entries. Clear the QListWidget for the list of tracks.
     *
     * @param albums unordered list of album names.
     */
    void scannedAlbums(const std::list<xMusicDirectory>& albums);
    /**
     * Receive the result of the track scan for a given artist/album
     *
     * Update the QListWidget for the list of tracks. The widget itself sorts
     * all added entries.
     *
     * @param tracks unordered list of track objects.
     */
    void scannedTracks(const std::list<xMusicFile*>& tracks);
    /**
     * Receive the result of the all album and track scan for a given artist
     *
     * @param albumTracks sorted list of pairs of album/list of track objects to be queued.
     */
    void scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,std::vector<xMusicFile*>>>& albumTracks);
    /**
     * Receive the result of all albums and track scan for a given list of artists.
     *
     * @param listTracks list of pair of album and list of track objects (sorted) for a list of artists.
     */
    void scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>& listTracks);

private slots:
    /**
     * Scan for albums of the selected artist.
     *
     * Function is triggered whenever a rows in the list of artists is
     * selected. The function then retrieves the artist name and signals
     * a scanForArtists.
     *
     * @param index listIndex of the artist in the QListWidget.
     */
    void selectArtist(int index);
    /**
     * Queue all albums for the currently selected artists.
     *
     * Function is triggered whenever an artist out of the list of artists
     * is double-clicked. The artist name is retrieved and all its albums
     * are queued.
     *
     * @param artistItem pointer to the currently selected listIndex.
     */
    void queueArtist(xPlayerListWidgetItem* artistItem);
    /**
     * Create context menu for artist info and transitions.
     *
     * Function is triggered by a right-click on an artist in the list of
     * artists. The menu contains the artist info and the top transitions
     * to and from the current artist.
     *
     * @param point position for the context menu.
     */
    void currentArtistContextMenu(const QPoint& point);
    /**
     * Scan for tracks of the selected artist/album.
     *
     * Function is triggered whenever a rows in the list of albums is
     * selected. The function then retrieves the selected artist name and
     * the selected album name. It then signals a scanForArtistAndAlbum.
     *
     * @param index listIndex of the artist in the QListWidget.
     */
    void selectAlbum(int index);
    /**
     * Queue the currently selected album.
     *
     * Function is triggered whenever an album out of the list of albums
     * is double-clicked. The album name is retrieved and queued.
     *
     * @param albumItem pointer to the currently selected listIndex.
     */
    void queueAlbum(xPlayerListWidgetItem* albumItem);
    /**
     * Append tracks to the playlist (queue).
     *
     * Function is triggered by a double click to an entry in the list of
     * tracks. The position of the selected track is determined and the track
     * and the following tracks will be added to the queue.
     *
     * @param trackItem pointer to the currently selected listIndex.
     */
    void selectTrack(xPlayerListWidgetItem* trackItem);
    /**
     * Append single track to the playlist (queue)
     *
     * Function is triggered by a right click to an entry in the list of
     * tracks. Only the track currently selected (at this position) is added.
     * (@see selectTrack)
     *
     * @param point position of the right-click.
     */
    void currentTrackRightClicked(const QPoint& point);
    /**
     * Filter the list of artists.
     *
     * Function is triggered whenever an element out of the selectors is selected.
     * The selector is retrieved and used to filter the list of artists.
     *
     * @param selector the currently selected selector as string.
     */
    void selectArtistSelector(const QString& selector);
    /**
     * Reorder the list of artists and albums
     *
     * @param enabled sort by latest time written if true, by name otherwise.
     */
    void selectSortingLatest(bool enabled);
    /**
     * Queue all albums for the currently all artists starting with the selector.
     *
     * Function is triggered whenever a selector out of the list of selectors
     * is double-clicked. All artists are retrieved and all their albums
     * are queued. A double-click on "none" will queue the entire library.
     *
     * @param selector the currently selected artist selector.
     */
    void queueArtistSelector(const QString& selector);
    /**
     * Filter the list of albums.
     *
     * Function is triggered whenever the album selectors are updated. The match
     * and not match selectors are retrieved and used to filter the list of albums.
     *
     * @param match the album must match one element out of this list.
     * @param notMatch the album must not match any element out of this list.
     */
    void selectAlbumSelectors(const QStringList& match, const QStringList& notMatch);
    /**
     * Filter the list of albums based on a database artist and album mapping.
     *
     * @param databaseMatch the artist and album mapping generated based on the database.
     * @param databaseNotMatch invert the matching if true.
     */
    void selectAlbumDatabaseSelectors(const std::map<QString,std::set<QString>>& databaseMatch, bool databaseNotMatch);
    /**
     * Clear the database artist and album mapping filter.
     */
    void clearAlbumDatabaseSelectors();
    /**
     * Clear all album selector filters. Includes database filters.
     */
    void clearAllAlbumSelectors();
    /**
     * Update the search selector filter.
     *
     * @param match the tuple of artist, album and track name match as string.
     */
    void updateSearchSelectorFilter(const std::tuple<QString,QString,QString>& match);
    /**
     * Clear the search selector filter.
     */
    void clearSearchSelectorFilter();
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
     * @param trackItem pointer to the double-clicked listItem.
     */
    void currentQueueTrackDoubleClicked(xPlayerListWidgetItem* trackItem);
    /**
     * Select artist and album in the corresponding lists.
     *
     * Function is triggered by a ctrl-click on an entry in the list of
     * queued tracks. The artist and album is only selected if they are
     * currently visible according to the selectors.
     *
     * @param trackItem pointer to the currently selected listIndex.
     */
    void currentQueueTrackCtrlClicked(xPlayerListWidgetItem* trackItem);
    /**
     *  Remove the current track from the playlist (queue).
     *
     * @param point position of the right-click.
     */
    void currentQueueTrackRightClicked(const QPoint& point);
    /**
     * Clear the list of queued tracks.
     */
    void clearQueue();
    /**
     * Update queue UI with the loaded playlist entries.
     *
     * @param entries list of tuples of artist, album and tracks.
     */
    void playlist(const std::vector<std::tuple<QString,QString,QString>>& entries);
    /**
     * Show playlist dialog.
     */
    void showPlaylistDialog();
    /**
     * Show tags dialog.
     */
    void showTagsDialog();
    /**
     * Show or hide the selector tabs.
     */
    void updatedMusicViewSelectors();
    /**
     * Show or hide the filters.
     */
    void updatedMusicViewFilters();
    /**
     * Update the music database overlay on configuration changes.
     */
    void updatedDatabaseMusicOverlay();
    /**
     * Update the album selectors and re-filter albums if necessary.
     */
    void updatedMusicLibraryAlbumSelectors();
    /**
     * Update the total time for the track list.
     *
     * @param total the total time (in ms) to be displayed, clear if 0.
     */
    void updateTracksTotalTime(qint64 total);
    /**
     * Update the total time for the queue list.
     *
     * @param total the total time (in ms) to be displayed, clear if 0.
     */
    void updateQueueTotalTime(qint64 total);
    /**
     * Worker function that inserts a chunk of tracks into queue.
     *
     * The functions inserts the tracks into the list to which the list iterator is currently pointing to.
     * A signal is emitted in order to insert the the next chunk of tracks into the queue without blocking the
     * event queue.
     *
     * @param listTracks list of pair of album and list of track objects (sorted) for a list of artists.
     * @param listTracksIterator iterator to the list element to be added.
     * @param currentListTracks current number of tracks inserted.
     * @param maxListTracks maximal number of tracks inserted overall.
     */
    void scannedAllAlbumsForListArtistsWorker(const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>& listTracks,
                                              const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>::const_iterator& listTracksIterator,
                                              int currentListTracks, int maxListTracks);

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
     * Clear track list and stop a running track list updater thread.
     */
    void clearTrackList();
    /**
     * Update the database overlay for currently played track and add tooltips.
     *
     * @param artist the artist for the currently played track.
     * @param album the album for the currently played track.
     * @param track the track number and name for the currently played track.
     * @param playCount the play count for the currently played track.
     * @param timeStamp the last played time stamp in milli seconds for the currently played track.
     */
    void updatePlayedTrack(const QString& artist, const QString& album, const QString& track, int playCount, qint64 timeStamp);
    /**
     * Update the list of artists.
     *
     * @param artists the list of all unfiltered artists.
     */
    void updateScannedArtists(const std::list<xMusicDirectory>& artists);
    /**
     * Filter the list of artists based on the selected selector.
     *
     * @param artists unfiltered list of artists.
     * @return filtered list of artists that start with the selector string.
     */
    [[nodiscard]] std::list<xMusicDirectory> filterArtists(const std::list<xMusicDirectory>& artists);
    /**
     * Sort the list of music directory entries.
     *
     * The sorting is performed by last time written if the corresponding flag is
     * set, otherwise the list is sorted by name.
     *
     * @param list the list of directory entries to be sorted.
     */
    void sortListMusicDirectory(std::list<xMusicDirectory>& list) const;
    /**
     * Show tag selection popup menu.
     *
     * @param list the list widget for which the popup menu is shown.
     * @param point the point where the popup menu is shown.
     */
    void tagPopupMenu(xPlayerListWidget* list, const QPoint& point);
    /**
     * Helper function creating a QGroupBox with an QListWidget.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @param displayTime add column to list widget for time display.
     * @param parent pointer to the parent widget.
     * @return pair of pointer to the created QGroupBox and QListWidget.
     */
    auto addListWidgetGroupBox(const QString& boxLabel, bool displayTime, QWidget* parent);
    /**
     * Helper function creating a QGroupBox with an QLineEdit.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @param parent pointer to the parent widget.
     * @return pair of pointer to the created QGroupBox and QLineEdit.
     */
    auto addLineEditGroupBox(const QString& boxLabel, QWidget* parent);

    QStackedWidget* musicStacked;
    QWidget* musicListView;
    xPlayerArtistInfo* musicInfoView;
    xMusicPlayer* musicPlayer;
    xMusicLibrary* musicLibrary;
    xMusicLibraryFilter musicLibraryFilter;
    xPlayerMusicWidget* playerWidget;
    xPlayerListWidget* artistList;
    QGroupBox* artistFilterBox;
    QLineEdit* artistFilterLineEdit;
    xPlayerListWidget* albumList;
    QGroupBox* albumFilterBox;
    QLineEdit* albumFilterLineEdit;
    xPlayerListWidget* trackList;
    QGroupBox* trackFilterBox;
    QLineEdit* trackFilterLineEdit;
    QGroupBox* trackBox;
    QTabWidget* selectorTabs;
    xPlayerMusicArtistSelectorWidget* artistSelectorList;
    xPlayerMusicAlbumSelectorWidget* albumSelectorList;
    xPlayerMusicSearchWidget* searchSelector;
    xPlayerListWidget* queueList;
    QProgressBar* queueProgress;
    QGroupBox* queueBox;
    int playedTrack;
    bool useDatabaseMusicOverlay;
    qint64 databaseCutOff;
    /**
     * Store the current list of unfiltered artists and albums for later filtering.
     */
    std::list<xMusicDirectory> unfilteredArtists;
    std::list<xMusicDirectory> filteredArtists;
    /**
     * Currently played artist and album. May differ from currently selected artist and album.
     */
    QString currentArtist;
    QString currentAlbum;
    /**
     * Save current artist selector and sorting mode.
     */
    QString currentArtistSelector;
    bool useSortingLatest;
};

#endif
