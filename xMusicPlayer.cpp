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
#include "xMusicPlayer.h"

#include <QDebug>
#include <taglib/fileref.h>
#include <taglib/audioproperties.h>
#include <taglib/flacproperties.h>
#include <taglib/wavpackproperties.h>

xMusicPlayer::xMusicPlayer(QObject* parent):
        QObject(parent) {
}

void xMusicPlayer::setBaseDirectory(const QString& base) {
    // Set base directory for the music library. Necessary for creating absolute file paths.
    baseDirectory = base;
}

QString xMusicPlayer::pathFromQueueEntry(const std::tuple<QString, QString, QString>& elem) {
    // Create an absolute path from the music library base and the tuple of album,artist and track.
    return baseDirectory+"/"+std::get<0>(elem)+"/"+std::get<1>(elem)+"/"+std::get<2>(elem);
}

std::tuple<int,int,int> xMusicPlayer::propertiesFromFile(const QString& filename) {
    // Use taglib to determine the sample rate and bitrate.
    TagLib::FileRef currentTrack(filename.toStdString().c_str());
    TagLib::AudioProperties* currentTrackProperties = currentTrack.audioProperties();
    // Most files do only support 16 bits per sample.
    int bitsPerSample = 16;
    try {
        if (filename.endsWith(".flac", Qt::CaseInsensitive)) {
            TagLib::FLAC::Properties* currentFlacProperties = dynamic_cast<TagLib::FLAC::Properties*>(currentTrackProperties);
            bitsPerSample = currentFlacProperties->bitsPerSample();
        } else if (filename.endsWith(".wv", Qt::CaseInsensitive)) {
            TagLib::WavPack::Properties* currentWvProperties = dynamic_cast<TagLib::WavPack::Properties*>(currentTrackProperties);
            bitsPerSample = currentWvProperties->bitsPerSample();
        }
    } catch(std::bad_cast& e) {
        // Ignore error.
        qCritical() << "xMusicPlayer: unable to determine properties for: " << filename << ", error: " << e.what();
    }
    return std::make_tuple(currentTrackProperties->bitrate(), currentTrackProperties->sampleRate(), bitsPerSample);
}
