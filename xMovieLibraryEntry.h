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

#ifndef __XMOVIELIBRARYENTRY_H__
#define __XMOVIELIBRARYENTRY_H__

#include <QObject>
#include <filesystem>
#include <vector>


class xMovieLibraryEntry:public QObject {
public:
    xMovieLibraryEntry();
    xMovieLibraryEntry(const QString& tag, const QString& directory,
                       const QString& movie, const std::filesystem::path& moviePath, QObject* parent= nullptr);
    xMovieLibraryEntry(const xMovieLibraryEntry& entry);
    ~xMovieLibraryEntry() override = default;

    /**
     * Retrieve the tag associated with the movie entry.
     *
     * @return the tag as string.
     */
    [[nodiscard]] const QString& getTagName() const;
    /**
     * Retrieve the directory associated with the movie entry.
     *
     * @return the directory as string.
     */
    [[nodiscard]] const QString& getDirectoryName() const;
    /**
     * Retrieve the movie associated with the movie entry.
     *
     * @return the movie as string.
     */
    [[nodiscard]] const QString& getMovieName() const;
    /**
     * Retrieve the file size of the movie file.
     *
     * @return the size of the file in bytes.
     */
    [[nodiscard]] std::uintmax_t getFileSize() const;
    /**
     * Retrieve the length of the movie file.
     *
     * The length is determined by libVLC and requires an
     * additional scan of the music file.
     *
     * @return the length in ms as integer.
     */
    [[nodiscard]] qint64 getLength() const;
    /**
     * Get the path for the movie file.
     *
     * @return the filesystem::path to the entry.
     */
    [[nodiscard]] const std::filesystem::path& getPath() const;
    /**
     * Verify if the movie has been scanned.
     *
     * @return true if the movie has been scanned, false otherwise.
     */
    [[nodiscard]] bool isScanned() const;

protected:
    void scan() const;

    QString entryTag;
    QString entryDirectory;
    QString entryMovie;
    std::filesystem::path entryPath;
    mutable qint64 entryLength;
    mutable std::uintmax_t entrySize;
};


#endif
