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

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QNetworkRequest>
#include <QDebug>

#include <filesystem>


xMusicLibraryEntry::xMusicLibraryEntry(QObject* qParent):
        QObject(qParent),
        entryName(),
        entryUrl(),
        entryLastWritten(),
        entryParent(nullptr) {
}

xMusicLibraryEntry::xMusicLibraryEntry(const QString& eName, const QUrl& eUrl,
                                       xMusicLibraryEntry* eParent, QObject* qParent):
        QObject(qParent),
        entryName(eName),
        entryUrl(eUrl),
        entryParent(eParent) {
    updateLastTimeWritten();
}

xMusicLibraryEntry::xMusicLibraryEntry(const xMusicLibraryEntry& entry):
        QObject(entry.parent()),
        entryName(entry.entryName),
        entryUrl(entry.entryUrl),
        entryLastWritten(entry.entryLastWritten),
        entryParent(entry.entryParent) {
}

[[nodiscard]] const QUrl& xMusicLibraryEntry::getUrl() const {
    return entryUrl;
}

[[nodiscard]] const QString& xMusicLibraryEntry::getName() const {
    return entryName;
}

[[nodiscard]] const QDateTime& xMusicLibraryEntry::getLastWritten() const {
    return entryLastWritten;
}

bool xMusicLibraryEntry::operator < (const xMusicLibraryEntry& entry) const {
    return (entryName.compare(entry.entryName, Qt::CaseInsensitive) < 0);
}

std::vector<std::tuple<QUrl,QString>> xMusicLibraryEntry::scanDirectory() {
    std::vector<std::tuple<QUrl,QString>> validDirEntries;
    if (entryUrl.isLocalFile()) {
        QFileInfo entryPath(entryUrl.toLocalFile());
        if (entryPath.isDir()) {
            auto dirEntries = QDir(entryUrl.toLocalFile()).entryInfoList(QDir::NoFilter, QDir::Name);
            for (const auto& dirEntry : dirEntries) {
                auto dirEntryUrl = QUrl::fromLocalFile(dirEntry.filePath());
                if (isDirectoryEntryValid(dirEntryUrl)) {
                    validDirEntries.emplace_back(dirEntryUrl, dirEntry.fileName());
                }
            }
        }
    }
    return validDirEntries;
}

bool xMusicLibraryEntry::rename(const QString& newEntryName) {
    // No rename possible for non-local files.
    if (!entryUrl.isLocalFile()) {
        return false;
    }
    // No name change. No need to rename.
    if (newEntryName == entryName) {
        return true;
    }
    // Rename.
    try {
        std::filesystem::path entryPath(entryUrl.toLocalFile().toStdString());
        auto entryParentPath = entryPath.parent_path();
        auto oldEntryPath = entryParentPath / entryName.toStdString();
        auto newEntryPath = entryParentPath / newEntryName.toStdString();
        // Rename
        std::filesystem::rename(oldEntryPath, newEntryPath);
        // Update the entry name, path and last time written
        entryName = newEntryName;
        entryPath = newEntryPath;
        entryUrl = QUrl::fromLocalFile(QString::fromStdString(entryPath));
        update();
        updateLastTimeWritten();
        // Update the child using the new entry path as the childs new parent path.
        for (size_t index = 0; child(index) != nullptr; ++index) {
            child(index)->updateChild(entryUrl);
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

void xMusicLibraryEntry::updateChild(const QUrl& newParentUrl) {
    // No child updates for non-local file URLs.
    if (!newParentUrl.isLocalFile()) {
        return;
    }
    entryUrl = QUrl::fromLocalFile(newParentUrl.toLocalFile() + "/" + entryName);
    // Update the child using the new entry path as the childs new parent path.
    for (size_t index = 0; child(index) != nullptr; ++index) {
        child(index)->updateChild(entryUrl);
    }
    update();
    updateLastTimeWritten();
}

void xMusicLibraryEntry::update() {
    // Additional updated to child after renaming.
}

void xMusicLibraryEntry::updateLastTimeWritten() {
    if (entryUrl.isLocalFile()) {
        entryLastWritten = QFileInfo(entryUrl.toLocalFile()).fileTime(QFile::FileModificationTime);
    }
}

