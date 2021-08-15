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
#include "xMusicFile.h"
#include "xPlayerConfiguration.h"
#include "xMusicDirectory.h"

#include <QDebug>

xMusicLibraryFilter::xMusicLibraryFilter():
        albumMatch(),
        albumNotMatch(),
        artistSearchMatch(),
        albumSearchMatch(),
        trackNameSearchMatch(),
        useArtistAlbumMatch(false),
        artistAlbumMatch(),
        artistAlbumNotMatch(false) {
}

bool xMusicLibraryFilter::hasArtistFilter() const {
    return ((!artistSearchMatch.isEmpty()) || (useArtistAlbumMatch));
}

bool xMusicLibraryFilter::hasAlbumFilter() const {
    return ((!albumMatch.isEmpty()) || (!albumNotMatch.isEmpty()) ||
            (!albumSearchMatch.isEmpty()) || (useArtistAlbumMatch));
}

bool xMusicLibraryFilter::hasTrackNameFilter() const {
    return (!trackNameSearchMatch.isEmpty());
}

bool xMusicLibraryFilter::isMatchingArtist(const QString& artist) const {
    return artist.contains(artistSearchMatch, Qt::CaseInsensitive);
}

bool xMusicLibraryFilter::isMatchingAlbum(const QString& album) const {
    if (!album.contains(albumSearchMatch, Qt::CaseInsensitive)) {
        return false;
    }
    if ((!albumNotMatch.isEmpty()) &&
        (std::any_of(albumNotMatch.begin(), albumNotMatch.end(), [&album](const QString& notMatch) {
            return album.contains(notMatch, Qt::CaseInsensitive);
        }))) {
        return false;
    }

    return (std::all_of(albumMatch.begin(), albumMatch.end(), [&album](const QString& match) {
        return album.contains(match, Qt::CaseInsensitive);
    }));
}

bool xMusicLibraryFilter::isMatchingTrackName(const QString& trackName) const {
    return trackName.contains(trackNameSearchMatch, Qt::CaseInsensitive);
}

bool xMusicLibraryFilter::isMatchingDatabaseArtistAndAlbum(const QString& artist, const QString& album) const {
    // No database matching im map is empty.
    if (!useArtistAlbumMatch) {
        return true;
    }
    auto artistPos = artistAlbumMatch.find(artist);
    if (artistPos != artistAlbumMatch.end()) {
        if (artistPos->second.find(album) != artistPos->second.end()) {
            return !artistAlbumNotMatch;
        }
    }
    return artistAlbumNotMatch;
}

void xMusicLibraryFilter::setAlbumMatch(const QStringList& match, const QStringList& notMatch) {
    albumMatch = match;
    albumNotMatch = notMatch;
}

void xMusicLibraryFilter::setSearchMatch(const std::tuple<QString,QString,QString>& match) {
    artistSearchMatch = std::get<0>(match);
    albumSearchMatch = std::get<1>(match);
    trackNameSearchMatch = std::get<2>(match);
}

void xMusicLibraryFilter::setDatabaseMatch(const std::map<QString,std::set<QString>>& databaseMatch, bool databaseNotMatch) {
    useArtistAlbumMatch = true;
    artistAlbumMatch = databaseMatch;
    artistAlbumNotMatch = databaseNotMatch;
}

void xMusicLibraryFilter::clearAlbumMatch() {
    albumMatch.clear();
    albumNotMatch.clear();
}

void xMusicLibraryFilter::clearDatabaseMatch() {
    useArtistAlbumMatch = false;
    artistAlbumMatch.clear();
    artistAlbumNotMatch = false;
}

void xMusicLibraryFilter::clearSearchMatch() {
    artistSearchMatch.clear();
    albumSearchMatch.clear();
    trackNameSearchMatch.clear();
}

void xMusicLibraryFilter::clearMatch() {
    clearAlbumMatch();
    clearSearchMatch();
    clearDatabaseMatch();
}


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

xMusicLibraryFiles::~xMusicLibraryFiles() noexcept {
    clear();
}

xMusicLibraryFiles* xMusicLibraryFiles::files() {
    // Create and return singleton.
    if (musicLibraryFiles == nullptr) {
        musicLibraryFiles = new xMusicLibraryFiles;
    }
    return musicLibraryFiles;
}

void xMusicLibraryFiles::set(const xMusicDirectory& artist, const std::map<xMusicDirectory,std::list<xMusicFile*>>& albumTracks) {
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        artistPos->second = albumTracks;
    } else {
        musicFiles[artist] = albumTracks;
    }
    musicFilesLock.unlock();
}

void xMusicLibraryFiles::set(const xMusicDirectory& artist, const xMusicDirectory& album, const std::list<xMusicFile*>& tracks) {
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
        std::map<xMusicDirectory, std::list<xMusicFile*>> albumMap;
        albumMap[album] = tracks;
        musicFiles[artist] = albumMap;
    }
    musicFilesLock.unlock();
}

bool xMusicLibraryFiles::isEmpty() const {
    // No need to lock here. Only read-only check for empty state.
    return musicFiles.empty();
}

void xMusicLibraryFiles::clear() {
    musicFilesLock.lock();
    // Remove all the music files.
    for (auto& artist : musicFiles) {
        for (auto& album : artist.second) {
            // Delete all music file objects.
            for (auto& track : album.second) {
                delete track;
            }
            album.second.clear();
        }
        artist.second.clear();
    }
    musicFiles.clear();
    musicFilesLock.unlock();
}

std::list<xMusicFile*> xMusicLibraryFiles::get(const xMusicDirectory& artist, const xMusicDirectory& album) {
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        auto albumPos = artistPos->second.find(album);
        if (albumPos != artistPos->second.end()) {
            auto trackPath = albumPos->second;
            // Remove album path use it to scan the directory.
            // We do not assume that there will be too many changes.
            auto albumPath = trackPath.front();
            trackPath.pop_front();
            if (trackPath.empty()) {
                qDebug() << "xMusicLibrary::scanAlbum: read files from disk";
                albumPos->second = musicLibraryFiles->scanForAlbumTracks(albumPath);
            }
            musicFilesLock.unlock();
            return albumPos->second;
        }
    }
    musicFilesLock.unlock();
    return {};
}

std::list<xMusicFile*> xMusicLibraryFiles::get(const xMusicDirectory& artist, const xMusicDirectory& album, const xMusicLibraryFilter& filter) {
    // Check if we really need to apply a filter here.
    if (!filter.hasTrackNameFilter()) {
        return get(artist, album);
    }
    // Filter in place.
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        auto albumPos = artistPos->second.find(album);
        if (albumPos != artistPos->second.end()) {
            auto trackPath = albumPos->second;
            // Remove album path use it to scan the directory.
            // We do not assume that there will be too many changes.
            auto albumPath = trackPath.front();
            trackPath.pop_front();
            if (trackPath.empty()) {
                qDebug() << "xMusicLibrary::scanAlbum: read files from disk";
                albumPos->second = musicLibraryFiles->scanForAlbumTracks(albumPath);
            }
            for (const auto& track : albumPos->second) {
                if (filter.isMatchingTrackName(track->getTrackName())) {
                    musicFilesLock.unlock();
                    // Match found. Return entire list.
                    return albumPos->second;
                }
            }
            musicFilesLock.unlock();
            // No matches found. Return empty list.
            return {};
        }
    }
    musicFilesLock.unlock();
    return {};
}

std::list<xMusicDirectory> xMusicLibraryFiles::get(const xMusicDirectory& artist) const {
    std::list<xMusicDirectory> albumList;
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        for (const auto& album : artistPos->second) {
            albumList.emplace_back(album.first);
        }
    }
    musicFilesLock.unlock();
    return albumList;
}

std::list<xMusicDirectory> xMusicLibraryFiles::get(const xMusicDirectory& artist, const xMusicLibraryFilter& filter) const {
    // Check if we really need to apply a filter here.
    if (!filter.hasAlbumFilter()) {
        return get(artist);
    }
    // Filter in place.
    std::list<xMusicDirectory> albumList;
    musicFilesLock.lock();
    auto artistPos = musicFiles.find(artist);
    if (artistPos != musicFiles.end()) {
        for (const auto& album : artistPos->second) {
            if ((filter.isMatchingAlbum(album.first.name())) &&
                (filter.isMatchingDatabaseArtistAndAlbum(artistPos->first.name(), album.first.name()))) {
                albumList.emplace_back(album.first);
            }
        }
    }
    musicFilesLock.unlock();
    return albumList;
}

std::list<xMusicDirectory> xMusicLibraryFiles::get(const xMusicLibraryFilter& filter) const {
    std::list<xMusicDirectory> artistList;
    musicFilesLock.lock();
    for (const auto& artist : musicFiles) {
        if (filter.isMatchingArtist(artist.first.name())) {
            artistList.emplace_back(artist.first);
        }
    }
    musicFilesLock.unlock();
    return artistList;
}

std::list<std::tuple<xMusicDirectory, xMusicDirectory, xMusicFile*>> xMusicLibraryFiles::get() const {
    std::list<std::tuple<xMusicDirectory, xMusicDirectory, xMusicFile*>> artistAlbumList;
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
    qDebug() << "xMusicLibraryFiles: skipping: " << QString::fromStdString(file.generic_string());
    return false;
}

std::list<xMusicFile*> xMusicLibraryFiles::scanForAlbumTracks(xMusicFile* albumPath) {
    std::list<xMusicFile*> trackList;
    // Add the album path itself
    trackList.push_back(albumPath);
    // Add all files within the album path
    for (const auto& trackFile : std::filesystem::directory_iterator(albumPath->getFilePath())) {
        if (isMusicFile(trackFile)) {
            auto trackFileObject = new xMusicFile(trackFile.path(), albumPath->getArtist(), albumPath->getAlbum(),
                                        QString::fromStdString(trackFile.path().filename()));
            trackList.push_back(trackFileObject);
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
        requestInterruption();
        wait();
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
        if (currentThread()->isInterruptionRequested()) {
            return;
        }
    }
    qDebug() << "xMusicLibraryScanning: finished";
}

void xMusicLibraryScanning::scan() {
    // Cleanup before scanning path
    musicLibraryFiles->clear();
    try {
        std::list<xMusicDirectory> artistList;
        for (const auto& artistDir : std::filesystem::directory_iterator(baseDirectory)) {
            const auto& artistPath = artistDir.path();
            if (!std::filesystem::is_directory(artistPath)) {
                // No directory, no artist.
                continue;
            }
            auto artistName = QString::fromStdString(artistPath.filename());
            auto artist = xMusicDirectory(artistName, artistDir.last_write_time());
            std::map<xMusicDirectory, std::list<xMusicFile*>> artistAlbumMap;
            // Read all albums for the given artist.
            for (const auto& albumDir : std::filesystem::directory_iterator(artistPath)) {
                const auto& albumPath = albumDir.path();
                if (!std::filesystem::is_directory(albumPath)) {
                    // No directory, no album.
                    continue;
                }
                auto albumName = QString::fromStdString(albumPath.filename());
                auto album = xMusicDirectory(albumName, albumDir.last_write_time());
                //qDebug() << "xMusicLibrary::scanLibrary: artist/album: " << artistName << "/" << albumName;
                // Do not scan the files in each directory as it is too costly.
                // Instead add the album path as first element element to track list.
                // It will be later used to read the tracks on demand.
                std::list<xMusicFile*> trackList;
                trackList.push_back(new xMusicFile(albumPath, artistName, albumName, ""));
                artistAlbumMap[album] = trackList;
            }
            musicLibraryFiles->set(artist, artistAlbumMap);
            artistList.emplace_back(artist);
            if (currentThread()->isInterruptionRequested()) {
                return;
            }
        }
        // Sort the artists.
        artistList.sort();
        // Update widget
        emit scannedArtists(artistList);
    } catch (...) {
        // Error scanning. Structure may not be as expected.
        // Clear everything currently scanned.
        musicLibraryFiles->clear();
    }

    if (musicLibraryFiles->isEmpty()) {
        qCritical() << "Scanning error. Library is empty...";
        emit scanningError();
    }
}


/*
 * class xMusicLibraryScanning
 */

// Use "/tmp" as default path
const std::string defaultBaseDirectory{ "/tmp" }; // NOLINT

xMusicLibrary::xMusicLibrary(QObject* parent):
        QObject(parent),
        baseDirectory(defaultBaseDirectory) {
    musicLibraryScanning = new xMusicLibraryScanning(this);
    connect(musicLibraryScanning, &xMusicLibraryScanning::finished, this, &xMusicLibrary::scanningFinished);
    connect(musicLibraryScanning, &xMusicLibraryScanning::scanningError, this, &xMusicLibrary::scanningError);
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
            musicLibraryScanning->requestInterruption();
            musicLibraryScanning->wait();
        }
        musicLibraryScanning->setBaseDirectory(base);
        musicLibraryScanning->start(QThread::IdlePriority);
    } else {
        emit scanningError();
    }
}

const std::filesystem::path& xMusicLibrary::getBaseDirectory() const {
    return baseDirectory;
}

xMusicFile* xMusicLibrary::getMusicFile(const QString& artist, const QString& album, const QString& trackName) {
    auto trackObjects = musicLibraryFiles->get(xMusicDirectory(artist), xMusicDirectory(album));
    trackObjects.pop_front();
    for (const auto& track : trackObjects) {
        if (track->getTrackName() == trackName) {
            return track;
        }
    }
    return nullptr;
}

void xMusicLibrary::cleanup() {
    if ((musicLibraryScanning) && (musicLibraryScanning->isRunning())) {
        musicLibraryScanning->requestInterruption();
        musicLibraryScanning->wait();
    }
    musicLibraryFiles->clear();
}

void xMusicLibrary::scan(const xMusicLibraryFilter& filter) {
    auto artistList = musicLibraryFiles->get(filter);
    if ((filter.hasAlbumFilter()) || (filter.hasTrackNameFilter())) {
        for (auto i = artistList.begin(); i != artistList.end();) {
            auto removeArtist = true;
            auto albumList = musicLibraryFiles->get(*i, filter);
            if (!albumList.empty()) {
                // track filtering can be time consuming.
                if (filter.hasTrackNameFilter()) {
                    for (const auto& album : albumList) {
                        if (!musicLibraryFiles->get(*i, album, filter).empty()) {
                            removeArtist = false;
                            break;
                        }
                    }
                } else {
                    removeArtist = false;
                }
            }
            if (removeArtist) {
                i = artistList.erase(i);
            } else {
                ++i;
            }
        }
    }
    emit scannedArtists(artistList);
}

void xMusicLibrary::scanForArtist(const xMusicDirectory& artist) {
    // Get list of albums for this artist.
    auto albumList = musicLibraryFiles->get(artist);
    // Update widget.
    emit scannedAlbums(albumList);
}

void xMusicLibrary::scanForArtist(const xMusicDirectory& artist, const xMusicLibraryFilter& filter) {
    // Get list of albums for this artist.
    auto albumList = musicLibraryFiles->get(artist, filter);
    if (filter.hasTrackNameFilter()) {
        for (auto i = albumList.begin(); i != albumList.end();) {
            // Remove album from list if no matching tracks are found.
            if (musicLibraryFiles->get(artist, *i, filter).empty()) {
                i = albumList.erase(i);
            } else {
                ++i;
            }
        }
    }
    // Update widget.
    emit scannedAlbums(albumList);
}


void xMusicLibrary::scanForArtistAndAlbum(const xMusicDirectory& artist, const xMusicDirectory& album) {
    std::list<xMusicFile*> trackList;
    try {
        auto trackObjects = musicLibraryFiles->get(artist, album);
        // Remove album path used it to scan the directory.
        // We do not need it for track list.
        trackObjects.pop_front();
        // Generate list of tracks for artist/album
        for (const auto& track : trackObjects) {
            trackList.push_back(track);
            qDebug() << "xMusicLibrary::scanAlbum: track: " << track->getTrackName();
        }
    } catch (...) {
        // Clear list on error
        trackList.clear();
    }
    trackList.sort([](xMusicFile* a, xMusicFile* b) { return (a->getTrackName() < b->getTrackName()); });
    // Update widget
    emit scannedTracks(trackList);
}

void xMusicLibrary::scanAllAlbumsForArtist(const xMusicDirectory& artist) {
    QList<std::pair<QString,std::vector<xMusicFile*>>> albumTracks;
    // Retrieve the albums tracks.
    getAllAlbumsForArtist(artist, albumTracks);
    // Update widget
    emit scannedAllAlbumTracks(artist.name(), albumTracks);
}

void xMusicLibrary::scanAllAlbumsForArtist(const xMusicDirectory& artist, const xMusicLibraryFilter& filter) {
    QList<std::pair<QString,std::vector<xMusicFile*>>> albumTracks;
    // Retrieve the albums tracks.
    getAllAlbumsForArtist(artist, albumTracks, filter);
    // Update widget
    emit scannedAllAlbumTracks(artist.name(), albumTracks);
}

void xMusicLibrary::scanAllAlbumsForListArtists(const std::list<xMusicDirectory>& listArtists) {
    QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicFile*>>>>> listTracks;
    try {
        // Retrieve list of albums and and sort them.
        for (const auto& artist : listArtists) {
            QList<std::pair<QString,std::vector<xMusicFile*>>> albumTracks;
            getAllAlbumsForArtist(artist, albumTracks);
            listTracks.push_back(std::make_pair(artist.name(), albumTracks));
        }
    } catch (...) {
        // Clear list on error
        listTracks.clear();
    }
    // Update widget
    emit scannedListArtistsAllAlbumTracks(listTracks);
}

void xMusicLibrary::scanAllAlbumsForListArtists(const std::list<xMusicDirectory>& listArtists, const xMusicLibraryFilter& filter) {
    QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicFile*>>>>> listTracks;
    try {
        // Retrieve list of albums and and sort them.
        for (const auto& artist : listArtists) {
            QList<std::pair<QString,std::vector<xMusicFile*>>> albumTracks;
            getAllAlbumsForArtist(artist, albumTracks, filter);
            if (!albumTracks.isEmpty()) {
                listTracks.push_back(std::make_pair(artist.name(), albumTracks));
            }
        }
    } catch (...) {
        // Clear list on error
        listTracks.clear();
    }
    // Update widget
    emit scannedListArtistsAllAlbumTracks(listTracks);
}

void xMusicLibrary::scanForUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries) {
    std::list<std::tuple<QString, QString, QString>> unknownDatabaseEntries;
    QString currentArtist, currentAlbum;
    std::list<xMusicFile*> currentTracks;
    for (const auto& entry : listEntries) {
        // We need to cache the entries if possible. List should be sorted by artist and album.
        if ((currentArtist != std::get<0>(entry)) || (currentAlbum != std::get<1>(entry))) {
            currentArtist = std::get<0>(entry);
            currentAlbum = std::get<1>(entry);
            currentTracks = musicLibraryFiles->get(xMusicDirectory(currentArtist), xMusicDirectory(currentAlbum));
        }
        // We need to match the relative path generated by artist/album/track with the absolute paths stored.
        auto entryTrack = std::get<2>(entry).toStdString();
        auto currentTracksPos = std::find_if(currentTracks.begin(), currentTracks.end(),
                                             [&entryTrack](xMusicFile* trackObject) {
                                                 return (trackObject->getFilePath().filename() == entryTrack);
                                             });
        // If we do not find the track then we append the entry to the unknown list.
        if (currentTracksPos == currentTracks.end()) {
            unknownDatabaseEntries.emplace_back(entry);
            qDebug() << "Unknown entry found: " << std::get<0>(entry) << "," << std::get<1>(entry) << "," << std::get<2>(entry);
        }
    }
    // Send the results.
    emit scannedUnknownEntries(unknownDatabaseEntries);
}

void xMusicLibrary::getAllAlbumsForArtist(const xMusicDirectory& artist,
                                          QList<std::pair<QString, std::vector<xMusicFile*>>>& albumTracks) {
    try {
        // Clear list.
        albumTracks.clear();
        // Retrieve list of albums and and sort them.
        auto albumList = musicLibraryFiles->get(artist);
        albumList.sort();
        for (const auto& album : albumList) {
            auto trackObjects = musicLibraryFiles->get(artist, album);
            trackObjects.pop_front();
            trackObjects.sort([](xMusicFile* a, xMusicFile* b) { return (a->getTrackName() < b->getTrackName()); });
            std::vector<xMusicFile*> albumTrackObjects;
            for (const auto& track : trackObjects) {
                albumTrackObjects.emplace_back(track);
            }
            albumTracks.push_back(std::make_pair(album.name(), albumTrackObjects));
        }
    } catch (...) {
        // Clear list on error
        albumTracks.clear();
    }

}

void xMusicLibrary::getAllAlbumsForArtist(const xMusicDirectory& artist,
                                          QList<std::pair<QString, std::vector<xMusicFile*>>>& albumTracks,
                                          const xMusicLibraryFilter& filter) {
    // Call version without filter if we do not need them.
    if ((!filter.hasAlbumFilter()) && (!filter.hasTrackNameFilter())) {
        getAllAlbumsForArtist(artist, albumTracks);
        return;
    }
    try {
        // Clear list.
        albumTracks.clear();
        // Retrieve list of albums and sort them.
        auto albumList = musicLibraryFiles->get(artist, filter);
        albumList.sort();
        for (const auto& album : albumList) {
            auto trackObjects = musicLibraryFiles->get(artist, album, filter);
            // Only add album with tracks if the track filter matches.
            if (!trackObjects.empty()) {
                trackObjects.pop_front();
                trackObjects.sort([](xMusicFile* a, xMusicFile* b) { return (a->getTrackName() < b->getTrackName()); });
                std::vector<xMusicFile*> albumTrackObjects;
                for (const auto& track : trackObjects) {
                    albumTrackObjects.emplace_back(track);
                }
                albumTracks.push_back(std::make_pair(album.name(), albumTrackObjects));
            }
        }
    } catch (...) {
        // Clear list on error
        albumTracks.clear();
    }
}