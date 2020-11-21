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

#include "xMovieLibrary.h"
#include "xPlayerConfiguration.h"

#include <QDebug>

xMovieLibraryScanning::xMovieLibraryScanning(xMovieFiles_t* movie, QObject* parent):
        QThread(parent),
        movieFiles(movie) {
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieLibraryExtensions,
            this, &xMovieLibraryScanning::updateMovieExtensions);
}

void xMovieLibraryScanning::setBaseDirectories(const std::list<std::pair<QString, std::filesystem::path>>& base) {
    baseDirectories = base;
}

void xMovieLibraryScanning::updateMovieExtensions() {
    movieExtensions = xPlayerConfiguration::configuration()->getMovieLibraryExtensionList();
}

bool xMovieLibraryScanning::isMovieFile(const std::filesystem::path& file) {
    if (std::filesystem::is_regular_file(file)) {
        auto extension = QString::fromStdString(file.extension().string());
        for (const auto& movieExtension : movieExtensions) {
            if (extension.startsWith(movieExtension, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    qDebug() << "xMovieLibrary: skipping: " << QString::fromStdString(file.generic_string());
    return false;
}

void xMovieLibraryScanning::run() {
    // Remove all movies.
    movieFiles->clear();
    // Loop over all base directories
    // We assume that the subdirectories names are distinct over all base directories.
    for (const auto& baseDirectory : baseDirectories) {
        qDebug() << "xMovieLibrary: Directory: " << QString::fromStdString(baseDirectory.second.filename());
        qDebug() << "xMovieLibrary: Tag: " << baseDirectory.first;
        auto baseDirectoryTag = baseDirectory.first;
        auto movieFilesTagPos = movieFiles->find(baseDirectoryTag);
        if (movieFilesTagPos == movieFiles->end()) {
            // Create missing TAG directory
            (*movieFiles)[baseDirectoryTag] = std::map<QString, std::vector<std::pair<QString,QString>>>();
            // Create entry for "." directory
            (*movieFiles)[baseDirectoryTag]["."] = std::vector<std::pair<QString,QString>>();
            movieFilesTagPos = movieFiles->find(baseDirectoryTag);
        }
        // Find "." directory for current TAG. Does exist.
        auto movieFilesMainPos = movieFilesTagPos->second.find(".");
        try {
            for (const auto& entry : std::filesystem::directory_iterator(baseDirectory.second)) {
                const auto& entryPath = entry.path();
                if (std::filesystem::is_directory(entryPath)) {
                    auto directoryName = QString::fromStdString(entryPath.filename());
                    auto movieFilesDirectoryPos = movieFilesTagPos->second.find(directoryName);
                    if (movieFilesDirectoryPos == movieFilesTagPos->second.end()) {
                        // Create missing directory entry for the current TAG.
                        movieFilesTagPos->second[directoryName] = std::vector<std::pair<QString,QString>>();
                        // Find the just directory name just created;
                        movieFilesDirectoryPos = movieFilesTagPos->second.find(directoryName);
                    }
                    for (const auto& directoryEntry : std::filesystem::directory_iterator(entryPath)) {
                        const auto& directoryEntryPath = directoryEntry.path();
                        if (isMovieFile(directoryEntryPath)) {
                            qDebug() << "xMovieLibrary: directory: " << QString::fromStdString(entryPath.filename())
                                     << ", movie: " << QString::fromStdString(directoryEntryPath.filename());
                            movieFilesDirectoryPos->second.emplace_back(QString::fromStdString(directoryEntryPath.filename()),
                                                                        QString::fromStdString(directoryEntryPath.generic_string()));
                        }
                    }
                } else if (isMovieFile(entryPath)) {
                    movieFilesMainPos->second.emplace_back(QString::fromStdString(entryPath.filename()),
                                                           QString::fromStdString(entryPath.generic_string()));
                    qDebug() << "xMovieLibrary: movie: " << QString::fromStdString(entryPath.filename());
                }
            }
        } catch (...) {
            qCritical() << "xMovieLibrary: directory not found: " << QString::fromStdString(baseDirectory.second.generic_string());
        }
    }
    // Create list of tags. Some debugging output.
    QStringList tagList;
    for (auto& tag : (*movieFiles)) {
        for (auto& dir : tag.second) {
            std::sort(dir.second.begin(), dir.second.end(), [](const std::pair<QString,QString>& a, const std::pair<QString,QString>& b) { return (a.first < b.first); } );
        }
        tagList.push_back(tag.first);
    }
    // Update UI.
    emit scannedTags(tagList);
}


xMovieLibrary::xMovieLibrary(QObject *parent):
        QObject(parent) {
    // Create movie file structure.
    movieFiles = new xMovieFiles_t;
    // Create scanning thread.
    movieLibraryScanning = new xMovieLibraryScanning(movieFiles, this);
    connect(movieLibraryScanning, &xMovieLibraryScanning::scannedTags, this, &xMovieLibrary::scannedTags);
}

xMovieLibrary::~xMovieLibrary() noexcept {
    delete movieFiles;
}

void xMovieLibrary::setBaseDirectories(const std::list<std::pair<QString,std::filesystem::path>>& base) {
    movieLibraryScanning->setBaseDirectories(base);
    movieLibraryScanning->start(QThread::IdlePriority);
}

void xMovieLibrary::scanForTag(const QString& tag) {
    auto tagPos = movieFiles->find(tag);
    if (tagPos != movieFiles->end()) {
        QStringList dirList;
        for (const auto& dirEntry : tagPos->second) {
            // Ignore the entry for the "." directory.
            if (dirEntry.first != ".") {
                dirList.push_back(dirEntry.first);
            }
        }
        // Update UI. List may be empty.
        emit scannedDirectories(dirList);
    }
}

void xMovieLibrary::scanForTagAndDirectory(const QString& tag, const QString& dir) {
    try {
        emit scannedMovies((*movieFiles)[tag][dir]);
    } catch (...) {
        // Ignore any error. Do not update UI.
        qCritical() << "Scanning Error for tag: " << tag << ", directory " << dir;
    }
}

