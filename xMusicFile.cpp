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

#include "xMusicFile.h"

#include <QDebug>

#include <taglib/fileref.h>
#include <taglib/audioproperties.h>
#include <taglib/flacproperties.h>
#include <taglib/wavpackproperties.h>

xMusicFile::xMusicFile():
        QObject(),
        filePath(),
        fileArtist(),
        fileAlbum(),
        fileTrackName(),
        fileLength(-1),
        fileBitsPerSample(-1),
        fileBitrate(-1),
        fileSampleRate(-1) {
}

xMusicFile::xMusicFile(const std::filesystem::path& file, const QString& artist, const QString& album,
                       const QString& trackName, QObject* parent):
        QObject(parent),
        filePath(file),
        fileArtist(artist),
        fileAlbum(album),
        fileTrackName(trackName),
        fileLength(-1),
        fileBitsPerSample(-1),
        fileBitrate(-1),
        fileSampleRate(-1) {
}

xMusicFile::xMusicFile(const xMusicFile& file):
        QObject(file.parent()),
        filePath(file.filePath),
        fileArtist(file.fileArtist),
        fileAlbum(file.fileAlbum),
        fileTrackName(file.fileTrackName),
        fileLength(file.fileLength),
        fileBitsPerSample(file.fileBitsPerSample),
        fileBitrate(file.fileBitrate),
        fileSampleRate(file.fileSampleRate) {
}

const std::filesystem::path& xMusicFile::getFilePath() const {
    return filePath;
}

const QString& xMusicFile::getArtist() const {
    return fileArtist;
}

const QString& xMusicFile::getAlbum() const {
    return fileAlbum;
}

const QString& xMusicFile::getTrackName() const {
    return fileTrackName;
}

qint64 xMusicFile::getLength() const {
    scan();
    return fileLength;
}

int xMusicFile::getBitsPerSample() const {
    scan();
    return fileBitsPerSample;
}

int xMusicFile::getSampleRate() const {
    scan();
    return fileSampleRate;
}

int xMusicFile::getBitrate() const {
    scan();
    return fileBitrate;
}

bool xMusicFile::isScanned() const {
    return (fileLength > 0);
}

void xMusicFile::scan() const {
    // Check if we have a file defined.
    if (isScanned() || (filePath.empty())) {
        return;
    }
    // Use taglib to determine the sample rate, bitrate, bits per sample and length.
    TagLib::FileRef currentTrack(filePath.c_str(), true, TagLib::AudioProperties::Fast);
    TagLib::AudioProperties* currentTrackProperties = currentTrack.audioProperties();
    if (currentTrackProperties == nullptr) {
        qCritical() << "xMusicFile: unable to get audio proprties.";
        return;
    }
    // Most files do only support 16 bits per sample.
    fileBitsPerSample = 16;
    try {
        auto filePathExt = QString::fromStdString(filePath.extension());
        if (filePathExt.compare(".flac", Qt::CaseInsensitive) == 0) {
            auto* currentFlacProperties = dynamic_cast<TagLib::FLAC::Properties*>(currentTrackProperties);
            fileBitsPerSample = currentFlacProperties->bitsPerSample();
        } else if (filePathExt.compare(".wv", Qt::CaseInsensitive) == 0) {
            auto* currentWvProperties = dynamic_cast<TagLib::WavPack::Properties*>(currentTrackProperties);
            fileBitsPerSample = currentWvProperties->bitsPerSample();
        }
    } catch(std::bad_cast& e) {
        // Ignore error.
        qCritical() << "xMusicFile: unable to scan properties for: "
                    << QString::fromStdString(filePath.generic_string()) << ", error: " << e.what();
    }
    fileBitrate = currentTrackProperties->bitrate();
    fileSampleRate = currentTrackProperties->sampleRate();
    fileLength = currentTrackProperties->lengthInMilliseconds();
}