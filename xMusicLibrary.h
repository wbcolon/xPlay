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

#include <filesystem>
#include <list>
#include <map>

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
    void scannedArtists(const std::list<QString>& artists);
    /**
     * Signal the list of scanned albums for the selected artist.
     *
     * @param albums list of the album names.
     */
    void scannedAlbums(const std::list<QString>& albums);
    /**
     * Signal the list of scanned tracks for the selected artist and album.
     *
     * @param tracks list of the track names.
     */
    void scannedTracks(const std::list<QString>& tracks);

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
    /**
     * Scan for all tracks in the giving album path.
     *
     * @param albumPath the path to the artist/album directory to scan for tracks.
     * @return list of paths containing the album path and all tracks.
     */
    std::list<std::filesystem::path> scanForAlbumTracks(const std::filesystem::path& albumPath);

    std::filesystem::path baseDirctory;
    /**
     * The structure that contains the music library. Tracks are cached after they have
     * been read on demand.
     */
    std::map<QString, std::map<QString, std::list<std::filesystem::path>>> musicFiles;
};

#endif
