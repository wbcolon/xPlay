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
#include "xMusicLibrary.h"

#include <QDebug>

/*
 * class xMusicLibraryFiles
 */
xMusicLibraryFiles::xMusicLibraryFiles(QObject *parent):
        QObject(parent) {
}

void xMusicLibraryFiles::set(const QString& artist, const std::map<QString,std::list<std::filesystem::path>>& albumTracks) {
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        artistPos->second = albumTracks;
    } else {
        musicFiles[artist] = albumTracks;
    }
    musicFilesLock.unlock();
}

void xMusicLibraryFiles::set(const QString& artist, const QString& album,
                             const std::list<std::filesystem::path>& tracks) {
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        auto albumPos = artistPos->second.find(album);
        if (albumPos != artistPos->second.end()) {
            albumPos->second = tracks;
        } else {
            artistPos->second[album] = tracks;
        }
    } else {
        std::map<QString,std::list<std::filesystem::path>> albumMap;
        albumMap[album] = tracks;
        musicFiles[artist] = albumMap;
    }
    musicFilesLock.unlock();
}

void xMusicLibraryFiles::clear() {
    musicFilesLock.lock();
    musicFiles.clear();
    musicFilesLock.unlock();
}

std::list<std::filesystem::path> xMusicLibraryFiles::get(const QString& artist, const QString& album) const {
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        auto albumPos = artistPos->second.find(album);
        if (albumPos != artistPos->second.end()) {
            musicFilesLock.unlock();
            return albumPos->second;
        }
    }
    musicFilesLock.unlock();
    return std::list<std::filesystem::path>();
}

QStringList xMusicLibraryFiles::get(const QString& artist) const {
    QStringList albumList;
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        for (const auto& album : artistPos->second) {
            albumList.push_back(album.first);
        }
    }
    musicFilesLock.unlock();
    return albumList;
}

std::list<std::tuple<QString, QString, std::filesystem::path>> xMusicLibraryFiles::get() const {
    std::list<std::tuple<QString, QString, std::filesystem::path>> artistAlbumList;
    musicFilesLock.lock();
    for (const auto& artist : musicFiles) {
        for (const auto& album : artist.second) {
            artistAlbumList.emplace_back(std::make_tuple(artist.first, album.first, *(album.second.begin())));
        }
    }
    musicFilesLock.unlock();
    return artistAlbumList;
}


/*
 * class xMusicLibraryScanning
 */
xMusicLibraryScanning::xMusicLibraryScanning(xMusicLibraryFiles* lib, const std::filesystem::path& dir, QObject *parent):
        QThread(parent),
        musicLibrary(lib),
        baseDirectory(dir) {
}

void xMusicLibraryScanning::run() {
    qDebug() << "xMusicLibraryScanning: scanning base structure";
    // First scan the basic structure.
    scan();
    qDebug() << "xMusicLibraryScanning: scanning each album";
    // Second scan for tracks.
    auto artistAlbumList = musicLibrary->get();
    for (auto entry : artistAlbumList) {
        musicLibrary->set(std::get<0>(entry), std::get<1>(entry),
                xMusicLibrary::scanForAlbumTracks(std::get<2>(entry)));
    }
    qDebug() << "xMusicLibraryScanning: finished";
}

void xMusicLibraryScanning::scan() {
    // Cleanup before scanning path
    musicLibrary->clear();
    try {
        QStringList artistList;
        for (const auto& artistDir : std::filesystem::directory_iterator(baseDirectory)) {
            const auto& artistPath = artistDir.path();
            if (!std::filesystem::is_directory(artistPath)) {
                // No directory, no artist.
                continue;
            }
            auto artistName = QString::fromStdString(artistPath.filename());
            std::map<QString, std::list<std::filesystem::path>> artistAlbumMap;
            // Read all albums for the given artist.
            for (const auto& albumDir : std::filesystem::directory_iterator(artistPath)) {
                const auto& albumPath = albumDir.path();
                if (!std::filesystem::is_directory(albumPath)) {
                    // No directory, no album.
                    continue;
                }
                auto albumName = QString::fromStdString(albumPath.filename());
                //qDebug() << "xMusicLibrary::scanLibrary: artist/album: " << artistName << "/" << albumName;
                // Do not scan the files in each directory as it is too costly.
                // Instead add the album path as first element element to track list.
                // It will be later used to read the tracks on demand.
                std::list<std::filesystem::path> trackList;
                trackList.push_back(albumPath);
                artistAlbumMap[albumName] = trackList;
            }
            musicLibrary->set(artistName, artistAlbumMap);
            artistList.push_back(artistName);
        }
        // Update widget
        emit scannedArtists(artistList);
    } catch (...) {
        // Error scanning. Structure may not be as expected.
        // Clear everything currently scanned.
        musicLibrary->clear();
    }
}


/*
 * class xMusicLibraryScanning
 */

// Use "/tmp" as default path
const std::string defaultBaseDirectory{ "/tmp" };

// static function.
std::list<std::filesystem::path> xMusicLibrary::scanForAlbumTracks(const std::filesystem::path& albumPath) {
    std::list<std::filesystem::path> trackList;
    // Add the album path itself
    trackList.push_back(albumPath);
    // Add all files in the album path
    for (const auto& trackFile : std::filesystem::directory_iterator(albumPath)) {
        if (std::filesystem::is_regular_file(trackFile)) {
            trackList.push_back(trackFile.path());
        }
    }
    return trackList;
}

xMusicLibrary::xMusicLibrary(QObject* parent):
        QObject(parent),
        baseDirectory(defaultBaseDirectory),
        musicLibraryScanning(nullptr) {
    musicLibraryFiles = new xMusicLibraryFiles(this);
}

xMusicLibrary::~xMusicLibrary() noexcept {
    if (musicLibraryFiles) {
        delete musicLibraryFiles;
    }
}

void xMusicLibrary::setBaseDirectory(const std::filesystem::path& base) {
    // Update only if the given path is a directory
    if (std::filesystem::is_directory(base)) {
        baseDirectory = base;
        // Scan music library after updated director.
        // Rescan with same base intentional. Miniamal API.
        if (musicLibraryScanning) {
            musicLibraryScanning->quit();
            delete musicLibraryScanning;
        }
        musicLibraryScanning = new xMusicLibraryScanning(musicLibraryFiles, baseDirectory, this);
        connect(musicLibraryScanning, &xMusicLibraryScanning::finished, this, &xMusicLibrary::scanningFinished);
        connect(musicLibraryScanning, &xMusicLibraryScanning::scannedArtists, this, &xMusicLibrary::scannedArtists);
        musicLibraryScanning->start(QThread::IdlePriority);
    }
}

const std::filesystem::path& xMusicLibrary::getBaseDirectory() const {
    return baseDirectory;
}

void xMusicLibrary::scanForArtist(const QString& artist) {
    // Get list of albums for this artist.
    auto albumList = musicLibraryFiles->get(artist);
    // Update widget.
    emit scannedAlbums(albumList);
}

void xMusicLibrary::scanForArtistAndAlbum(const QString& artist, const QString& album) {
    QStringList trackList;
    try {
        auto trackPath = musicLibraryFiles->get(artist, album);
        // Remove album path use it to scan the directory.
        // We do not assume that there will be too many changes.
        auto albumPath = trackPath.front();
        trackPath.pop_front();
        if (trackPath.empty()) {
            qDebug() << "xMusicLibrary::scanAlbum: read files from disk";
            trackPath = xMusicLibrary::scanForAlbumTracks(albumPath);
            musicLibraryFiles->set(artist, album, trackPath);
            // Remove album path again.
            // We do not need it for track list.
            trackPath.pop_front();
        }
        // Generate list of tracks for artist/album
        for (const auto& track : trackPath) {
            trackList.push_back(QString::fromStdString(track.filename()));
            qDebug() << "xMusicLibrary::scanAlbum: track: " << QString::fromStdString(track.filename());
        }
    } catch (...) {
        // Clear list on error
        trackList.clear();
    }
    // Update widget
    emit scannedTracks(trackList);
}

void xMusicLibrary::scanningFinished() {
    if (musicLibraryScanning) {
        // Disconnect signals.
        disconnect(musicLibraryScanning, &xMusicLibraryScanning::finished, this, &xMusicLibrary::scanningFinished);
        disconnect(musicLibraryScanning, &xMusicLibraryScanning::scannedArtists, this, &xMusicLibrary::scannedArtists);
        // Delete scanning thread and reset pointer.
        delete musicLibraryScanning;
        musicLibraryScanning = nullptr;
    }
}
