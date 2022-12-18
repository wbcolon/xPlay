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

#include "xMusicLibraryTrackEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryArtistEntry.h"
#include "xPlayerConfiguration.h"

#include <QRegularExpression>
#include <QProcess>
#include <QDebug>

#include <taglib/fileref.h>
#include <taglib/audioproperties.h>
#include <taglib/flacproperties.h>
#include <taglib/wavpackproperties.h>


xMusicLibraryTrackEntry::xMusicLibraryTrackEntry():
    xMusicLibraryEntry(),
    fileSize(-1),
    trackLength(-1),
    trackBitsPerSample(-1),
    trackBitrate(-1),
    trackSampleRate(-1) {
}

xMusicLibraryTrackEntry::xMusicLibraryTrackEntry(const QString& track, const std::filesystem::path& trackPath, xMusicLibraryEntry* album):
        xMusicLibraryEntry(track, trackPath, album),
        trackLength(-1),
        trackBitsPerSample(-1),
        trackBitrate(-1),
        trackSampleRate(-1) {
    try {
        fileSize = std::filesystem::file_size(trackPath);
    } catch (const std::filesystem::filesystem_error& error) {
        qCritical() << "Unable to access track: "
                    << QString::fromStdString(entryPath.generic_string()) << ", error: " << error.what();
        fileSize = static_cast<std::uintmax_t>(-1);
    }
}

[[nodiscard]] xMusicLibraryAlbumEntry* xMusicLibraryTrackEntry::getAlbum() const {
    return reinterpret_cast<xMusicLibraryAlbumEntry*>(entryParent);
}

[[nodiscard]] const QString& xMusicLibraryTrackEntry::getArtistName() const {
    if ((getAlbum() == nullptr) || (getAlbum()->getArtist() == nullptr)) {
        throw std::runtime_error("xMusicLibraryTrackEntry::getArtistName(): track not connected to artist or album");
    }
    return getAlbum()->getArtist()->getArtistName();
}

[[nodiscard]] const QString& xMusicLibraryTrackEntry::getAlbumName() const {
    if (getAlbum() == nullptr) {
        throw std::runtime_error("xMusicLibraryTrackEntry::getAlbumName(): track not connected to album");
    }
    return getAlbum()->getAlbumName();
}

[[nodiscard]] const QString& xMusicLibraryTrackEntry::getTrackName() const {
    return entryName;
}

std::uintmax_t xMusicLibraryTrackEntry::getFileSize() const {
    return fileSize;
}

qint64 xMusicLibraryTrackEntry::getLength() const {
    scanTags();
    return trackLength;
}

int xMusicLibraryTrackEntry::getBitsPerSample() const {
    scanTags();
    return trackBitsPerSample;
}

int xMusicLibraryTrackEntry::getSampleRate() const {
    scanTags();
    return trackSampleRate;
}

int xMusicLibraryTrackEntry::getBitrate() const {
    scanTags();
    return trackBitrate;
}

bool xMusicLibraryTrackEntry::isScanned() const {
    return (trackLength > 0);
}

void xMusicLibraryTrackEntry::scan() {
    scanTags();
}

void xMusicLibraryTrackEntry::updateTags() {
    // Split the track name into nr, name and extension.
    QRegularExpression trackNameRegExp(R"((?<nr>\d\d) (?<name>.*)\.(?<ext>.*))");
    auto trackNameMatch = trackNameRegExp.match(entryName);
    if (trackNameMatch.hasMatch()) {
        auto trackNr = trackNameMatch.captured("nr");
        auto trackName = trackNameMatch.captured("name");
        // Convert into to std::string
        auto artistName = getArtistName();
        auto albumName = getAlbumName();
        if (xPlayerConfiguration::configuration()->useLLTag()) {
            auto lltagProcess = new QProcess();
            // Merge channels as we ignore any output.
            lltagProcess->setProcessChannelMode(QProcess::MergedChannels);
            lltagProcess->start(xPlayerConfiguration::configuration()->getLLTag(),
                                { {"--yes"}, {"--ARTIST"}, artistName, {"--ALBUM"}, albumName, {"--TITLE"},
                                  trackName, {"--NUMBER"}, trackNr, QString::fromStdString(entryPath.string()) });
            lltagProcess->waitForFinished(-1);
            if (lltagProcess->exitCode() != QProcess::NormalExit) {
                qWarning() << "updateTags(): unable to update tags using lltag. Using taglib fallback...";
            }
        } else {
            // Use taglib to update artist, album, track nr and name.
            TagLib::FileRef currentTrack(entryPath.c_str(), true, TagLib::AudioProperties::Fast);
            auto currentTag = currentTrack.tag();
            currentTag->setArtist(artistName.toStdString());
            currentTag->setAlbum(albumName.toStdString());
            currentTag->setTitle(trackName.toStdString());
            currentTag->setTrack(trackNr.toInt());
            currentTrack.save();
        }
    } else {
        qCritical() << "updateTags(): track name does not match pattern: " << entryName;
    }
}

void xMusicLibraryTrackEntry::scanTags() const {
    // Scanning for tags only on demand if corresponding properties are accessed. Therefor const and mutable.
    // Check if we have a file defined.
    if (isScanned() || (entryPath.empty())) {
        return;
    }
    // Use taglib to determine the sample rate, bitrate, bits per sample and length.
    TagLib::FileRef currentTrack(entryPath.c_str(), true, TagLib::AudioProperties::Fast);
    TagLib::AudioProperties* currentTrackProperties = currentTrack.audioProperties();
    if (currentTrackProperties == nullptr) {
        qCritical() << "Unable to get audio proprties.";
        return;
    }
    // Most files do only support 16 bits per sample.
    trackBitsPerSample = 16;
    try {
        auto trackPathExt = QString::fromStdString(entryPath.extension());
        if (trackPathExt.compare(".flac", Qt::CaseInsensitive) == 0) {
            auto* currentFlacProperties = dynamic_cast<TagLib::FLAC::Properties*>(currentTrackProperties);
            trackBitsPerSample = currentFlacProperties->bitsPerSample();
        } else if (trackPathExt.compare(".wv", Qt::CaseInsensitive) == 0) {
            auto* currentWvProperties = dynamic_cast<TagLib::WavPack::Properties*>(currentTrackProperties);
            trackBitsPerSample = currentWvProperties->bitsPerSample();
        }
    } catch(const std::bad_cast& error) {
        // Ignore error.
        qCritical() << "Unable to scan properties for: "
                    << QString::fromStdString(entryPath.generic_string()) << ", error: " << error.what();
    }
    trackBitrate = currentTrackProperties->bitrate();
    trackSampleRate = currentTrackProperties->sampleRate();
    trackLength = currentTrackProperties->lengthInMilliseconds();
}

bool xMusicLibraryTrackEntry::equal(xMusicLibraryTrackEntry* track, bool checkFileSize) const {
    // The file path different because music libraries have different base directories.
    // The other attributes may not match. E.g. if the file is not scanned yet.
    return ((track != nullptr) &&
            (getArtistName() == track->getArtistName()) &&
            (getAlbumName() == track->getAlbumName()) &&
            (getTrackName() == track->getTrackName()) &&
            ((getFileSize() == track->getFileSize()) || (!checkFileSize)));
}

int xMusicLibraryTrackEntry::compareTrackName(xMusicLibraryTrackEntry* track) const {
    if (track) {
        // Perform comparison case-insensitive.
        return getTrackName().compare(track->getTrackName(), Qt::CaseSensitive);
    } else {
        // An existing track name is always greater than a non-existing one.
        return 1;
    }
}


bool xMusicLibraryTrackEntry::isDirectoryEntryValid(const std::filesystem::directory_entry& dirEntry) {
    // No directory entry scanning for tracks.
    Q_UNUSED(dirEntry)
    throw std::runtime_error("invalid call of xMusicLibraryTrackEntry::isDirectoryEntryValid");
}

xMusicLibraryEntry* xMusicLibraryTrackEntry::child(size_t index) {
    // tracks do not have any children.
    Q_UNUSED(index)
    return nullptr;
}

void xMusicLibraryTrackEntry::update() {
    updateTags();
}

void xMusicLibraryTrackEntry::updateParent(xMusicLibraryEntry* childEntry) {
    // no childs, therefor no updateParent necessary.
    Q_UNUSED(childEntry)
    throw std::runtime_error("invalid call of xMusicLibraryTrackEntry::updateParent");
}
