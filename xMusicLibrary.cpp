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
#include "xPlayerConfiguration.h"

#include <QDebug>


// singleton
xMusicLibraryFiles* xMusicLibraryFiles::musicLibraryFiles = nullptr;

/*
 * class xMusicLibraryFiles
 */
xMusicLibraryFiles::xMusicLibraryFiles(QObject *parent):
        QObject(parent) {
    // Connect to configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMusicLibraryExtensions,
            this, &xMusicLibraryFiles::updateMusicExtensions);
}

xMusicLibraryFiles* xMusicLibraryFiles::files() {
    // Create and return singleton.
    if (musicLibraryFiles == nullptr) {
        musicLibraryFiles = new xMusicLibraryFiles;
    }
    return musicLibraryFiles;
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

bool xMusicLibraryFiles::isMusicFile(const std::filesystem::path& file) {
    if (std::filesystem::is_regular_file(file)) {
        auto extension = QString::fromStdString(file.extension().string());
        for (const auto& musicExtension : musicExtensions) {
            if (extension.startsWith(musicExtension, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    qDebug() << "xMovieLibraryFiles: skipping: " << QString::fromStdString(file.generic_string());
    return false;
}

std::list<std::filesystem::path> xMusicLibraryFiles::scanForAlbumTracks(const std::filesystem::path& albumPath) {
    std::list<std::filesystem::path> trackList;
    // Add the album path itself
    trackList.push_back(albumPath);
    // Add all files in the album path
    for (const auto& trackFile : std::filesystem::directory_iterator(albumPath)) {
        if (isMusicFile(trackFile)) {
            trackList.push_back(trackFile.path());
        }
    }
    return trackList;
}

void xMusicLibraryFiles::updateMusicExtensions() {
    musicExtensions = xPlayerConfiguration::configuration()->getMusicLibraryExtensionList();
    qDebug() << "xMusicLibraryFiles: extensions: " << musicExtensions;
}


/*
 * class xMusicLibraryScanning
 */
xMusicLibraryScanning::xMusicLibraryScanning(QObject *parent):
        QThread(parent) {
    musicLibraryFiles = xMusicLibraryFiles::files();
}

xMusicLibraryScanning::~xMusicLibraryScanning() noexcept {
    if (isRunning()) {
        quit();
    }
}

void xMusicLibraryScanning::setBaseDirectory(const std::filesystem::path& dir) {
    baseDirectory = dir;
}

void xMusicLibraryScanning::run() {
    qDebug() << "xMusicLibraryScanning: scanning base structure";
    // First scan the basic structure.
    scan();
    qDebug() << "xMusicLibraryScanning: scanning each album";
    // Second scan for tracks.
    auto artistAlbumList = musicLibraryFiles->get();
    for (auto entry : artistAlbumList) {
        musicLibraryFiles->set(std::get<0>(entry), std::get<1>(entry),
                musicLibraryFiles->scanForAlbumTracks(std::get<2>(entry)));
    }
    qDebug() << "xMusicLibraryScanning: finished";
}

void xMusicLibraryScanning::scan() {
    // Cleanup before scanning path
    musicLibraryFiles->clear();
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
            musicLibraryFiles->set(artistName, artistAlbumMap);
            artistList.push_back(artistName);
        }
        // Update widget
        emit scannedArtists(artistList);
    } catch (...) {
        // Error scanning. Structure may not be as expected.
        // Clear everything currently scanned.
        musicLibraryFiles->clear();
    }
}


/*
 * class xMusicLibraryScanning
 */

// Use "/tmp" as default path
const std::string defaultBaseDirectory{ "/tmp" };

xMusicLibrary::xMusicLibrary(QObject* parent):
        QObject(parent),
        baseDirectory(defaultBaseDirectory) {
    musicLibraryScanning = new xMusicLibraryScanning(this);
    connect(musicLibraryScanning, &xMusicLibraryScanning::finished, this, &xMusicLibrary::scanningFinished);
    connect(musicLibraryScanning, &xMusicLibraryScanning::scannedArtists, this, &xMusicLibrary::scannedArtists);
    musicLibraryFiles = xMusicLibraryFiles::files();
}

void xMusicLibrary::setBaseDirectory(const std::filesystem::path& base) {
    // Update only if the given path is a directory
    if (std::filesystem::is_directory(base)) {
        baseDirectory = base;
        // Scan music library after updated directory.
        // Rescan with same base intentional.
        if (musicLibraryScanning->isRunning()) {
            musicLibraryScanning->quit();
        }
        musicLibraryScanning->setBaseDirectory(base);
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
            trackPath = musicLibraryFiles->scanForAlbumTracks(albumPath);
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
    qDebug() << "xMusicLibrary: scanning finished.";
}
