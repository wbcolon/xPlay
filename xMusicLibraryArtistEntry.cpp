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

#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xPlayerBluOSControl.h"

#include <QFileInfo>
#include <QDebug>


xMusicLibraryArtistEntry::xMusicLibraryArtistEntry():xMusicLibraryEntry() {
}

xMusicLibraryArtistEntry::xMusicLibraryArtistEntry(const QString& artist, const QUrl& musicLibraryPath, xMusicLibraryEntry* mParent):
        xMusicLibraryEntry(artist, musicLibraryPath, mParent),
        artistAlbums(),
        artistAlbumsMap() {
}

xMusicLibraryArtistEntry::~xMusicLibraryArtistEntry() {
    // Cleanup album entries.
    for (auto album : artistAlbums) {
        delete album;
    }
}

[[nodiscard]] const QString& xMusicLibraryArtistEntry::getArtistName() const {
    return entryName;
}

[[nodiscard]] std::vector<xMusicLibraryAlbumEntry*> xMusicLibraryArtistEntry::getAlbums() const {
    return artistAlbums;
}

[[nodiscard]] size_t xMusicLibraryArtistEntry::getNoOfAlbums() const {
    return artistAlbums.size();
}

[[nodiscard]] xMusicLibraryAlbumEntry* xMusicLibraryArtistEntry::getAlbum(const QString& albumName) const {
    auto albumPos = artistAlbumsMap.find(albumName);
    if (albumPos != artistAlbumsMap.end()) {
        return albumPos->second;
    } else {
        return nullptr;
    }
}

[[nodiscard]] std::uintmax_t xMusicLibraryArtistEntry::getTotalSize() const {
    std::uintmax_t totalSize = 0;
    for (auto album : artistAlbums) {
        totalSize += album->getTotalSize();
    }
    return totalSize;
}

void xMusicLibraryArtistEntry::scan() {
    std::vector<xDirectoryEntry> albumEntries;
    if (entryUrl.isLocalFile()) {
        albumEntries = scanDirectory();
    } else {
        albumEntries = xPlayerBluOSControls::controls()->getAlbums(entryName);
    }
    std::sort(albumEntries.begin(), albumEntries.end());
    // Clear vector and map
    artistAlbums.clear();
    artistAlbumsMap.clear();
    // Fill vector and map
    for (const auto& [albumUrl, albumPath, albumName, length] : albumEntries) {
        auto album = new xMusicLibraryAlbumEntry(albumName, albumUrl, this);
        artistAlbums.emplace_back(album);
        artistAlbumsMap[albumName] = album;
    }
}

bool xMusicLibraryArtistEntry::isScanned() const {
    return (!artistAlbums.empty());
}

bool xMusicLibraryArtistEntry::isDirectoryEntryValid(const QUrl& dirEntry) {
    if (dirEntry.isLocalFile()) {
        QFileInfo dirPath(dirEntry.toLocalFile());
        if ((dirPath.isDir()) && (dirPath.exists())) {
            auto dirName = dirPath.fileName();
            // Special directories "." and ".." are not valid. Other directories starting with "." are valid.
            if ((dirName != ".") && (dirName != "..")) {
                return true;
            }
        }
        return false;
    } else {
        // Function is not used for remote BluOS player library.
        return false;
    }
}

xMusicLibraryEntry* xMusicLibraryArtistEntry::child(size_t index) {
    if (index < artistAlbums.size()) {
        return artistAlbums[index];
    }
    return nullptr;
}

void xMusicLibraryArtistEntry::updateParent(xMusicLibraryEntry* updatedEntry) {
    auto updatedAlbum = reinterpret_cast<xMusicLibraryAlbumEntry*>(updatedEntry);
    // Find the updated album in the artist albums map
    auto albumEntry = std::find_if(artistAlbumsMap.begin(), artistAlbumsMap.end(),
                                   [&updatedAlbum](auto entry) {
                                       return entry.second == updatedAlbum;
                                   });
    // Update the artist album map and vector.
    if (albumEntry != artistAlbumsMap.end()) {
        // Remove old album entry and insert updated one.
        artistAlbumsMap.erase(albumEntry);
        artistAlbumsMap[updatedAlbum->getAlbumName()] = updatedAlbum;
        // Sort vector as renaming could change ordering.
        std::sort(artistAlbums.begin(), artistAlbums.end(), [](xMusicLibraryAlbumEntry* a, xMusicLibraryAlbumEntry* b) {
            return *a < *b;
        });
    } else {
        qCritical() << "updateParent: did not find album for given artist: " << updatedAlbum->getAlbumName();
    }
}

