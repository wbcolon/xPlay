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

#ifndef __XMUSICLIBRARYARTISTENTRY_H__
#define __XMUSICLIBRARYARTISTENTRY_H__

#include "xMusicLibraryEntry.h"


class xMusicLibraryArtistEntry: public xMusicLibraryEntry {

public:
    /**
     * Constructors/Destructor
     */
    xMusicLibraryArtistEntry();
    xMusicLibraryArtistEntry(const QString& artist, const std::filesystem::path& artistPath, xMusicLibraryEntry* parent);
    xMusicLibraryArtistEntry(const xMusicLibraryArtistEntry& entry) = default;
    ~xMusicLibraryArtistEntry() override;
    /**
     * Scan for album entries for the given artist.
     */
    void scan() override;
    /**
     * Verify if entry has been scanned.
     *
     * @return true if the entry has been scanned, false otherwise.
     */
    [[nodiscard]] bool isScanned() const override;
    /**
     * Get the name for the artist entry.
     *
     * @return the artist name as string.
     */
    [[nodiscard]] const QString& getArtistName() const;
    /**
     * Get all albums.
     *
     * @return a sorted vector of album entries.
     */
    [[nodiscard]] std::vector<xMusicLibraryAlbumEntry*> getAlbums() const;
    /**
     * Get number of albums for this artist.
     *
     * @return the number of album entries.
     */
    [[nodiscard]] std::size_t getNoOfAlbums() const;
    /**
     * Get a specific album for the artist.
     *
     * @param albumName the name of album as string.
     * @return a pointer to the album entry if found, nullptr otherwise.
     */
    [[nodiscard]] xMusicLibraryAlbumEntry* getAlbum(const QString& albumName) const;
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
     * @return true if the entry is valid album, false otherwise.
     */
    bool isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) override;
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
    void updateParent(xMusicLibraryEntry* updatedEntry) override;

    // Keep sorted vector of albums
    std::vector<xMusicLibraryAlbumEntry*> artistAlbums;
    // Keep map to allow fast access to specific album by name
    std::map<QString, xMusicLibraryAlbumEntry*> artistAlbumsMap;
};

Q_DECLARE_METATYPE(xMusicLibraryArtistEntry)
Q_DECLARE_METATYPE(xMusicLibraryArtistEntry*)


#endif
