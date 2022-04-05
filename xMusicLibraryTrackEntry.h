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

#ifndef __XMUSICLIBRARYTRACKENTRY_H__
#define __XMUSICLIBRARYTRACKENTRY_H__

#include "xMusicLibraryEntry.h"

class xMusicLibraryTrackEntry:public xMusicLibraryEntry {

public:
    xMusicLibraryTrackEntry();
    xMusicLibraryTrackEntry(const QString& track, const std::filesystem::path& trackPath, xMusicLibraryEntry* album);
    xMusicLibraryTrackEntry(const xMusicLibraryTrackEntry& file) = default;
    ~xMusicLibraryTrackEntry() override = default;
    /**
     * Scan for tags for the given track.
     */
    void scan() override;
    /**
     * Verify if the track tags have been scanned.
     *
     * @return true if the track tags have been scanned, false otherwise.
     */
    [[nodiscard]] bool isScanned() const override;
    /**
     * Update the artist, album, track nr and name tags in the track entry.
     */
    void updateTags();
    /**
     * Retrieve the artist name associated with the track entry.
     *
     * @return the artist name as string.
     */
    [[nodiscard]] const QString& getArtistName() const;
    /**
     * Retrieve the album name associated with the track entry.
     *
     * @return the album name as string.
     */
    [[nodiscard]] const QString& getAlbumName() const;
    /**
     * Retrieve the track name associated with the track entry
     *
     * @return the track name as string.
     */
    [[nodiscard]] const QString& getTrackName() const;
    /**
     * Get the album to the track.
     *
     * @return a pointer to the album entry.
     */
    [[nodiscard]] xMusicLibraryAlbumEntry* getAlbum() const;
    /**
     * Retrieve the file size of the music file.
     *
     * @return the size of the file in bytes.
     */
    [[nodiscard]] std::uintmax_t getFileSize() const;
    /**
     * Retrieve the length of the music file.
     *
     * The length is determined by taglib and requires an
     * additional scan of the music file.
     *
     * @return the length in ms as integer.
     */
    [[nodiscard]] qint64 getLength() const;
    /**
     * Retrieve the bits per sample for the music file.
     *
     * The bits per sample are determined by taglib and
     * require an additional scan of the music file.
     *
     * @return the bits per samples as integer.
     */
    [[nodiscard]] int getBitsPerSample() const;
    /**
     * Retrieve the sample rate for the music file.
     *
     * The sample rate is determined by taglib and
     * requires an additional scan of the music file.
     *
     * @return the sample rate as integer
     */
    [[nodiscard]] int getSampleRate() const;
    /**
     * Retrieve the bitrate for the music file.
     *
     * The bitrate is determined by taglib and
     * requires an additional scan of the music file.
     *
     * @return the bitrate as integer.
     */
    [[nodiscard]] int getBitrate() const;
    /**
     * Compare music files based on artist, album, track name and size.
     *
     * @param file the other music file to compare to.
     * @return true if the music files are equal, false otherwise.
     */
    [[nodiscard]] bool equal(xMusicLibraryTrackEntry* track, bool checkFileSize=true) const;
    /**
     * Compare the music files according to their track names.
     *
     * The comparison is performed case-insensitive.
     *
     * @param file the other music file we compare to.
     * @return the result of the compare on the track names.
     */
    [[nodiscard]] int compareTrackName(xMusicLibraryTrackEntry* track) const;

protected:
    /**
     * Determine status of the given directory entry.
     *
     * Not called. Throws exception.
     */
    bool isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) override;
    /**
     * Access the a specific child.
     *
     * Not called. Throws exception.
     */
    xMusicLibraryEntry* child(size_t index) override;
    /**
     * Update the child.
     */
    void update() override;
    /**
     * Update the tracks if a child was renamed.
     *
     * Not called. Throws exception.
     */
    void updateParent(xMusicLibraryEntry* childEntry) override;

private:
    /**
     * Scan for tags in the given track.
     */
    void scanTags() const;

    std::uintmax_t fileSize;
    // make track properties mutable, because we scan the file only on demand.
    mutable qint64 trackLength;
    mutable int trackBitsPerSample;
    mutable int trackBitrate;
    mutable int trackSampleRate;
};

Q_DECLARE_METATYPE(xMusicLibraryTrackEntry)
Q_DECLARE_METATYPE(xMusicLibraryTrackEntry*)

#endif
