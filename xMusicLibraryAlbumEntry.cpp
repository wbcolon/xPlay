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

#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryTrackEntry.h"
#include "xPlayerConfiguration.h"

#include <QDebug>


xMusicLibraryAlbumEntry::xMusicLibraryAlbumEntry():xMusicLibraryEntry() {
}

xMusicLibraryAlbumEntry::xMusicLibraryAlbumEntry(const QString& artist, const std::filesystem::path& musicLibraryPath, xMusicLibraryEntry* mParent):
        xMusicLibraryEntry(artist, musicLibraryPath, mParent),
        albumTracks() {
}

xMusicLibraryAlbumEntry::~xMusicLibraryAlbumEntry() {
    // Cleanup track entries.
    for (auto track : albumTracks) {
        delete track;
    }
}

[[nodiscard]] const QString& xMusicLibraryAlbumEntry::getArtistName() const {
    if (getArtist() == nullptr) {
        throw std::runtime_error("xMusicLibraryAlbumEntry::getArtistName(): album not connected to artist");
    }
    return getArtist()->getArtistName();
}

[[nodiscard]] const QString& xMusicLibraryAlbumEntry::getAlbumName() const {
    return entryName;
}

[[nodiscard]] std::vector<xMusicLibraryTrackEntry*> xMusicLibraryAlbumEntry::getTracks() const {
    return albumTracks;
}

[[nodiscard]] xMusicLibraryTrackEntry* xMusicLibraryAlbumEntry::getTrack(size_t trackNr) const {
    if (trackNr < albumTracks.size()) {
        return albumTracks[trackNr];
    } else {
        return nullptr;
    }
}

[[nodiscard]] xMusicLibraryArtistEntry* xMusicLibraryAlbumEntry::getArtist() const {
    return reinterpret_cast<xMusicLibraryArtistEntry*>(entryParent);
}

[[nodiscard]] std::uintmax_t xMusicLibraryAlbumEntry::getTotalSize() const {
    std::uintmax_t totalSize = 0;
    for (auto track : albumTracks) {
        auto trackSize = track->getFileSize();
        if (trackSize != std::uintmax_t(-1)) {
            totalSize += track->getFileSize();
        }
    }
    return totalSize;
}

void xMusicLibraryAlbumEntry::scan() {
    // Do not scan if already scanned.
    if (isScanned()) {
        return;
    }
    // Scan for tracks in album directory.
    auto trackEntries = scanDirectory();
    // Sort the entries according to their name.
    std::sort(trackEntries.begin(), trackEntries.end());
    // Clear vector and map
    albumTracks.clear();
    // Fill vector and map
    for (const auto& trackEntry : trackEntries) {
        auto trackName = QString::fromStdString(trackEntry.path().filename().string());
        auto track = new xMusicLibraryTrackEntry(trackName, trackEntry.path(), this);
        albumTracks.emplace_back(track);
    }
}

bool xMusicLibraryAlbumEntry::isScanned() const {
    return (!albumTracks.empty());
}

bool xMusicLibraryAlbumEntry::isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) {
    // Only query once for valid extensions.
    static auto validExtensions = xPlayerConfiguration::configuration()->getMusicLibraryExtensionList();
    try {
        // Check if we have a file with a valid music file extension.
        if (dirEntry.exists() && dirEntry.is_regular_file()) {
            auto extension = QString::fromStdString(dirEntry.path().extension().string());
            if (validExtensions.contains(extension, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& error) {
        qCritical() << "Unable to access directory entry: "
                    << QString::fromStdString(dirEntry.path())
                    << ", error: " << error.what();
    }
    return false;
}

xMusicLibraryEntry* xMusicLibraryAlbumEntry::child(size_t index) {
    if (index < albumTracks.size()) {
        return albumTracks[index];
    }
    return nullptr;
}

void xMusicLibraryAlbumEntry::updateParent(xMusicLibraryEntry* updatedEntry) {
    auto updatedTrack = reinterpret_cast<xMusicLibraryTrackEntry*>(updatedEntry);
    // Find the updated album in the artist albums map
    auto trackEntry = std::find(albumTracks.begin(), albumTracks.end(), updatedTrack);
    // Update the artist album map and vector.
    if (trackEntry != albumTracks.end()) {
        // Sort vector as renaming could change ordering.
        std::sort(albumTracks.begin(), albumTracks.end(), [](xMusicLibraryTrackEntry* a, xMusicLibraryTrackEntry* b) {
            return *a < *b;
        });
    } else {
        qCritical() << "updateParent: did not find track for given album: " << updatedTrack->getTrackName();
    }
}

