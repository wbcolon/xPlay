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

#include "xMusicDirectory.h"


xMusicDirectory::xMusicDirectory():
        QObject(),
        directoryName(),
        directoryPath(),
        directoryLastWritten() {
}

xMusicDirectory::xMusicDirectory(const QString& name, QObject* parent):
        QObject(parent),
        directoryName(name),
        directoryLastWritten() {
}

xMusicDirectory::xMusicDirectory(const std::filesystem::directory_entry& entry, QObject* parent):
        QObject(parent) {
    directoryPath = entry.path();
    directoryName = QString::fromStdString(directoryPath.filename());
    directoryLastWritten = entry.last_write_time();
}

xMusicDirectory::xMusicDirectory(const xMusicDirectory& dir):
        QObject(dir.parent()),
        directoryName(dir.directoryName),
        directoryPath(dir.directoryPath),
        directoryLastWritten(dir.directoryLastWritten) {
}

const QString& xMusicDirectory::name() const {
    return directoryName;
}

const std::filesystem::path& xMusicDirectory::path() const {
    return directoryPath;
}

const std::filesystem::file_time_type& xMusicDirectory::lastWritten() const {
    return directoryLastWritten;
}

bool xMusicDirectory::operator < (const xMusicDirectory& entry) const {
    return (directoryName.compare(entry.directoryName, Qt::CaseInsensitive) < 0);
}

xMusicDirectory& xMusicDirectory::operator = (const xMusicDirectory& entry) {
    directoryName = entry.directoryName;
    directoryPath = entry.directoryPath;
    directoryLastWritten = entry.directoryLastWritten;
    return *this;
}
