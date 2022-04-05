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

#ifndef __XMUSICLIBRARYENTRY_H__
#define __XMUSICLIBRARYENTRY_H__

#include <QObject>
#include <QString>

#include <filesystem>

class xMusicLibraryEntry:public QObject {

public:
    /**
     * Get the path for the entry.
     *
     * @return the filesystem::path to the entry.
     */
    [[nodiscard]] const std::filesystem::path& getPath() const;
    /**
     * Get the name for the entry.
     *
     * @return the entry name as string.
     */
    [[nodiscard]] const QString& getName() const;
    /**
     * Get the last written time stamp for the entry.
     *
     * @return the last writtem time stamp.
     */
    [[nodiscard]] const std::filesystem::file_time_type& getLastWritten() const;
    /**
     * Scan for child entries.
     */
    virtual void scan() = 0;
    /**
     * Verify if entry has been scanned.
     *
     * @return true if the entry has been scanned, false otherwise.
     */
    [[nodiscard]] virtual bool isScanned() const = 0;
    /**
     * Rename the current entry.
     *
     * @param newEntryName the new entry name as string.
     * @return true if the renaming was successful, false otherwise.
     */
    [[nodiscard]] bool rename(const QString& newEntryName);
    /**
     * Lesser compare two music library entries.
     *
     * @param entry the other music library entry for the comparison
     * @return the result of the lesser comparison on the directory entry names.
     */
    bool operator < (const xMusicLibraryEntry& entry) const;

protected:
    /**
     * Constructors/Destructor
     */
    explicit xMusicLibraryEntry(QObject* qParent = nullptr);
    xMusicLibraryEntry(const QString& eName, const std::filesystem::path& ePath,
                       xMusicLibraryEntry* eParent = nullptr, QObject* qParent = nullptr);
    xMusicLibraryEntry(const xMusicLibraryEntry& entry);
    ~xMusicLibraryEntry() override = default;
    /**
     * Scan the entry path (if it is a directory) for valid entries.
     *
     * @return a vector of valid directory entries.
     */
    std::vector<std::filesystem::directory_entry> scanDirectory();
    /**
     * Determine status of the given directory entry.
     *
     * @param dirEntry the directory entry.
     * @return true if the entry is valid, false otherwise.
     */
    virtual bool isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) = 0;
    /**
     * Access the a specific child.
     *
     * @param index the index of the child as integer.
     * @return a pointer to the child if it exists, nullptr otherwise.
     */
    virtual xMusicLibraryEntry* child(size_t index) = 0;
    /**
     * Update the child if the parent was renamed.
     *
     * @param entryParentPath the new path of the parent entry.
     */
    void updateChild(const std::filesystem::path& entryParentPath);
    /**
     * Update the child.
     */
    virtual void update();
    /**
     * Update the parent if a child was renamed.
     *
     * @param childEntry the pointer to the renamed child entry.
     */
    virtual void updateParent(xMusicLibraryEntry* childEntry) = 0;
    /**
     * Update the last written time stamp.
     */
    void updateLastTimeWritten();

    QString entryName;
    std::filesystem::path entryPath;
    std::filesystem::file_time_type entryLastWritten;
    xMusicLibraryEntry* entryParent;
};

// Forward declaration for artist, album and track entries.
class xMusicLibraryArtistEntry;
class xMusicLibraryAlbumEntry;
class xMusicLibraryTrackEntry;


#endif
