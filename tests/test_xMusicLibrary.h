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

#include <QtTest>
#include <QtTestWidgets>
#include <QMetaType>

#include "xMusicLibrary.h"

class test_xMusicLibrary:public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testScanInvalidLibrary();
    void testScannedArtists();
    void testScannedArtistsFiltered_data();
    void testScannedArtistsFiltered();
    void testScannedAlbums_data();
    void testScannedAlbums();
    void testScannedAlbumsFilter_data();
    void testScannedAlbumsFilter();
    void testScannedAllAlbumTracks_data();
    void testScannedAllAlbumTracks();
    void testScannedAllAlbumTracksFiltered_data();
    void testScannedAllAlbumTracksFiltered();
    void testScannedListArtistsAllAlbumTracks_data();
    void testScannedListArtistsAllAlbumTracks();
    void testScannedListArtistsAllAlbumTracksFilter_data();
    void testScannedListArtistsAllAlbumTracksFilter();
    void testScannedTracks_data();
    void testScannedTracks();

private:
    xMusicLibrary* musicLibrary;

};
