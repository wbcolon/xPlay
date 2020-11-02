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

class xMusicLibraryFiles:public QObject {
    Q_OBJECT

public:
    /**
     * Return the music library files object for thread safe access.
     *
     * @return pointer to a singleton of the music library files object.
     */
    static xMusicLibraryFiles* files();
    /**
     * Set map of albums with tracks for the given artist.
     *
     * @param artist name of the artist as string.
     * @param albumTracks the tracks for each albums of an artist.
     */
    void set(const QString& artist, const std::map<QString,std::list<std::filesystem::path>>& albumTracks);
    /**
     * Set the tracks for the given artist and album.
     *
     * @param artist name of the artist as string.
     * @param album name of the album for the specified artist as string.
     * @param tracks the tracks for the given artist and album.
     */
    void set(const QString& artist, const QString& album, const std::list<std::filesystem::path>& tracks);
    /**
     * Retrieve the tracks for the given artist and album.
     *
     * @param artist name of the artist as string.
     * @param album  name of the album for the specified artist as string.
     * @return a list of filesystem paths for the given artist and album.
     */
    std::list<std::filesystem::path> get(const QString& artist, const QString& album) const;
    /**
     * Retrieve the albums for the given artist.
     *
     * @param artist name of the artist as string.
     * @return a list of strings containing the albums for the specified artist.
     */
    QStringList get(const QString& artist) const;
    /**
     * Retrieve all artists, albums and tracks.
     *
     * @return a list of tuples with artist,album and path to the track.
     */
    std::list<std::tuple<QString, QString, std::filesystem::path>> get() const;
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
    std::list<std::filesystem::path> scanForAlbumTracks(const std::filesystem::path& albumPath);

private slots:
    /**
     * Update accepted music file extensions.
     */
    void updateMusicExtensions();

private:
    xMusicLibraryFiles(QObject* parent=nullptr);
    ~xMusicLibraryFiles() = default;
    /**
     * Determine if the given file is a music file.
     *
     * @param file the path to the file checked.
     * @return true if the file has a music extension, false otherwise.
     */
    bool isMusicFile(const std::filesystem::path& file);

    std::map<QString, std::map<QString, std::list<std::filesystem::path>>> musicFiles;
    QStringList musicExtensions;
    mutable QMutex musicFilesLock;
    static xMusicLibraryFiles* musicLibraryFiles;
};

class xMusicLibraryScanning:public QThread {
    Q_OBJECT

public:
    xMusicLibraryScanning(QObject* parent=nullptr);
    ~xMusicLibraryScanning();

    void setBaseDirectory(const std::filesystem::path& dir);

    void run() override;

signals:
    void scannedArtists(const QStringList& artists);

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
    xMusicLibrary(QObject* parent=nullptr);
    ~xMusicLibrary() = default;

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
    const std::filesystem::path& getBaseDirectory() const;

signals:
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
     * @param tracks list of the track names.
     */
    void scannedTracks(const QStringList& tracks);

public slots:
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

private slots:
    void scanningFinished();

private:
    /**
     * Scan music library using the xMusicLibraryScanning class.
     */
    void scan();

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
