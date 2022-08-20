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
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"

#include <QDebug>

#include <filesystem>


xMusicLibrary::xMusicLibrary(QObject* parent):
        xMusicLibraryEntry(parent),
        musicLibraryScanning(nullptr) {
}

void xMusicLibrary::setPath(const std::filesystem::path& base) {
    // Update only if the given path is a directory
    if (std::filesystem::is_directory(base)) {
        entryPath = base;
        scan();
    } else {
        emit scanningError();
    }
}

[[nodiscard]] std::vector<xMusicLibraryArtistEntry*> xMusicLibrary::getArtists() {
    return musicLibraryArtists;
}

void xMusicLibrary::cleanup() {
    musicLibraryLock.lock();
    musicLibraryArtists.clear();
    musicLibraryArtistsMap.clear();
    musicLibraryLock.unlock();
}

void xMusicLibrary::scan() {
    if (!entryPath.empty()) {
        if (musicLibraryScanning && musicLibraryScanning->isRunning()) {
            musicLibraryScanning->requestInterruption();
            musicLibraryScanning->wait();
            cleanup();
        }
        musicLibraryScanning = QThread::create([this]() {
            scanThread();
        });
        connect(musicLibraryScanning, &QThread::finished, this, &xMusicLibrary::scanningFinished);
        musicLibraryScanning->start(QThread::IdlePriority);
    }
}

void xMusicLibrary::scan(const xMusicLibraryFilter& filter) {
    auto filteredArtists = filterArtists(musicLibraryArtists, filter);
    if ((filter.hasAlbumFilter()) || (filter.hasTrackNameFilter())) {
        for (auto i = filteredArtists.begin(); i != filteredArtists.end();) {
            auto removeArtist = true;
            auto filteredAlbums = filterAlbums(*i, filter);
            if (!filteredAlbums.empty()) {
                // track filtering can be time consuming.
                if (filter.hasTrackNameFilter()) {
                    for (auto album : filteredAlbums) {
                        if (!filterTracks(album, filter).empty()) {
                            removeArtist = false;
                            break;
                        }
                    }
                } else {
                    removeArtist = false;
                }
            }
            if (removeArtist) {
                i = filteredArtists.erase(i);
            } else {
                ++i;
            }
        }
    }
    emit scannedArtists(filteredArtists);
}

void xMusicLibrary::scanForArtist(const QString& artistName) {
    // Get list of albums for this artist.
    musicLibraryLock.lock();
    auto artistAlbum = musicLibraryArtistsMap.find(artistName);
    if (artistAlbum != musicLibraryArtistsMap.end()) {
        emit scannedAlbums(artistAlbum->second->getAlbums());
    } else {
        emit scannedAlbums({});
    }
    musicLibraryLock.unlock();
}

void xMusicLibrary::scanForArtist(const QString& artistName, const xMusicLibraryFilter& filter) {
    // Get filtered list of albums for this artist.
    std::vector<xMusicLibraryAlbumEntry*> filteredAlbums;
    musicLibraryLock.lock();
    auto artistAlbum = musicLibraryArtistsMap.find(artistName);
    if (artistAlbum != musicLibraryArtistsMap.end()) {
        filteredAlbums = filterAlbums(artistAlbum->second, filter);
        for (auto i = filteredAlbums.begin(); i != filteredAlbums.end();) {
            // Scan album tracks.
            (*i)->scan();
            // Remove album from list if no matching tracks are found.
            if (filterTracks(*i, filter).empty()) {
                i = filteredAlbums.erase(i);
            } else {
                ++i;
            }
        }
    }
    musicLibraryLock.unlock();
    // Update widget.
    emit scannedAlbums(filteredAlbums);
}

void xMusicLibrary::scanForArtistAndAlbum(const QString& artistName, const QString& albumName) {
    musicLibraryLock.lock();
    std::vector<xMusicLibraryTrackEntry*> artistAlbumTracks;
    try {
        auto artist = musicLibraryArtistsMap.find(artistName);
        if (artist != musicLibraryArtistsMap.end()) {
            auto artistAlbum = artist->second->getAlbum(albumName);
            // Did we find artist/album?
            if (artistAlbum) {
                artistAlbum->scan();
                artistAlbumTracks = artistAlbum->getTracks();
            }
        }
    } catch (...) {
        // Clear list on error
        artistAlbumTracks.clear();
    }
    musicLibraryLock.unlock();
    // Update widget
    emit scannedTracks(artistAlbumTracks);
}

void xMusicLibrary::scanAllAlbumsForArtist(const QString& artistName) {
    musicLibraryLock.lock();
    QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>> artistAlbumsTracks;
    try {
        auto artist = musicLibraryArtistsMap[artistName];
        if (artist) {
            auto artistAlbums = artist->getAlbums();
            for (auto album : artistAlbums) {
                album->scan();
                artistAlbumsTracks.push_back(std::make_pair(album->getAlbumName(), album->getTracks()));
            }
        }
    } catch (...) {
        artistAlbumsTracks.clear();
    }
    musicLibraryLock.unlock();
    // Update widget
    emit scannedAllAlbumTracks(artistName, artistAlbumsTracks);
}

void xMusicLibrary::scanAllAlbumsForArtist(const QString& artistName, const xMusicLibraryFilter& filter) {
    musicLibraryLock.lock();
    QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>> artistAlbumsTracks;
    try {
        auto artist = musicLibraryArtistsMap[artistName];
        if (artist) {
            auto artistAlbums = filterAlbums(artist, filter);
            for (auto album : artistAlbums) {
                auto albumTracks = filterTracks(album, filter);
                if (!albumTracks.empty()) {
                    artistAlbumsTracks.push_back(std::make_pair(album->getAlbumName(), album->getTracks()));
                }
            }
        }
    } catch (...) {
        artistAlbumsTracks.clear();
    }
    musicLibraryLock.unlock();
    // Update widget
    emit scannedAllAlbumTracks(artistName, artistAlbumsTracks);
}

void xMusicLibrary::scanAllAlbumsForListArtists(const QStringList& listArtists) {
    musicLibraryLock.lock();
    QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>> listTracks;
    try {
        for (const auto& artistName : listArtists) {
            QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>> artistAlbumsTracks;
            auto artist = musicLibraryArtistsMap.find(artistName);
            if (artist != musicLibraryArtistsMap.end()) {
                auto artistAlbums = artist->second->getAlbums();
                for (auto album: artistAlbums) {
                    album->scan();
                    artistAlbumsTracks.push_back(std::make_pair(album->getAlbumName(), album->getTracks()));
                }
            }
            // Add empty list if artist name is invalid.
            listTracks.push_back(std::make_pair(artistName, artistAlbumsTracks));
        }
    } catch (...) {
        // Clear everything on exception.
        qCritical() << "xMusicLibrary::scanAllAlbumsForListArtists: scanning error.";
        listTracks.clear();
    }
    musicLibraryLock.unlock();
    // Update widget
    emit scannedListArtistsAllAlbumTracks(listTracks);
}

void xMusicLibrary::scanAllAlbumsForListArtists(const QStringList& listArtists, const xMusicLibraryFilter& filter) {
    musicLibraryLock.lock();
    QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>> listTracks;
    try {
        for (const auto& artistName : listArtists) {
            auto artist = musicLibraryArtistsMap[artistName];
            if (artist) {
                QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>> artistAlbumsTracks;
                auto artistAlbums = filterAlbums(artist, filter);
                for (auto album: artistAlbums) {
                    auto albumTracks = filterTracks(album, filter);
                    if (!albumTracks.empty()) {
                        artistAlbumsTracks.push_back(std::make_pair(album->getAlbumName(), album->getTracks()));
                    }
                }
                if (!artistAlbumsTracks.empty()) {
                    listTracks.push_back(std::make_pair(artistName, artistAlbumsTracks));
                }
            }
        }
    } catch (...) {
        // Clear everything on exception.
        qCritical() << "xMusicLibrary::scanAllAlbumsForListArtists: scanning error.";
        listTracks.clear();
    }
    musicLibraryLock.unlock();
    // Update widget
    emit scannedListArtistsAllAlbumTracks(listTracks);
}

xMusicLibraryTrackEntry* xMusicLibrary::getTrackEntry(const QString& artistName, const QString& albumName, const QString& trackName) {
    musicLibraryLock.lock();
    try {
        auto artistAlbum = musicLibraryArtistsMap[artistName]->getAlbum(albumName);
        if (artistAlbum) {
            artistAlbum->scan();
            for (auto track : artistAlbum->getTracks()) {
                if (track->getTrackName() == trackName) {
                    musicLibraryLock.unlock();
                    return track;
                }
            }
        }
    } catch (...) {
        // ignore any errors.
    }
    musicLibraryLock.unlock();
    return nullptr;
}

void xMusicLibrary::scanForUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries) {
    std::list<std::tuple<QString, QString, QString>> unknownDatabaseEntries;
    QString currentArtist, currentAlbum;
    std::vector<xMusicLibraryTrackEntry*> currentTracks;
    for (const auto& entry : listEntries) {
        // We need to cache the entries if possible. List should be sorted by artist and album.
        if ((currentArtist != std::get<0>(entry)) || (currentAlbum != std::get<1>(entry))) {
            currentArtist = std::get<0>(entry);
            currentAlbum = std::get<1>(entry);
            auto artist = musicLibraryArtistsMap.find(currentArtist);
            if (artist != musicLibraryArtistsMap.end()) {
                auto album = artist->second->getAlbum(currentAlbum);
                if (album) {
                    currentTracks = album->getTracks();
                }
            }
        }
        // We need to match the relative path generated by artist/album/track with the absolute paths stored.
        auto entryTrack = std::get<2>(entry).toStdString();
        auto currentTracksPos = std::find_if(currentTracks.begin(), currentTracks.end(),
                                             [&entryTrack](xMusicLibraryTrackEntry* trackObject) {
                                                 return (trackObject->getPath().filename() == entryTrack);
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

bool xMusicLibrary::isScanned() const {
    return (!musicLibraryArtists.empty());
}

void xMusicLibrary::scanThread() {
    auto artistEntries = scanDirectory();
    size_t totalNoAlbums = 0;
    std::sort(artistEntries.begin(), artistEntries.end());
    // Protect access to the music library
    musicLibraryLock.lock();
    // Clear vector and map
    musicLibraryArtists.clear();
    musicLibraryArtistsMap.clear();
    // Fill vector and map
    for (const auto& artistEntry : artistEntries) {
        // Is the scanning thread interrupted.
        if (musicLibraryScanning->isInterruptionRequested()) {
            // Unlock when scanning is interrupted.
            musicLibraryLock.unlock();
            return;
        }
        auto artistName = QString::fromStdString(artistEntry.path().filename().string());
        auto artist = new xMusicLibraryArtistEntry(artistName, artistEntry.path(), this);
        musicLibraryArtists.emplace_back(artist);
        musicLibraryArtistsMap[artistName] = artist;
        // Scan albums for the current artist.
        artist->scan();
        totalNoAlbums += artist->getNoOfAlbums();
        musicLibraryLock.unlock();
    }
    qDebug() << "Total number of albums: " << totalNoAlbums;
    // Unlock only after the base structure is scanned.
    musicLibraryLock.unlock();
    if (!musicLibraryArtists.empty()) {
        // Emit scanned structure
        emit scannedArtists(musicLibraryArtists);
        size_t currentNoAlbum = 0;
        // Scan the remaining structure.
        for (auto artist : musicLibraryArtists) {
            auto albums = artist->getAlbums();
            for (auto album : albums) {
                // Is the scanning thread interrupted.
                if (musicLibraryScanning->isInterruptionRequested()) {
                    return;
                }
                // Lock around the scanning of the album tracks.
                musicLibraryLock.lock();
                album->scan();
                ++currentNoAlbum;
                musicLibraryLock.unlock();
                emit scanningProgress(static_cast<int>(currentNoAlbum*100 / totalNoAlbums));
            }
        }
    } else {
        // No artists found. Emit error.
        emit scanningError();
    }
}

void xMusicLibrary::compare(const xMusicLibrary* library, QStringList& missingArtists, QStringList& additionalArtists,
                            std::map<QString, QStringList>& missingAlbums, std::map<QString, QStringList>& additionalAlbums,
                            std::list<xMusicLibraryTrackEntry*>& missingTracks, std::list<xMusicLibraryTrackEntry*>& additionalTracks,
                            std::pair<std::list<xMusicLibraryTrackEntry*>, std::list<xMusicLibraryTrackEntry*>>& differentTracks) const {
    musicLibraryLock.lock();
    // Compare the artists.
    QStringList equalArtists;
    for (auto artist : musicLibraryArtists) {
        auto artistName = artist->getArtistName();
        if (library->musicLibraryArtistsMap.find(artistName) != library->musicLibraryArtistsMap.end()) {
            equalArtists.push_back(artistName);
        } else {
            missingArtists.push_back(artistName);
        }
    }
    for (auto artist : library->musicLibraryArtists) {
        auto artistName = artist->getArtistName();
        if (musicLibraryArtistsMap.find(artistName) == musicLibraryArtistsMap.end()) {
            additionalArtists.push_back(artistName);
        }
    }
    // Compare the albums for equal artists.
    std::map<QString, QStringList> equalAlbums;
    for (const auto& equalArtist : equalArtists) {
        try {
            QStringList missing, additional, equal;

            auto artist = musicLibraryArtistsMap.at(equalArtist);
            auto libraryArtist = library->musicLibraryArtistsMap.at(equalArtist);

            for (auto album : artist->getAlbums()) {
                auto albumName = album->getAlbumName();
                if (libraryArtist->getAlbum(albumName)) {
                    equal.push_back(albumName);
                } else {
                    missing.push_back(albumName);
                }
            }
            for (auto libraryAlbum : libraryArtist->getAlbums()) {
                auto libraryAlbumName = libraryAlbum->getAlbumName();
                // It's additional if we cannot find it in our library.
                if (!artist->getAlbum(libraryAlbumName)) {
                    additional.push_back(libraryAlbumName);
                }
            }

            if (!missing.empty()) {
                missingAlbums[equalArtist] = missing;
            }
            if (!additional.empty()) {
                additionalAlbums[equalArtist] = additional;
            }
            if (!equal.empty()) {
                equalAlbums[equalArtist] = equal;
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
            musicLibraryLock.unlock();
            return;
        }
    }
    // Compare the tracks for the equal artist and albums.
    for (const auto& artist : equalAlbums) {
        std::map<QString, std::list<xMusicLibraryTrackEntry*>> missingArtistTracks, additionalArtistTracks, differentArtistTracks;
        for (const auto& album : artist.second) {
            try {
                listDifference(musicLibraryArtistsMap.at(artist.first)->getAlbum(album)->getTracks(),
                               library->musicLibraryArtistsMap.at(artist.first)->getAlbum(album)->getTracks(),
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
                musicLibraryLock.unlock();
                return;
            }
        }
    }
    musicLibraryLock.unlock();
}

void xMusicLibrary::compare(const xMusicLibrary* library,
                            std::map<QString, std::map<QString, std::list<xMusicLibraryTrackEntry*>>>& equalTracks) const {
    musicLibraryLock.lock();
    for (auto libraryArtist : library->musicLibraryArtists) {
        auto libraryArtistName = libraryArtist->getArtistName();
        auto artist = musicLibraryArtistsMap.find(libraryArtistName);
        if (artist != musicLibraryArtistsMap.end()) {
            std::map<QString, std::list<xMusicLibraryTrackEntry*>> artistAlbums;
            auto libraryArtistAlbums = libraryArtist->getAlbums();
            for (auto libraryArtistAlbum : libraryArtistAlbums) {
                auto libraryArtistAlbumName = libraryArtistAlbum->getAlbumName();
                auto artistAlbum = artist->second->getAlbum(libraryArtistAlbumName);
                if (artistAlbum) {
                    std::list<xMusicLibraryTrackEntry*> artistAlbumTracks;
                    for (auto track : artistAlbum->getTracks()) {
                        for (auto libraryTrack : libraryArtistAlbum->getTracks()) {
                            if (track->equal(libraryTrack)) {
                                artistAlbumTracks.emplace_back(track);
                                break;
                            }
                        }
                    }
                    // Add list only if we have equal tracks.
                    if (!artistAlbumTracks.empty()) {
                        artistAlbums[libraryArtistAlbumName] = artistAlbumTracks;
                    }
                }
            }
            // Add albums if we have any albums with equal tracks.
            if (!artistAlbums.empty()) {
                equalTracks[libraryArtistName] = artistAlbums;
            }
        }
    }
    musicLibraryLock.unlock();
}

void xMusicLibrary::listDifference(const std::vector<xMusicLibraryTrackEntry*>& a, const std::vector<xMusicLibraryTrackEntry*>& b,
                                   std::list<xMusicLibraryTrackEntry*>& missing, std::list<xMusicLibraryTrackEntry*>& additional,
                                   std::pair<std::list<xMusicLibraryTrackEntry*>, std::list<xMusicLibraryTrackEntry*>>& different) {

    for (auto aEntry : a) {
        bool found = false;
        for (auto bEntry : b) {
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

bool xMusicLibrary::isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) {
    try {
        if (dirEntry.is_directory()) {
            auto dirName = dirEntry.path().filename().string();
            // Special directories "." and ".." are not valid. Other directories starting with "." are valid.
            if ((dirName != ".") && (dirName != "..")) {
                return true;
            }
        }
    } catch (const std::filesystem::filesystem_error& error) {
        qCritical() << "Unable to access directory entry, error: " << error.what();
    }
    return false;
}

xMusicLibraryEntry* xMusicLibrary::child(size_t index) {
    if (index < musicLibraryArtists.size()) {
        return musicLibraryArtists[index];
    }
    return nullptr;
}

void xMusicLibrary::updateParent(xMusicLibraryEntry* updatedEntry) {
    auto updatedArtist = reinterpret_cast<xMusicLibraryArtistEntry*>(updatedEntry);
    // Find the updated artist in the artist map
    auto artistEntry = std::find_if(musicLibraryArtistsMap.begin(), musicLibraryArtistsMap.end(),
                                   [&updatedArtist](auto entry) {
                                       return entry.second == updatedArtist;
                                   });
    // Update the artist album map and vector.
    if (artistEntry != musicLibraryArtistsMap.end()) {
        // Remove old album entry and insert updated one.
        musicLibraryArtistsMap.erase(artistEntry);
        musicLibraryArtistsMap[updatedArtist->getArtistName()] = updatedArtist;
        // Sort vector as renaming could change ordering.
        std::sort(musicLibraryArtists.begin(), musicLibraryArtists.end(), [](xMusicLibraryArtistEntry* a, xMusicLibraryArtistEntry* b) {
            return *a < *b;
        });
    } else {
        qCritical() << "updateParent: did not find album for given artist: " << updatedArtist->getArtistName();
    }
}

std::vector<xMusicLibraryArtistEntry*> xMusicLibrary::filterArtists(const std::vector<xMusicLibraryArtistEntry*>& artists, const xMusicLibraryFilter& filter) {
    if (filter.hasArtistFilter()) {
        std::vector<xMusicLibraryArtistEntry*> filteredArtists;
        for (auto artist : artists) {
            if (filter.isMatchingArtist(artist->getArtistName())) {
                filteredArtists.emplace_back(artist);
            }
        }
        return filteredArtists;
    } else {
        return artists;
    }
}

std::vector<xMusicLibraryAlbumEntry*> xMusicLibrary::filterAlbums(xMusicLibraryArtistEntry* artist, const xMusicLibraryFilter& filter) {
    std::vector<xMusicLibraryAlbumEntry*> filteredAlbums;
    if (filter.hasAlbumFilter()) {
        for (auto album : artist->getAlbums()) {
            if ((filter.isMatchingAlbum(album->getAlbumName())) &&
                (filter.isMatchingDatabaseArtistAndAlbum(artist->getArtistName(), album->getAlbumName()))) {
                filteredAlbums.emplace_back(album);
            }
        }
    } else {
        filteredAlbums = artist->getAlbums();
    }
    return filteredAlbums;
}

std::vector<xMusicLibraryTrackEntry*> xMusicLibrary::filterTracks(xMusicLibraryAlbumEntry* album, const xMusicLibraryFilter& filter) {
    std::vector<xMusicLibraryTrackEntry*> filteredTracks;
    // Scan album tracks.
    album->scan();
    if (filter.hasTrackNameFilter()) {
        for (auto track : album->getTracks()) {
            if (filter.isMatchingTrackName(track->getTrackName())) {
                filteredTracks.emplace_back(track);
            }
        }
    } else {
        filteredTracks = album->getTracks();
    }
    return filteredTracks;
}


