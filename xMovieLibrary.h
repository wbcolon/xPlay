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

#include <QObject>
#include <QStringList>
#include <QThread>
#include <QMutex>
#include <filesystem>
#include <map>
#include <vector>

typedef std::map<QString, std::map<QString, std::vector<std::pair<QString,QString>>>> xMovieFiles_t;

class xMovieLibraryScanning:public QThread {
    Q_OBJECT

public:
    xMovieLibraryScanning(xMovieFiles_t* movie, QObject* parent=nullptr);
    ~xMovieLibraryScanning() = default;

    void setBaseDirectories(const std::list<std::pair<QString,std::filesystem::path>>& base);

    void run() override;

signals:
    void scannedTags(const QStringList& tags);

private slots:
    void updateMovieExtensions();

private:
    bool isMovieFile(const std::filesystem::path& file);

    xMovieFiles_t* movieFiles;
    QStringList movieExtensions;
    std::list<std::pair<QString,std::filesystem::path>> baseDirectories;
};

class xMovieLibrary:public QObject {
    Q_OBJECT

public:
    xMovieLibrary(QObject* parent=nullptr);
    ~xMovieLibrary() noexcept;

    /**
     * Set base directory for the music library.
     *
     * A certain structure of the music library is expected.
     * The music library is scanned (@see scan).
     *
     * @param base directory that contains the music library.
     */
    void setBaseDirectories(const std::list<std::pair<QString,std::filesystem::path>>& base);

signals:
    void scannedTags(const QStringList& tags);
    void scannedDirectories(const QStringList& directories);
    void scannedMovies(const std::vector<std::pair<QString,QString>>& movies);

public slots:
    void scanForTag(const QString& tag);
    void scanForTagAndDirectory(const QString& tag, const QString& dir);

private:
    // maps directories and files to an assigned tag
    // movieFiles[tag][directory] = files
    xMovieFiles_t* movieFiles;
    xMovieLibraryScanning* movieLibraryScanning;
};

#endif
