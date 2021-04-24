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

#include <QString>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QStringList>

#include <filesystem>
#include <list>
#include <map>
#include <set>

class xMusicFile;

class xMusicLibraryFilter {
public:
    xMusicLibraryFilter();
    ~xMusicLibraryFilter() = default;
    /**
     * Check if the filter has defined any artist related filters.
     *
     * @return true if any artist filter have been specified, false otherwise.
     */
    [[nodiscard]] bool hasArtistFilter() const;
    /**
     * Check if the filter has defined any album related filters.
     *
     * @return true if any album filter have been specified, false otherwise.
     */
    [[nodiscard]] bool hasAlbumFilter() const;
    /**
     * Check if the filter has defined any track name related filters.
     *
     * @return true if any track name filter have been been specified, false otherwise.
     */
    [[nodiscard]] bool hasTrackNameFilter() const;
    /**
     * Check if the given artist passes the artist filter.
     *
     * @param artist the given artist to be verified.
     * @return true if the artist passes the artist filter, false otherwise.
     */
    [[nodiscard]] bool isMatchingArtist(const QString& artist) const;
    /**
     * Check if the given album passes the album filter.
     *
     * @param album the given album to be verified.
     * @return true if the album passes the album filter, false otherwise.
     */
    [[nodiscard]] bool isMatchingAlbum(const QString& album) const;
    /**
     * Check if the given track name passes the track name filter.
     *
     * @param trackName the given track name to be verified.
     * @return true if the track name passes the track name filter, false otherwise.
     */
    [[nodiscard]] bool isMatchingTrackName(const QString& trackName) const;
    /**
     * Check if the given artist and album match the database mapping filter.
     *
     * @param artist the given artist to be verified.
     * @param album the given album to be verified.
     * @return artistAlbumNotMatch if the artist/album are not in the mapping, not artistAlbumNotMatch otherwise.
     */
    [[nodiscard]] bool isMatchingDatabaseArtistAndAlbum(const QString& artist, const QString& album) const;
    /**
     * Set the album match and not match. Overwrite old setting.
     *
     * @param match list of strings that must match.
     * @param notMatch list of strings that are not allowed to match.
     */
    void setAlbumMatch(const QStringList& match, const QStringList& notMatch);
    /**
     * Add the search match.
     *
     * @param match a tuple of artist, album and track name match.
     */
    void addSearchMatch(const std::tuple<QString,QString,QString>& match);
    /**
     * Add the database album match and not match.
     *
     * @param databaseMatch the artist and albums recorded in the database.
     * @param databaseNotMatch use databaseMatch to invert matching if true.
     */
    void setDatabaseMatch(const std::map<QString,std::set<QString>>& databaseMatch, bool databaseNotMatch);
    /**
     * Clear the database album match.
     */
    void clearDatabaseMatch();
    /**
     * Clear all match and not match strings.
     */
    void clearMatch();

private:
    /**
     * Helper function checking any of list elements matches the given text.
     *
     * @param listMatch the list of matches as strings.
     * @param text the text to compare to.
     * @param emptyMatch the result if list of matches is empty.
     *
     * @return true if the any of the list elements matches the text, false otherwise.
     */
    static bool filterMatchesElement(const QStringList& listMatch, const QString& text, bool emptyMatch);

    // Substrings that artist,album,track match or not match.
    QStringList albumMatch;
    QStringList albumNotMatch;
    QString artistSearchMatch;
    QString albumSearchMatch;
    QString trackNameSearchMatch;
    bool useArtistAlbumMatch;
    std::map<QString,std::set<QString>> artistAlbumMatch;
    bool artistAlbumNotMatch;
};

class xMusicLibraryFiles:public QObject {
    Q_OBJECT

public:
    /**
     * Return the music library files object for thread safe access.
     *
     * @return pointer to a singleton of the music library files object.
     */
    [[nodiscard]] static xMusicLibraryFiles* files();
    /**
     * Set map of albums with tracks for the given artist.
     *
     * @param artist name of the artist as string.
     * @param albumTracks the tracks for each albums of an artist.
     */
    void set(const QString& artist, const std::map<QString,std::list<xMusicFile*>>& albumTracks);
    /**
     * Set the tracks for the given artist and album.
     *
     * @param artist name of the artist as string.
     * @param album name of the album for the specified artist as string.
     * @param tracks the tracks for the given artist and album.
     */
    void set(const QString& artist, const QString& album, const std::list<xMusicFile*>& tracks);
    /**
     * Retrieve the tracks for the given artist and album.
     *
     * @param artist name of the artist as string.
     * @param album  name of the album for the specified artist as string.
     * @return a list of filesystem paths for the given artist and album.
     */
    [[nodiscard]] std::list<xMusicFile*> get(const QString& artist, const QString& album);
    /**
     * Retrieve the filtered tracks for the given artist and album.
     *
     * The filtered tracks is empty if none of the tracks passes track name filter and is
     * unfiltered otherwise. The function returns either an empty list of the list of all
     * tracks.
     *
     * @param artist name of the artist as string.
     * @param album  name of the album for the specified artist as string.
     * @param filter the filter to be applied.
     * @return a filtered list of music file objects for the given artist and album.
     */
    [[nodiscard]] std::list<xMusicFile*> get(const QString& artist, const QString& album, const xMusicLibraryFilter& filter);
    /**
     * Retrieve the albums for the given artist.
     *
     * @param artist name of the artist as string.
     * @return a list of strings containing the albums for the specified artist.
     */
    [[nodiscard]] QStringList get(const QString& artist) const;
    /**
     * Retrieve the filtered albums for the given artist.
     *
     * @param artist name of the artist as string.
     * @param filter the filter to be applied.
     * @return a list of strings containing the albums for the specified artist and passing the album filter.
     */
    [[nodiscard]] QStringList get(const QString& artist, const xMusicLibraryFilter& filter) const;
    /**
     * Retrieve the filtered artists.
     *
     * @param filter the filter to be applied.
     * @return a list of strings containing the artists of the library that pass the artist filter.
     */
    [[nodiscard]] QStringList get(const xMusicLibraryFilter& filter) const;
    /**
     * Retrieve all artists, albums and tracks.
     *
     * @return a list of tuples with artist,album and path to the track.
     */
    [[nodiscard]] std::list<std::tuple<QString, QString, xMusicFile*>> get() const;
    /**
     * Check whether or not the library is empty.
     *
     * @return true if the library is empty, false otherwise.
     */
    [[nodiscard]] bool isEmpty() const;
    /**
     * Clear the library.
     */
    void clear();
    /**
     * Scan for all tracks in the giving album path.
     *
     * @param albumPath the path to the artist/album directory to scan for tracks.
     * @return list of paths containing the album path and all tracks.
     */
    [[nodiscard]] std::list<xMusicFile*> scanForAlbumTracks(xMusicFile* albumPath);

private slots:
    /**
     * Update accepted music file extensions.
     */
    void updateMusicExtensions();

private:
    explicit xMusicLibraryFiles(QObject* parent=nullptr);
    ~xMusicLibraryFiles() noexcept override;
    /**
     * Determine if the given file is a music file.
     *
     * @param file the path to the file checked.
     * @return true if the file has a music extension, false otherwise.
     */
    bool isMusicFile(const std::filesystem::path& file);

    std::map<QString, std::map<QString, std::list<xMusicFile*>>> musicFiles;
    QStringList musicExtensions;
    mutable QMutex musicFilesLock;
    static xMusicLibraryFiles* musicLibraryFiles;
};

class xMusicLibraryScanning:public QThread {
    Q_OBJECT

public:
    explicit xMusicLibraryScanning(QObject* parent=nullptr);
    ~xMusicLibraryScanning() noexcept override;
    /**
     * Set the base directory for the music library.
     *
     * This function does not trigger a scan. The scan must be
     * triggered explicitly.
     *
     * @param dir absolute path to the music library.
     */
    void setBaseDirectory(const std::filesystem::path& dir);
    /**
     * Scan the music library for music files.
     */
    void run() override;

signals:
    /**
     * Signal emitted with list of artist after initial scan is finished.
     *
     * @param artists list of scanned artists as string.
     */
    void scannedArtists(const QStringList& artists);
    /**
     * Signal emitted on scanning error or empty database.
     */
    void scanningError();

private:
    /**
     * Scan music library
     *
     * Assumes that music library has the following directory layout:
     * * artist/album/track
     * The function only scans the artist and album directories. The tracks
     * itself are not scanned. They will be later scanned, but on demand.
     * Triggers the signal scannedArtists.
     */
    void scan();

    std::filesystem::path baseDirectory;
    xMusicLibraryFiles* musicLibraryFiles;
};

class xMusicLibrary:public QObject {
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
     * @param base directory that contains the music library.
     */
    void setBaseDirectory(const std::filesystem::path& base);
    /**
     * Retrieve the currently set base directory of the music library.
     *
     * @return the base directory currently in use.
     */
    [[nodiscard]] const std::filesystem::path& getBaseDirectory() const;
    /**
     * Retrieve the music file object for the given artist, album and track name.
     *
     * @param artist the artist for the music file object.
     * @param album the album for the music file object.
     * @param trackName the track name for the music file object.
     * @return a pointer to corresponding the music file object in the library.
     */
    [[nodiscard]] xMusicFile* getMusicFile(const QString& artist, const QString& album, const QString& trackName);

    void cleanup();

signals:
    /**
     * Signal emitted on scanning error or empty library.
     */
    void scanningError();
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
    void scannedArtists(const QStringList& artists);
    /**
     * Signal the list of scanned albums for the selected artist.
     *
     * @param albums list of the album names.
     */
    void scannedAlbums(const QStringList& albums);
    /**
     * Signal the list of scanned tracks for the selected artist and album.
     *
     * @param tracks list of the music file objects.
     */
    void scannedTracks(const std::list<xMusicFile*>& tracks);
    /**
     * Signal the list of scanned album/tracks for the selected artist.
     *
     * @param albumTracks list of pairs of album and list of track name (sorted).
     */
    void scannedAllAlbumTracks(const QString& artist, const QList<std::pair<QString,std::vector<xMusicFile*>>>& albumTracks);
    /**
     * Signal the list of scanned album/tracks for a list of artists.
     *
     * @param listTracks list of pair of album and list of track name (sorted) for a list of artists.
     */
    void scannedListArtistsAllAlbumTracks(const QList<std::pair<QString, QList<std::pair<QString, std::vector<xMusicFile*>>>>>& listTracks);
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
     * @param artist the artist name for which we scan for albums.
     */
    void scanForArtist(const QString& artist);
    /**
     * Scan and filter albums for the given artist.
     *
     * No actual scanning is performed here. The mediaFile data structure
     * generated in scan is used. The album and track name filters are
     * applied. Triggers the signal scannedAlbums.
     *
     * @param artist the artist name for which we scan for albums.
     * @param filter the filter to be applied.
     */
    void scanForArtist(const QString& artist, const xMusicLibraryFilter& filter);
    /**
     * Scan tracks for the given artist and album.
     *
     * The mediaFile data structure is used to obtain the album path
     * for the given artist/album. The tracks are scanned if necessary.
     * Scanning results will be cached in the mediaFile data structure
     * for later use (@see scanForAlbumTracks). Triggers the signal
     * scannedTracks.
     *
     * @param artist the artist name for which we scan for tracks.
     * @param album the album name for which we scan for tracks.
     */
    void scanForArtistAndAlbum(const QString& artist, const QString& album);
    /**
     * Scan all albums and tracks for the given artist.
     *
     * @param artist the artist name for which we scan all albums and tracks.
     */
    void scanAllAlbumsForArtist(const QString& artist);
    /**
     * Scan and filter all albums and tracks for the given artist.
     *
     * @param artist the artist name for which we scan all albums and tracks.
     * @param filter the filter to be applied.
     */
    void scanAllAlbumsForArtist(const QString& artist, const xMusicLibraryFilter& filter);
    /**
     * Scan all albums and tracks for a given list of artists.
     *
     * @param listArtist the list of artists name for which we scan all albums and tracks.
     */
    void scanAllAlbumsForListArtists(const QStringList& listArtist);
    /**
     * Scan and filter all albums and tracks for a given list of artists.
     *
     * @param listArtist the list of artists name for which we scan all albums and tracks.
     * @param filter the filter to be applied.
     */
    void scanAllAlbumsForListArtists(const QStringList& listArtist, const xMusicLibraryFilter& filter);
    /**
     * Scan list to find entries that are not in the music library.
     *
     * @param listEntries the list of tuples of artist, album and track to verify.
     */
    void scanForUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries);

private slots:

private:
    /*
     * Helper function that only scans all albums and tracks for an artist.
     *
     * @param artist the artist name for which we scan all albums and tracks.
     * @param albumTracks the result of the scan as list of pairs of album name and vector of tracks.
     */
    void getAllAlbumsForArtist(const QString& artist,
                               QList<std::pair<QString,std::vector<xMusicFile*>>>& albumTracks);
    /*
     * Helper function that only scans and filters all albums and tracks for an artist.
     *
     * @param artist the artist name for which we scan all albums and tracks.
     * @param albumTracks the filtered result of the scan as list of pairs of album name and vector of tracks.
     * @param filter the filter to be applied.
     */
    void getAllAlbumsForArtist(const QString& artist,
                               QList<std::pair<QString,std::vector<xMusicFile*>>>& albumTracks,
                               const xMusicLibraryFilter& filter);

    std::filesystem::path baseDirectory;
    /**
     * The structure that contains the music library. Tracks are cached after they have
     * been read on demand.
     */
    xMusicLibraryFiles* musicLibraryFiles;
    /**
     * Scanning thread.
     */
    xMusicLibraryScanning* musicLibraryScanning;
};

#endif
