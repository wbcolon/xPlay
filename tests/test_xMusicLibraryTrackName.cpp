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

#include "test_xMusicLibraryTrackName.h"

#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"


void test_xMusicLibraryTrackEntry::testAccessFunctions() {
    xMusicLibraryArtistEntry artistEntry("artist", QUrl::fromLocalFile("/tmp/artist"), nullptr);
    xMusicLibraryArtistEntry albumEntry("album", QUrl::fromLocalFile("/tmp/artist/album"), &artistEntry);
    xMusicLibraryTrackEntry trackEntry("01 track.flac", QUrl::fromLocalFile("/tmp/artist/album/01 track.flac"),
                                       "/tmp/artist/album/01 track.flac", 1, &albumEntry);
    QVERIFY(trackEntry.getArtistName() == QString("artist"));
    QVERIFY(trackEntry.getAlbumName() == QString("album"));
    QVERIFY(trackEntry.getTrackName() == QString("01 track.flac"));
    // track does not exist, therefore -1
    QVERIFY(trackEntry.getFileSize() == static_cast<std::uintmax_t>(-1));
}

void test_xMusicLibraryTrackEntry::testScanInvalidFile() {
    xMusicLibraryTrackEntry trackEntry("01 track.flac", QUrl::fromLocalFile("/tmp/artist/album/01 track.flac"),
                                       "/tmp/artist/album/01 track.flac", 1, nullptr);
    QVERIFY(!trackEntry.isScanned());
    QVERIFY(trackEntry.getLength() <= 0);
    QVERIFY(trackEntry.getBitrate() <= 0);
    QVERIFY(trackEntry.getBitsPerSample() <= 0);
    QVERIFY(trackEntry.getSampleRate() <= 0);
    QVERIFY(!trackEntry.isScanned());
}

void test_xMusicLibraryTrackEntry::testScanFlacFile() {
    xMusicLibraryTrackEntry trackEntry("test_file.flac", QUrl::fromLocalFile("../tests/input/test_file.flac"),
                                       "../tests/input/test_file.flac", -1, nullptr);
    QVERIFY(trackEntry.getFileSize() != static_cast<std::uintmax_t>(-1));
    QVERIFY(trackEntry.getLength() > 0);
    QVERIFY(trackEntry.getBitrate() > 0);
    QVERIFY(trackEntry.getBitsPerSample() == 16);
    QVERIFY(trackEntry.getSampleRate() == 44100);
    QVERIFY(trackEntry.isScanned());
}

void test_xMusicLibraryTrackEntry::testScanHDFlacFile() {
    xMusicLibraryTrackEntry trackEntry("test_file_hd.flac", QUrl::fromLocalFile("../tests/input/test_file_hd.flac"),
                                       "../tests/input/test_file_hd.flac", -1, nullptr);
    QVERIFY(trackEntry.getFileSize() != static_cast<std::uintmax_t>(-1));
    QVERIFY(trackEntry.getLength() > 0);
    QVERIFY(trackEntry.getBitrate() > 0);
    QVERIFY(trackEntry.getBitsPerSample() == 24);
    QVERIFY(trackEntry.getSampleRate() == 96000);
    QVERIFY(trackEntry.isScanned());
}

void test_xMusicLibraryTrackEntry::testScanWavpackFile() {
    xMusicLibraryTrackEntry trackEntry("test_file.wv", QUrl::fromLocalFile("../tests/input/test_file.wv"),
                                       "../tests/input/test_file.wv", -1, nullptr);
    QVERIFY(trackEntry.getFileSize() != static_cast<std::uintmax_t>(-1));
    QVERIFY(trackEntry.getLength() > 0);
    QVERIFY(trackEntry.getBitrate() > 0);
    QVERIFY(trackEntry.getBitsPerSample() == 16);
    QVERIFY(trackEntry.getSampleRate() == 44100);
    QVERIFY(trackEntry.isScanned());
}

void test_xMusicLibraryTrackEntry::testScanMP3File() {
    xMusicLibraryTrackEntry trackEntry("test_file.mp3", QUrl::fromLocalFile("../tests/input/test_file.mp3"),
                                       "../tests/input/test_file.mp3", -1, nullptr);
    QVERIFY(trackEntry.getFileSize() != static_cast<std::uintmax_t>(-1));
    QVERIFY(trackEntry.getLength() > 0);
    QVERIFY(trackEntry.getBitrate() > 0);
    QVERIFY(trackEntry.getBitsPerSample() == 16);
    QVERIFY(trackEntry.getSampleRate() == 44100);
    QVERIFY(trackEntry.isScanned());
}

