#ifndef __XMAINWIDGET_H__
#define __XMAINWIDGET_H__

#include "xMusicPlayer.h"
#include "xPlayerWidget.h"

#include <QListWidget>
#include <QString>

#include <vector>
#include <list>
#include <set>

class xMainWidget:public QWidget {
    Q_OBJECT

public:
    xMainWidget(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xMainWidget() = default;

signals:
    /**
     * Signals used for communication with player widget and the music player.
     * The corresponding slots are implemented in the play widget or music
     * player classes.
     */
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
     * Signal a set of tracks to be queued in the playlist.
     *
     * @param artist the artist name for the tracks.
     * @param album the album name for the tracks.
     * @param tracks ordered vector of track names.
     */
    void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks);
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
    void scannedArtists(const std::list<QString>& artists);
    /**
     * Receive the result of the album scan for a given artist.
     *
     * Update the QListWidget for the list of albums. The widget itself sorts
     * all added entries. Clear the QListWidget for the list of tracks.
     *
     * @param albums unordered list of album names.
     */
    void scannedAlbums(const std::list<QString>& albums);
    /**
     * Receive the result of the track scan for a given artist/album
     *
     * Update the QListWidget for the list of tracks. The widget itself sorts
     * all added entries.
     *
     * @param tracks unordered list of track names.
     */
    void scannedTracks(const std::list<QString>& tracks);

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
     * Filter the list of artists.
     *
     * Function is triggered whenever an element out of the selectors is selected.
     * The selector is retrieved and used to filter the list of artists.
     *
     * @param selector position of the first character used to filter artists.
     */
    void selectArtistSelector(int selector);
    /**
     * Select the current track in the playlist (queue).
     *
     * @param index position of the track currently played.
     */
    void currentQueueTrack(int index);
    /**
     * Clear the list of queued tracks.
     */
    void clearQueue();

    void selectSingleTrack(const QPoint& point);

private:
    /**
     * Update the list of selectors (and add "none").
     *
     * @param selectors a set of characters (as QString) used for filtering artists.
     */
    void scannedArtistsSelectors(const std::set<QString>& selectors);
    /**
     * Filter the list of artists based on the selected selector.
     *
     * @param artists unfiltered list of artists.
     * @return filtered list of artists that start with the selector string.
     */
    std::list<QString> filterArtists(const std::list<QString>& artists);

    /**
     * Helper function creating a QGroupBox with an QListWidget.
     *
     * @param boxLabel contains the label for the surrounding groupbox.
     * @return pair of pointer to the created QGroupBox and QListWidget.
     */
    auto addGroupBox(const QString& boxLabel);

    xMusicPlayer* musicPlayer;
    QListWidget* artistList;
    QListWidget* albumList;
    QListWidget* trackList;
    QListWidget* artistSelectorList;
    QListWidget* queueList;
    /**
     * Store the current list of unfiltered artists for later filtering.
     */
    std::list<QString> unfilteredArtists;
};

#endif
