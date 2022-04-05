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
#include "test_xMusicLibraryEntry.h"
#include "test_xMusicLibrary.h"
#include "test_xMovieLibrary.h"

#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"

#include <tuple>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    // Register Type
    qRegisterMetaType<xMusicLibraryTrackEntry>();
    qRegisterMetaType<xMusicLibraryTrackEntry*>();
    qRegisterMetaType<std::vector<xMusicLibraryTrackEntry*>>();
    qRegisterMetaType<QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>>();
    qRegisterMetaType<QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>>>();
    qRegisterMetaType<xMusicLibraryArtistEntry>();
    qRegisterMetaType<std::vector<xMusicLibraryArtistEntry*>>();
    qRegisterMetaType<xMusicLibraryAlbumEntry>();
    qRegisterMetaType<std::vector<xMusicLibraryAlbumEntry*>>();
    qRegisterMetaType<std::vector<std::pair<QString,QString>>>();

    test_xMusicLibraryTrackEntry musicLibraryTrackEntry;
    test_xMusicLibraryEntry musicLibraryEntry;
    test_xMusicLibrary musicLibrary;
    test_xMovieLibrary movieLibrary;

    return QTest::qExec(&musicLibraryTrackEntry, argc, argv) |
           QTest::qExec(&musicLibraryEntry, argc, argv) |
           QTest::qExec(&musicLibrary, argc, argv) |
           QTest::qExec(&movieLibrary, argc, argv);
}
