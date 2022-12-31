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

#ifndef __XMUSICLIBRARYALBUMENTRY_H__
#define __XMUSICLIBRARYALBUMENTRY_H__

#include "xMusicLibraryEntry.h"

class xMusicLibraryAlbumEntry:public xMusicLibraryEntry {

public:
    /**
     * Constructors/Destructor
     */
    xMusicLibraryAlbumEntry();
    xMusicLibraryAlbumEntry(const QString& album, const QUrl& albumUrl, xMusicLibraryEntry* artist);
    xMusicLibraryAlbumEntry(const xMusicLibraryAlbumEntry& entry) = default;
    ~xMusicLibraryAlbumEntry() override;
    /**
     * Scan for album entries for the given artist.
     */
    void scan() override;
    /**
     * Verify if tracks for the albums have been scanned.
     *
     * @return true if the tracks have been scanned, false otherwise.
     */
    [[nodiscard]] bool isScanned() const override;
    /**
     * Retrieve the artist name associated with the album entry.
     *
     * @return the artist name as string.
     */
    [[nodiscard]] const QString& getArtistName() const;
    /**
     * Get the name for the album entry.
     *
     * @return the album name as string.
     */
    [[nodiscard]] const QString& getAlbumName() const;
    /**
     * Get all tracks sorted according to their name.
     *
     * @return a sorted vector of track entries.
     */
    [[nodiscard]] std::vector<xMusicLibraryTrackEntry*> getTracks() const;
    /**
     * Get specific track.
     *
     * @param trackNr number of the track.
     * @return a pointer to the track entry.
     */
    [[nodiscard]] xMusicLibraryTrackEntry* getTrack(size_t trackNr) const;
    /**
     * Get the artist for the album.
     *
     * @return a pointer to the artist entry.
     */
    [[nodiscard]] xMusicLibraryArtistEntry* getArtist() const;
    /**
     * Return the total size of the artist tracks.
     *
     * @return the total size in bytes.
     */
    [[nodiscard]] std::uintmax_t getTotalSize() const;

protected:
    /**
     * Determine status of the given directory entry.
     *
     * @param dirEntry the directory entry.
     * @return true if the entry is valid track, false otherwise.
     */
    bool isDirectoryEntryValid(const QUrl& dirEntry) override;
    /**
     * Access the a specific child.
     *
     * @param index the index of the child as integer.
     * @return a pointer to the child if it exists, nullptr otherwise.
     */
    xMusicLibraryEntry* child(size_t index) override;
    /**
     * Update the artist album map and vector if a child was renamed.
     *
     * @param updatedEntry a pointer to the updated child entry.
     */
    void updateParent(xMusicLibraryEntry* childEntry) override;

    // Store tracks sorted according to their name.
    std::vector<xMusicLibraryTrackEntry*> albumTracks;
};

Q_DECLARE_METATYPE(xMusicLibraryAlbumEntry)
Q_DECLARE_METATYPE(xMusicLibraryAlbumEntry*)

#endif
