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

#ifndef __XMUSICLIBRARY_H__
#define __XMUSICLIBRARY_H__

#include "xMusicLibraryEntry.h"
#include "xMusicLibraryFilter.h"

#include <QThread>
#include <QMutex>


class xMusicLibrary: public xMusicLibraryEntry {
    Q_OBJECT

public:
    explicit xMusicLibrary(QObject* parent=nullptr);
    ~xMusicLibrary() override = default;
    /**
     * Set base directory for the music library.
     *
     * A certain structure of the music library is expected.
     * The music library is scanned (@see scan).
     *
     * @param path directory that contains the music library.
     */
    void setPath(const std::filesystem::path& base);
    /**
     * Scan the entire music library.
     */
    void scan() override;
    /**
     * Verify if the music library has been scanned.
     *
     * @return true if the entry has been scanned, false otherwise.
     */
    [[nodiscard]] bool isScanned() const override;
    /**
     * Get all artists.
     *
     * @return a sorted vector of album entries.
     */
    [[nodiscard]] std::vector<xMusicLibraryArtistEntry*> getArtists();
    /**
     * Stop the scanning process and clear the library.
     */
    void clear();

signals:
    /**
     * Signal emitted on scanning error or empty library.
     */
    void scanningError();
    /**
     * Signal emitted on scanning progess.
     *
     * @param percent the scanning progress in percent.
     */
    void scanningProgress(int percent);
    /**
     * Signal that the complete scanning process is finished.
     */
    void scanningFinished();
    /**
     * The following signals are triggers by the different scanning functions.
     * They are used to communicate the scanning result from the music library
     * to the main widget which implements the corresponding slots.
     */
    /**
     * Signal the list of scanned artists
     *
     * @param artists list of the artist names.
     */
    void scannedArtists(const std::vector<xMusicLibraryArtistEntry*>& artists);
    /**
     * Signal the list of scanned albums for the selected artist.
     *
     * @param albums list of the album names.
     */
    void scannedAlbums(const std::vector<xMusicLibraryAlbumEntry*>& albums);
    /**
     * Signal the list of scanned tracks for the selected artist and album.
     *
     * @param tracks list of the music file objects.
     */
    void scannedTracks(const std::vector<xMusicLibraryTrackEntry*>& tracks);
    /**
     * Signal the list of scanned album/tracks for the selected artist.
     *
     * @param albumTracks list of pairs of album and list of track name (sorted).
     */
    void scannedAllAlbumTracks(const QString& artistName, const QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>& albumTracks);
    /**
     * Signal the list of scanned album/tracks for a list of artists.
     *
     * @param listTracks list of pair of album and list of track name (sorted) for a list of artists.
     */
    void scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>>>& listTracks);
    /**
     * Signal the list of entries not found in the music library
     *
     * @param listEntries a list of tuples of artist, album and track not found.
     */
    void scannedUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries);

public slots:
    /**
     * Scan and filter for artists.
     *
     * No actual scanning is performed here. The mediaFile data structure
     * generated in scan is used. The artist, album and track name filters
     * are applied. Triggers the signal scannedArtists.
     *
     * @param filter the filter to be applied.
     */
    void scan(const xMusicLibraryFilter& filter);
    /**
     * Scan albums for the given artist.
     *
     * No actual scanning is performed here. The mediaFile data structure
     * generated in scan is used. Triggers the signal scannedAlbums.
     *
     * @param artistName the artist for which we scan for albums.
     */
    void scanForArtist(const QString& artistName);
    /**
     * Scan and filter albums for the given artist.
     *
     * No actual scanning is performed here. The mediaFile data structure
     * generated in scan is used. The album and track name filters are
     * applied. Triggers the signal scannedAlbums.
     *
     * @param artistName the artist for which we scan for albums.
     * @param filter the filter to be applied.
     */
    void scanForArtist(const QString& artistName, const xMusicLibraryFilter& filter);
    /**
     * Scan tracks for the given artist and album.
     *
     * The mediaFile data structure is used to obtain the album path
     * for the given artist/album. The tracks are scanned if necessary.
     * Scanning results will be cached in the mediaFile data structure
     * for later use (@see scanForAlbumTracks). Triggers the signal
     * scannedTracks.
     *
     * @param artistName the artist for which we scan for tracks.
     * @param albumName the album for which we scan for tracks.
     */
    void scanForArtistAndAlbum(const QString& artistName, const QString& albumName);
    /**
     * Scan all albums and tracks for the given artist.
     *
     * @param artistName the artist for which we scan all albums and tracks.
     */
    void scanAllAlbumsForArtist(const QString& artistName);
    /**
     * Scan and filter all albums and tracks for the given artist.
     *
     * @param artistName the artist for which we scan all albums and tracks.
     * @param filter the filter to be applied.
     */
    void scanAllAlbumsForArtist(const QString& artistName, const xMusicLibraryFilter& filter);
    /**
     * Scan all albums and tracks for a given list of artists.
     *
     * @param listArtists the list of artists for which we scan all albums and tracks.
     */
    void scanAllAlbumsForListArtists(const QStringList& listArtists);
    /**
     * Scan and filter all albums and tracks for a given list of artists.
     *
     * @param listArtists the list of artists for which we scan all albums and tracks.
     * @param filter the filter to be applied.
     */
    void scanAllAlbumsForListArtists(const QStringList& listArtists, const xMusicLibraryFilter& filter);
    /**
     * Scan list to find entries that are not in the music library.
     *
     * @param listEntries the list of tuples of artist, album and track to verify.
     */
    void scanForUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries);
    /**
     * Find the track entry for the given artist, album and track name.
     *
     * @param artistName name of the artist as string.
     * @param albumName name of the album for the artist as string.
     * @param trackName track name for the album as string.
     * @return a pointer to the track entry if existent, nullptr otherwise.
     */
    xMusicLibraryTrackEntry* getTrackEntry(const QString& artistName, const QString& albumName, const QString& trackName);
    /**
     * Compare the current music library files to a given one.
     *
     * We compare the libraries based on the difference in artists,
     * albums and tracks.
     *
     * @param libraryFiles the music library that is compared to.
     * @param missingArtists list of artists that are not present in given library.
     * @param additionalArtists list of artists that are only present in the given library.
     * @param missingAlbums list of albums for present artists that are not present in given library.
     * @param additionalAlbums list of albums for present artists that are only present in given library.
     * @param missingTracks list of tracks for present artists and albums that are not present in given library.
     * @param additionalTracks list of tracks for present artists and albums that are only present in the given library.
     * @param differentTracks pair of list of tracks present in both, but file sizes are different.
     */
    void compare(const xMusicLibrary* library, QStringList& missingArtists, QStringList& additionalArtists,
                 std::map<QString, QStringList>& missingAlbums, std::map<QString, QStringList>& additionalAlbums,
                 std::list<xMusicLibraryTrackEntry*>& missingTracks, std::list<xMusicLibraryTrackEntry*>& additionalTracks,
                 std::pair<std::list<xMusicLibraryTrackEntry*>, std::list<xMusicLibraryTrackEntry*>>& differentTracks) const;
    /**
     * Compare the current music library files to a given one.
     *
     * We compare the libraries in order to find equal tracks.
     *
     * @param libraryFiles the music library that is compared to.
     * @param equalTracks map of artist, album and list of tracks that are equal.
     */
    void compare(const xMusicLibrary* library,
                 std::map<QString, std::map<QString, std::list<xMusicLibraryTrackEntry*>>>& equalTracks) const;

private:
    /**
     * Determine status of the given artist directory entry.
     *
     * @param dirEntry the directory entry.
     * @return true if the entry is a valid artist directory, false otherwise.
     */
    bool isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) override;
    /**
     * Access the a specific child.
     *
     * @param index the index of the child as integer.
     * @return a pointer to the child if it exists, nullptr otherwise.
     */
    xMusicLibraryEntry* child(size_t index) override;
    /**
     * Update the parent if a child was renamed.
     *
     * @param childEntry the pointer to the renamed child entry.
     */
    void updateParent(xMusicLibraryEntry* childEntry) override;
    /**
     * Actual scanning function, running in a thread.
     */
    void scanThread();
    /**
     * Helper function filtering a vector of artists as defined by the filter.
     *
     * @param artists the vector of unfiltered artists.
     * @param filter the filter to be applied.
     * @return the vector of filtered artists.
     */
    static std::vector<xMusicLibraryArtistEntry*> filterArtists(const std::vector<xMusicLibraryArtistEntry*>& artists,
                                                                const xMusicLibraryFilter& filter);
    /**
     * Helper function filtering the albums of an artist as defined by the filter.
     *
     * @param artist a pointer to the artist entry object.
     * @param filter the filter to be applied to the albums.
     * @return the vector of filtered albums for the artist.
     */
    static std::vector<xMusicLibraryAlbumEntry*> filterAlbums(xMusicLibraryArtistEntry* artist,
                                                              const xMusicLibraryFilter& filter);
    /**
     * Helper function filtering the tracks of an album as defined by the filter.
     *
     * @param album a pointer to the album entry object.
     * @param filter the filter to be applied to the tracks.
     * @return the vector of filtered tracks for the album.
     */
    static std::vector<xMusicLibraryTrackEntry*> filterTracks(xMusicLibraryAlbumEntry* album,
                                                              const xMusicLibraryFilter& filter);
    /**
     * Implement a list difference for track entries
     */
    static void listDifference(const std::vector<xMusicLibraryTrackEntry*>& a, const std::vector<xMusicLibraryTrackEntry*>& b,
                               std::list<xMusicLibraryTrackEntry*>& missing, std::list<xMusicLibraryTrackEntry*>& additional,
                               std::pair<std::list<xMusicLibraryTrackEntry*>, std::list<xMusicLibraryTrackEntry*>>& different);

    // Use mutex to secure access to the artists structures.
    mutable QMutex musicLibraryLock;
    QThread* musicLibraryScanning;
    std::vector<xMusicLibraryArtistEntry*> musicLibraryArtists;
    std::map<QString, xMusicLibraryArtistEntry*> musicLibraryArtistsMap;
};



#endif
