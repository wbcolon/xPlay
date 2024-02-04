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

#ifndef __XMOVIELIBRARY_H__
#define __XMOVIELIBRARY_H__

#include "xMovieLibraryEntry.h"

#include <QObject>
#include <QStringList>
#include <QThread>
#include <QMutex>
#include <filesystem>
#include <map>
#include <vector>
#include "xMovieLibraryEntry.h"


/**
 * Typedef for movie library files structure. Improve the readability of the code.
 */
typedef std::map<QString, std::map<QString, std::vector<xMovieLibraryEntry*>>> xMovieFiles_t;

class xMovieLibraryScanning:public QThread {
    Q_OBJECT

public:
    explicit xMovieLibraryScanning(xMovieFiles_t* movie, QObject* parent=nullptr);
    ~xMovieLibraryScanning() override = default;
    /**
     * Set the base directories with tags for scanning.
     *
     * @param base a list of pairs of tags and filesystem paths.
     */
    void setBaseDirectories(const std::list<std::pair<QString,std::filesystem::path>>& base);
    /**
     * Scan the tag and filesystem paths for movie files.
     */
    void run() override;

signals:
    /**
     * Signal emitted after successful scanning of the movie library.
     *
     * @param tags the list of tags.
     */
    void scannedTags(const QStringList& tags);

private slots:
    /**
     * Update accepted movie file extensions.
     */
    void updateMovieExtensions();

private:
    /**
     * Determine if the given file is a movie file.
     *
     * @param file the path to the file checked.
     * @return true if the file has a movie extension, false otherwise.
     */
    bool isMovieFile(const std::filesystem::path& file);

    xMovieFiles_t* movieFiles;
    QStringList movieExtensions;
    std::list<std::pair<QString,std::filesystem::path>> baseDirectories;
};

class xMovieLibrary:public QObject {
    Q_OBJECT

public:
    explicit xMovieLibrary(QObject* parent=nullptr);
    ~xMovieLibrary() noexcept override;
    /**
     * Set base directories and tags for the movie library.
     *
     * A certain structure of the movie library is expected.
     * The movie library is scanned in a thread using the
     * movie library scanning class.
     *
     * @param base list of pairs of tag and filesystem path for movies.
     */
    void setBaseDirectories(const std::list<std::pair<QString,std::filesystem::path>>& base);

signals:
    /**
     * Signal emitted after scanning of the movie library.
     *
     * This signal is connected to the movie library scanning signal
     * of the same name and triggered after scanning.
     *
     * @param tags the list of strings of tags.
     */
    void scannedTags(const QStringList& tags);
    /**
     * Signal emitted after a scan for a specific tag.
     *
     * @param directories the list of directories for this tag.
     */
    void scannedDirectories(const QStringList& directories);
    /**
     * Signal emitted after a scan for a tag and directory.
     *
     * @param movies a vector of pairs of file name and full path.
     */
    void scannedMovies(const std::vector<xMovieLibraryEntry*>& movies);
    /**
     * Signal the list of entries (movies and cached length) not found in the movie library
     *
     * @param listEntries a list of tuples of tag, directory and movie not found.
     * @param listCachedEntries a list of tuples of tag, directory and movie not found.
     */
    void scannedUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries,
                               const std::list<std::tuple<QString, QString, QString>>& listCachedEntries);

public slots:
    /**
     * Scan the movie library for a specific tag.
     *
     * @param tag the tag as string to search for.
     */
    void scanForTag(const QString& tag);
    /**
     * Scan the movie library for a specific tag and directory.
     *
     * @param tag the tag as string to search for.
     * @param dir the corresponding directory for the given tag.
     */
    void scanForTagAndDirectory(const QString& tag, const QString& dir);
    /**
     * Scan list to find entries that are not in the movie library.
     *
     * @param listEntries the list of tuples of tag, directory and movie to verify.
     * @param listCachedEntries the list of tuples of tag, directory and movie to verify.
     */
    void scanForUnknownEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries,
                               const std::list<std::tuple<QString, QString, QString>>& listCachedEntries);

private:
    /**
     * Check a list of tag/directory/movie entries and record unknown entries.
     *
     * @param listEntries the list of entries to verify.
     * @param listUnknownEntries add unknown entries to existing list (do not clear).
     */
    void scanListOfEntries(const std::list<std::tuple<QString, QString, QString>>& listEntries,
                           std::list<std::tuple<QString, QString, QString>>& listUnknownEntries);

    // maps directories and files to an assigned tag
    // movieFiles[tag][directory] = files
    xMovieFiles_t* movieFiles;
    xMovieLibraryScanning* movieLibraryScanning;
};

Q_DECLARE_METATYPE(xMovieLibraryEntry)
Q_DECLARE_METATYPE(xMovieLibraryEntry*)

#endif
