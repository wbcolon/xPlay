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
                albumPos->second = scanForAlbumTracks(albumPath);
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
                albumPos->second = scanForAlbumTracks(albumPath);
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

std::vector<xMusicDirectory> xMusicLibraryFiles::getArtists() const {
    std::vector<xMusicDirectory> artists;
    musicFilesLock.lock();
    for (const auto& artist : musicFiles) {
        artists.push_back(artist.first);
    }
    musicFilesLock.unlock();
    std::sort(artists.begin(), artists.end());
    return artists;
}

std::vector<xMusicDirectory> xMusicLibraryFiles::getAlbums(const xMusicDirectory& artist) const {
    std::vector<xMusicDirectory> albums;
    musicFilesLock.lock();
    auto artistAlbums = musicFiles.find(artist);
    if (artistAlbums != musicFiles.end()) {
        for (const auto& album : artistAlbums->second) {
            albums.push_back(album.first);
        }
    }
    musicFilesLock.unlock();
    std::sort(albums.begin(), albums.end());
    return albums;
}

std::vector<xMusicFile*> xMusicLibraryFiles::getMusicFiles(const xMusicDirectory &artist,
                                                           const xMusicDirectory &album) const {
    std::vector<xMusicFile*> tracks;
    musicFilesLock.lock();
    auto artistAlbums = musicFiles.find(artist);
    if (artistAlbums != musicFiles.end()) {
        auto artistAlbumTracks = artistAlbums->second.find(album);
        if (artistAlbumTracks != artistAlbums->second.end()) {
            for (auto artistAlbumTrack : artistAlbumTracks->second) {
                if (!artistAlbumTrack->getTrackName().isEmpty()) {
                    tracks.push_back(artistAlbumTrack);
                }
            }
        }
    }
    std::sort(tracks.begin(), tracks.end(),
              [](xMusicFile* a, xMusicFile* b) { return (a->getTrackName() < b->getTrackName()); });
    musicFilesLock.unlock();
    return tracks;
}

xMusicFile* xMusicLibraryFiles::getMusicFile(const QString& artist, const QString& album, const QString& trackName) const {
    musicFilesLock.lock();
    auto artistAlbums = musicFiles.find(xMusicDirectory(artist));
    if (artistAlbums != musicFiles.end()) {
        auto artistAlbumTracks = artistAlbums->second.find(xMusicDirectory(album));
        if (artistAlbumTracks != artistAlbums->second.end()) {
            for (auto track : artistAlbumTracks->second) {
                if (track->getTrackName() == trackName) {
                    musicFilesLock.unlock();
                    return track;
                }
            }
        }
    }
    musicFilesLock.unlock();
    return nullptr;
}

std::uintmax_t xMusicLibraryFiles::getTotalSize(const xMusicDirectory& artist) const {
    std::uintmax_t totalSize = 0;
    musicFilesLock.lock();
    auto artistAlbums = musicFiles.find(xMusicDirectory(artist));
    if (artistAlbums != musicFiles.end()) {
        for (const auto& album : artistAlbums->second) {
            for (auto track : album.second) {
                totalSize += track->getFileSize();
            }
        }
    }
    musicFilesLock.unlock();
    return totalSize;
}

std::uintmax_t xMusicLibraryFiles::getTotalSize(const xMusicDirectory& artist, const xMusicDirectory& album) const {
    std::uintmax_t totalSize = 0;
    musicFilesLock.lock();
    auto artistAlbums = musicFiles.find(xMusicDirectory(artist));
    if (artistAlbums != musicFiles.end()) {
        auto artistAlbumTracks = artistAlbums->second.find(xMusicDirectory(album));
        if (artistAlbumTracks != artistAlbums->second.end()) {
            for (auto track : artistAlbumTracks->second) {
                totalSize += track->getFileSize();
            }
        }
    }
    musicFilesLock.unlock();
    return totalSize;
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
            auto trackFileObject = new xMusicFile(
                    trackFile.path(), trackFile.file_size(), albumPath->getArtist(), albumPath->getAlbum(),
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

void xMusicLibraryFiles::compare(const xMusicLibraryFiles* libraryFiles,
                                 std::list<xMusicDirectory>& missingArtists,
                                 std::list<xMusicDirectory>& additionalArtists,
                                 std::map<xMusicDirectory, std::list<xMusicDirectory>>& missingAlbums,
                                 std::map<xMusicDirectory, std::list<xMusicDirectory>>& additionalAlbums,
                                 std::list<xMusicFile*>& missingTracks,
                                 std::list<xMusicFile*>& additionalTracks,
                                 std::pair<std::list<xMusicFile*>, std::list<xMusicFile*>>& differentTracks) const {
    musicFilesLock.lock();
    // Compare the artists.
    std::list<xMusicDirectory> equalArtists;
    for (const auto& artist : musicFiles) {
        if (libraryFiles->musicFiles.find(artist.first) != libraryFiles->musicFiles.end()) {
            equalArtists.emplace_back(artist.first);
        } else {
            missingArtists.emplace_back(artist.first);
        }
    }
    for (const auto& artist : libraryFiles->musicFiles) {
        if (musicFiles.find(artist.first) == musicFiles.end()) {
            additionalArtists.emplace_back(artist.first);
        }
    }
    // Compare the albums for equal artists.
    std::map<xMusicDirectory, std::list<xMusicDirectory>> equalAlbums;
    for (const auto& artist : equalArtists) {
        try {
            std::list<xMusicDirectory> missing, additional, equal;

            const auto& musicFilesArtistAlbums { musicFiles.at(artist) };
            const auto& libraryMusicFilesArtistAlbums { libraryFiles->musicFiles.at(artist) };

            for (const auto& album : musicFilesArtistAlbums) {
                if (libraryMusicFilesArtistAlbums.find(album.first) != libraryMusicFilesArtistAlbums.end()) {
                    equal.emplace_back(album.first);
                } else {
                    missing.emplace_back(album.first);
                }
            }
            for (const auto& album : libraryMusicFilesArtistAlbums) {
                if (musicFilesArtistAlbums.find(album.first) == musicFilesArtistAlbums.end()) {
                    additional.emplace_back(album.first);
                }
            }

            if (!missing.empty()) {
                missingAlbums[artist] = missing;
            }
            if (!additional.empty()) {
                additionalAlbums[artist] = additional;
            }
            if (!equal.empty()) {
                equalAlbums[artist] = equal;
            }
        }
        catch (const std::out_of_range& e) {
            qCritical() << "Unable to access artist in music library files: " << e.what();
            missingArtists.clear();
            additionalArtists.clear();
            missingAlbums.clear();
            additionalAlbums.clear();
            missingTracks.clear();
            additionalTracks.clear();
            differentTracks.first.clear();
            differentTracks.second.clear();
            musicFilesLock.unlock();
            return;
        }
    }
    // Compare the tracks for the equal artist and albums.
    for (const auto& artist : equalAlbums) {
        std::map<xMusicDirectory, std::list<xMusicFile*>> missingArtistTracks, additionalArtistTracks, differentArtistTracks;
        for (const auto& album : artist.second) {
            try {
                listDifference(musicFiles.at(artist.first).at(album),
                               libraryFiles->musicFiles.at(artist.first).at(album),
                               missingTracks, additionalTracks, differentTracks);
            }
            catch (const std::out_of_range& e) {
                qCritical() << "Unable to access artist or album in music library files: " << e.what();
                missingArtists.clear();
                additionalArtists.clear();
                missingAlbums.clear();
                additionalAlbums.clear();
                missingTracks.clear();
                additionalTracks.clear();
                differentTracks.first.clear();
                differentTracks.second.clear();
                musicFilesLock.unlock();
                return;
            }
        }
    }
    musicFilesLock.unlock();
}

void xMusicLibraryFiles::compare(const xMusicLibraryFiles* libraryFiles,
                                 std::map<xMusicDirectory, std::map<xMusicDirectory, std::list<xMusicFile*>>>& equalTracks) const {
    musicFilesLock.lock();
    for (const auto& artist : libraryFiles->musicFiles) {
        auto musicFilesArtist = musicFiles.find(artist.first);
        if (musicFilesArtist != musicFiles.end()) {
            std::map<xMusicDirectory, std::list<xMusicFile*>> artistAlbums;
            for (const auto& album : artist.second) {
                auto musicFilesArtistAlbums = musicFilesArtist->second.find(album.first);
                if (musicFilesArtistAlbums != musicFilesArtist->second.end()) {
                    std::list<xMusicFile*> artistAlbumTracks;
                    for (auto track : album.second) {
                        for (auto musicFilesTrack : musicFilesArtistAlbums->second) {
                            if (track->equal(musicFilesTrack)) {
                                // Add left side music file to our list.
                                artistAlbumTracks.emplace_back(musicFilesTrack);
                                break;
                            }
                        }
                    }
                    // Add list only if we have equal tracks.
                    if (!artistAlbumTracks.empty()) {
                        artistAlbums[album.first] = artistAlbumTracks;
                    }
                }
            }
            // Add albums if we have any albums with equal tracks.
            if (!artistAlbums.empty()) {
                equalTracks[artist.first] = artistAlbums;
            }
        }
    }
    musicFilesLock.unlock();
}

void xMusicLibraryFiles::listDifference(const std::list<xMusicFile*>& a, const std::list<xMusicFile*>& b,
                                        std::list<xMusicFile*>& missing, std::list<xMusicFile*>& additional,
                                        std::pair<std::list<xMusicFile*>, std::list<xMusicFile*>>& different) {

    for (const auto& aEntry : a) {
        bool found = false;
        for (const auto& bEntry : b) {
            if (aEntry->equal(bEntry, false)) {
                if (!aEntry->equal(bEntry)) {
                    different.first.push_back(aEntry);
                    different.second.push_back(bEntry);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            missing.push_back(aEntry);
        }
    }
    for (const auto& bEntry : b) {
        bool found = false;
        for (const auto& aEntry : b) {
            if (bEntry->equal(aEntry, false)) {
                found = true;
                break;
            }
        }
        if (!found) {
            additional.push_back(bEntry);
        }
    }
}


/*
 * class xMusicLibraryScanning
 */
xMusicLibraryScanning::xMusicLibraryScanning(xMusicLibraryFiles* libraryFiles, QObject *parent):
        QThread(parent),
        musicLibraryFiles(libraryFiles) {
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
            if (!artistDir.is_directory()) {
                // No directory, no artist.
                continue;
            }
            auto artist = xMusicDirectory(artistDir);
            std::map<xMusicDirectory, std::list<xMusicFile*>> artistAlbumMap;
            // Read all albums for the given artist.
            for (const auto& albumDir : std::filesystem::directory_iterator(artistDir)) {
                if (!albumDir.is_directory()) {
                    // No directory, no album.
                    continue;
                }
                auto album = xMusicDirectory(albumDir);
                //qDebug() << "xMusicLibrary::scanLibrary: artist/album: " << artistName << "/" << albumName;
                // Do not scan the files in each directory as it is too costly.
                // Instead add the album path as first element element to track list.
                // It will be later used to read the tracks on demand.
                std::list<xMusicFile*> trackList;
                trackList.push_back(new xMusicFile(albumDir.path(), 0, artist.name(), album.name(), ""));
                artistAlbumMap[album] = trackList;
            }
            if (!artistAlbumMap.empty()) {
                musicLibraryFiles->set(artist, artistAlbumMap);
                artistList.emplace_back(artist);
            }
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
    musicLibraryFiles = new xMusicLibraryFiles();
    musicLibraryScanning = new xMusicLibraryScanning(musicLibraryFiles, this);
    connect(musicLibraryScanning, &xMusicLibraryScanning::finished, this, &xMusicLibrary::scanningFinished);
    connect(musicLibraryScanning, &xMusicLibraryScanning::scanningError, this, &xMusicLibrary::scanningError);
    connect(musicLibraryScanning, &xMusicLibraryScanning::scannedArtists, this, &xMusicLibrary::scannedArtists);
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
        // Did we find artist/album?
        if (!trackObjects.empty()) {
            // Remove album path (if it exists) used it to scan the directory.
            // We do not need it for track list.
            trackObjects.pop_front();
            // Generate list of tracks for artist/album
            for (const auto& track : trackObjects) {
                trackList.push_back(track);
                qDebug() << "xMusicLibrary::scanAlbum: track: " << track->getTrackName();
            }
        }
    } catch (...) {
        // Clear list on error
        trackList.clear();
    }
    trackList.sort([](xMusicFile* a, xMusicFile* b) { return (a->compareTrackName(b) < 0); });
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
            trackObjects.sort([](xMusicFile* a, xMusicFile* b) { return (a->compareTrackName(b) < 0); });
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
                trackObjects.sort([](xMusicFile* a, xMusicFile* b) { return (a->compareTrackName(b) < 0); });
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

const xMusicLibraryFiles* xMusicLibrary::getLibraryFiles() const {
    return musicLibraryFiles;
}