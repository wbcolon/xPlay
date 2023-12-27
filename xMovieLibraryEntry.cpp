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

#include "xMovieLibraryEntry.h"
#include "xPlayerDatabase.h"

#include <vlc/vlc.h>

#include <QThread>
#include <QDebug>

xMovieLibraryEntry::xMovieLibraryEntry():
        QObject(),
        entryTag(),
        entryDirectory(),
        entryMovie(),
        entryPath(),
        entryLength(0),
        entrySize(0) {
}

xMovieLibraryEntry::xMovieLibraryEntry(const QString& tag, const QString& directory,
                                       const QString& movie, const std::filesystem::path& moviePath, QObject* parent):
        QObject(parent),
        entryTag(tag),
        entryDirectory(directory),
        entryMovie(movie),
        entryPath(moviePath),
        entryLength(-1),
        entrySize(-1) {
}

xMovieLibraryEntry::xMovieLibraryEntry(const xMovieLibraryEntry& entry):
        QObject(entry.parent()),
        entryTag(entry.entryTag),
        entryDirectory(entry.entryDirectory),
        entryMovie(entry.entryMovie),
        entryPath(entry.entryPath),
        entryLength(entry.entryLength),
        entrySize(entry.entrySize) {
}

const QString& xMovieLibraryEntry::getTagName() const {
    return entryTag;
}

const QString& xMovieLibraryEntry::getDirectoryName() const {
    return entryDirectory;
}

const QString& xMovieLibraryEntry::getMovieName() const {
    return entryMovie;
}

std::uintmax_t xMovieLibraryEntry::getFileSize() const {
    scan();
    return entrySize;
}

qint64 xMovieLibraryEntry::getLength() const {
    scan();
    return entryLength;
}

const std::filesystem::path& xMovieLibraryEntry::getPath() const {
    return entryPath;
}

void xMovieLibraryEntry::scan() const {
    if ((entrySize > 0) && (entryLength > 0)) {
        return;
    }
    // Determine file size.
    try {
        entrySize = std::filesystem::file_size(entryPath);
    } catch (const std::filesystem::filesystem_error& error) {
        qCritical() << "Unable to access track: "
                    << QString::fromStdString(entryPath.generic_string()) << ", error: " << error.what();
        entrySize = static_cast<std::uintmax_t>(-1);
        entryLength = -1;
        return;
    }

    auto [movieSize, movieLength] = xPlayerDatabase::database()->getMovieFileLength(entryTag, entryDirectory, entryMovie);
    if ((entrySize == static_cast<std::uintmax_t>(movieSize)) && (movieLength > 0)) {
        entryLength = movieLength;
        return;
    }
    // Determine movie length. Use VLC without video output.
    // Only instantiate VLC once for movie file entries.
    static const char* const vlcArgs[] = { "--vout=none", "--quiet" };
    static auto vlcInstance = libvlc_new(sizeof(vlcArgs)/sizeof(vlcArgs[0]), vlcArgs);
    auto vlcMedia = libvlc_media_new_path(vlcInstance, entryPath.generic_string().c_str());
    libvlc_media_parse_with_options(vlcMedia, libvlc_media_parse_network, 1000);
    auto vlcStatus = libvlc_media_get_parsed_status(vlcMedia);
    // Wait until parsing complete. Wait for 10 seconds maximum.
    for (auto vlcStatusLoop = 0; (vlcStatus != libvlc_media_parsed_status_done) &&
                                 (vlcStatus != libvlc_media_parsed_status_failed) &&
                                 (vlcStatusLoop < 50); ++vlcStatusLoop) {
        // Break out of loop if requested.
        if (QThread::currentThread()->isInterruptionRequested()) {
            // Release media and vlc instance.
            libvlc_media_release(vlcMedia);
            // Reset entrySize to allow scan.
            entrySize = -1;
            return;
        }
        // Wait for 200ms.
        QThread::msleep(200);
        vlcStatus = libvlc_media_get_parsed_status(vlcMedia);
    }
    entryLength = static_cast<qint64>(libvlc_media_get_duration(vlcMedia));
    // Release media and vlc instance.
    libvlc_media_release(vlcMedia);
    qDebug() << "Scan: length: " << entryLength << ", size: " << entrySize;
    if (entryLength > 0) {
        // We only record non-zero length.
        xPlayerDatabase::database()->updateMovieFileLength(entryTag, entryDirectory, entryMovie, static_cast<qint64>(entrySize), entryLength);
    }
}

bool xMovieLibraryEntry::isScanned() const {
    return (entryLength > 0);
}