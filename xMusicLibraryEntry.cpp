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

#include "xMusicLibraryEntry.h"

#include <QDebug>


xMusicLibraryEntry::xMusicLibraryEntry(QObject* qParent):
        QObject(qParent),
        entryName(),
        entryPath(),
        entryLastWritten(),
        entryParent(nullptr) {
}

xMusicLibraryEntry::xMusicLibraryEntry(const QString& eName, const std::filesystem::path& ePath,
                                       xMusicLibraryEntry* eParent, QObject* qParent):
        QObject(qParent),
        entryName(eName),
        entryPath(ePath),
        entryParent(eParent) {
    updateLastTimeWritten();
}

xMusicLibraryEntry::xMusicLibraryEntry(const xMusicLibraryEntry& entry):
        QObject(entry.parent()),
        entryName(entry.entryName),
        entryPath(entry.entryPath),
        entryLastWritten(entry.entryLastWritten),
        entryParent(entry.entryParent) {
}

[[nodiscard]] const std::filesystem::path& xMusicLibraryEntry::getPath() const {
    return entryPath;
}

[[nodiscard]] const QString& xMusicLibraryEntry::getName() const {
    return entryName;
}

[[nodiscard]] const std::filesystem::file_time_type& xMusicLibraryEntry::getLastWritten() const {
    return entryLastWritten;
}

bool xMusicLibraryEntry::operator < (const xMusicLibraryEntry& entry) const {
    return (entryName.compare(entry.entryName, Qt::CaseInsensitive) < 0);
}

std::vector<std::filesystem::directory_entry> xMusicLibraryEntry::scanDirectory() {
    if (std::filesystem::is_directory(entryPath)) {
        std::vector<std::filesystem::directory_entry> validDirEntries;
        for (const auto &dirEntry: std::filesystem::directory_iterator(entryPath)) {
            if (!isDirectoryEntryValid(dirEntry)) {
                // Ignore invalid entries
                continue;
            }
            validDirEntries.emplace_back(dirEntry);
        }
        return validDirEntries;
    }
    return {};
}

bool xMusicLibraryEntry::rename(const QString& newEntryName) {
    // No name change. No need to rename.
    if (newEntryName == entryName) {
        return true;
    }
    // Rename.
    try {
        auto entryParentPath = entryPath.parent_path();
        auto oldEntryPath = entryParentPath / entryName.toStdString();
        auto newEntryPath = entryParentPath / newEntryName.toStdString();
        // Rename
        std::filesystem::rename(oldEntryPath, newEntryPath);
        // Update the entry name, path and last time written
        entryName = newEntryName;
        entryPath = newEntryPath;
        update();
        updateLastTimeWritten();
        // Update the child using the new entry path as the childs new parent path.
        for (size_t index = 0; child(index) != nullptr; ++index) {
            child(index)->updateChild(entryPath);
        }
        // Update the parent.
        entryParent->updateParent(this);
        // Successful
        return true;
    } catch (const std::filesystem::filesystem_error& error) {
        qCritical() << "Unable to rename music library entry: " << entryName << ", error: " << error.what();
        return false;
    }
}

void xMusicLibraryEntry::updateChild(const std::filesystem::path& newParentPath) {
    entryPath = newParentPath / entryName.toStdString();
    // Update the child using the new entry path as the childs new parent path.
    for (size_t index = 0; child(index) != nullptr; ++index) {
        child(index)->updateChild(entryPath);
    }
    update();
    updateLastTimeWritten();
}

void xMusicLibraryEntry::update() {
    // Additional updated to child after renaming.
}

void xMusicLibraryEntry::updateLastTimeWritten() {
    try {
        entryLastWritten = std::filesystem::last_write_time(entryPath);
    } catch (std::filesystem::filesystem_error &error) {
        qCritical() << "Unable to access entry " << QString::fromStdString(entryPath) << ", error: " << error.what();
        entryLastWritten = std::filesystem::file_time_type();
    }
}

