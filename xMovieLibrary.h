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
#include <filesystem>
#include <map>
#include <vector>

class xMovieLibrary:public QObject {
    Q_OBJECT

public:
    xMovieLibrary(QObject* parent=nullptr);
    ~xMovieLibrary() = default;

    /**
     * Set base directory for the music library.
     *
     * A certain structure of the music library is expected.
     * The music library is scanned (@see scan).
     *
     * @param base directory that contains the music library.
     */
    void setBaseDirectories(const std::list<std::pair<QString,std::filesystem::path>>& base);
    /**
     * Retrieve the currently set base directory of the music library.
     *
     * @return the base directory currently in use.
     */
    const std::list<std::pair<QString,std::filesystem::path>>& getBaseDirectories() const;

signals:
    void scannedTags(const QStringList& tags);
    void scannedDirectories(const QStringList& directories);
    void scannedMovies(const std::vector<std::pair<QString,QString>>& movies);

public slots:
    void scanForTag(const QString& tag);
    void scanForTagAndDirectory(const QString& tag, const QString& dir);

private:
    bool isMovieFile(const std::filesystem::path& file);
    void scan();

    // maps directories and files to an assigned tag
    // movieFiles[tag][directory] = files
    std::map<QString, std::map<QString, std::vector<std::pair<QString,QString>>>> movieFiles;
    std::list<std::pair<QString,std::filesystem::path>> baseDirectories;
};

#endif
