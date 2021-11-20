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

#include "test_xMusicFile.h"

void test_xMusicFile::testAccessFunctions() {
    xMusicFile musicFile(std::filesystem::path("/tmp/artist/album/01 track.flac"), 123456, "artist", "album", "01 track.flac");
    QVERIFY(musicFile.getArtist() == QString("artist"));
    QVERIFY(musicFile.getAlbum() == QString("album"));
    QVERIFY(musicFile.getTrackName() == QString("01 track.flac"));
    QVERIFY(musicFile.getFileSize() == 123456);
}

void test_xMusicFile::testScanInvalidFile() {
    xMusicFile musicFile(std::filesystem::path("/tmp/artist/album/01 track.flac"), 0, "artist", "album", "01 track.flac");
    QVERIFY(!musicFile.isScanned());
    QVERIFY(musicFile.getLength() <= 0);
    QVERIFY(musicFile.getBitrate() <= 0);
    QVERIFY(musicFile.getBitsPerSample() <= 0);
    QVERIFY(musicFile.getSampleRate() <= 0);
    QVERIFY(!musicFile.isScanned());
}

void test_xMusicFile::testScanFlacFile() {
    xMusicFile musicFile(std::filesystem::path("../tests/input/test_file.flac"), 0, "artist", "album", "01 track.flac");
    QVERIFY(musicFile.getLength() > 0);
    QVERIFY(musicFile.getBitrate() > 0);
    QVERIFY(musicFile.getBitsPerSample() == 16);
    QVERIFY(musicFile.getSampleRate() == 44100);
    QVERIFY(musicFile.isScanned());
}

void test_xMusicFile::testScanHDFlacFile() {
    xMusicFile musicFile(std::filesystem::path("../tests/input/test_file_hd.flac"), 0, "artist", "album", "01 track.flac");
    QVERIFY(musicFile.getLength() > 0);
    QVERIFY(musicFile.getBitrate() > 0);
    QVERIFY(musicFile.getBitsPerSample() == 24);
    QVERIFY(musicFile.getSampleRate() == 96000);
    QVERIFY(musicFile.isScanned());
}

void test_xMusicFile::testScanWavpackFile() {
    xMusicFile musicFile(std::filesystem::path("../tests/input/test_file.wv"), 0, "artist", "album", "01 track.flac");
    QVERIFY(musicFile.getLength() > 0);
    QVERIFY(musicFile.getBitrate() > 0);
    QVERIFY(musicFile.getBitsPerSample() == 16);
    QVERIFY(musicFile.getSampleRate() == 44100);
    QVERIFY(musicFile.isScanned());
}

void test_xMusicFile::testScanMP3File() {
    xMusicFile musicFile(std::filesystem::path("../tests/input/test_file.mp3"), 0, "artist", "album", "01 track.flac");
    QVERIFY(musicFile.getLength() > 0);
    QVERIFY(musicFile.getBitrate() > 0);
    QVERIFY(musicFile.getBitsPerSample() == 16);
    QVERIFY(musicFile.getSampleRate() == 44100);
    QVERIFY(musicFile.isScanned());
}

