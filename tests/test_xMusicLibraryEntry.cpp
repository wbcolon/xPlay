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

#include "test_xMusicLibraryEntry.h"

#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"

#include <filesystem>

void test_xMusicLibraryEntry::testSimpleAccessFunctions() {
    xMusicLibraryAlbumEntry albumEntry("album", std::filesystem::path(), nullptr);
    QVERIFY(albumEntry.getAlbumName() == QString("album"));
}

void test_xMusicLibraryEntry::testAccessFunctions() {
    xMusicLibraryArtistEntry artistEntry("ac-dc", std::filesystem::path("../tests/input/musiclibrary/ac-dc"), nullptr);
    xMusicLibraryAlbumEntry albumEntry("back in black [hd]", std::filesystem::path("../tests/input/musiclibrary/ac-dc/back in black [hd]"), &artistEntry);
    QVERIFY(artistEntry.getArtistName() == QString("ac-dc"));
    QVERIFY(artistEntry.getPath() == std::filesystem::path("../tests/input/musiclibrary/ac-dc"));
    QVERIFY(albumEntry.getArtistName() == QString("ac-dc"));
    QVERIFY(albumEntry.getAlbumName() == QString("back in black [hd]"));
    QVERIFY(albumEntry.getPath() == std::filesystem::path("../tests/input/musiclibrary/ac-dc/back in black [hd]"));
    QVERIFY(albumEntry.getLastWritten() >= artistEntry.getLastWritten());
    QVERIFY(artistEntry < albumEntry);
}

