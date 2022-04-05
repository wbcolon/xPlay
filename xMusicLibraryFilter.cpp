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

#include "xMusicLibraryFilter.h"


xMusicLibraryFilter::xMusicLibraryFilter():
        albumMatch(),
        albumNotMatch(),
        artistSearchMatch(),
        albumSearchMatch(),
        trackNameSearchMatch(),
        useArtistAlbumMatch(false),
        artistAlbumMatch(),
        artistAlbumNotMatch(false) {
}

bool xMusicLibraryFilter::hasArtistFilter() const {
    return ((!artistSearchMatch.isEmpty()) || (useArtistAlbumMatch));
}

bool xMusicLibraryFilter::hasAlbumFilter() const {
    return ((!albumMatch.isEmpty()) || (!albumNotMatch.isEmpty()) ||
            (!albumSearchMatch.isEmpty()) || (useArtistAlbumMatch));
}

bool xMusicLibraryFilter::hasTrackNameFilter() const {
    return (!trackNameSearchMatch.isEmpty());
}

bool xMusicLibraryFilter::isMatchingArtist(const QString& artist) const {
    return artist.contains(artistSearchMatch, Qt::CaseInsensitive);
}

bool xMusicLibraryFilter::isMatchingAlbum(const QString& album) const {
    if (!album.contains(albumSearchMatch, Qt::CaseInsensitive)) {
        return false;
    }
    if ((!albumNotMatch.isEmpty()) &&
        (std::any_of(albumNotMatch.begin(), albumNotMatch.end(), [&album](const QString& notMatch) {
            return album.contains(notMatch, Qt::CaseInsensitive);
        }))) {
        return false;
    }

    return (std::all_of(albumMatch.begin(), albumMatch.end(), [&album](const QString& match) {
        return album.contains(match, Qt::CaseInsensitive);
    }));
}

bool xMusicLibraryFilter::isMatchingTrackName(const QString& trackName) const {
    return trackName.contains(trackNameSearchMatch, Qt::CaseInsensitive);
}

bool xMusicLibraryFilter::isMatchingDatabaseArtistAndAlbum(const QString& artist, const QString& album) const {
    // No database matching im map is empty.
    if (!useArtistAlbumMatch) {
        return true;
    }
    auto artistPos = artistAlbumMatch.find(artist);
    if (artistPos != artistAlbumMatch.end()) {
        if (artistPos->second.find(album) != artistPos->second.end()) {
            return !artistAlbumNotMatch;
        }
    }
    return artistAlbumNotMatch;
}

void xMusicLibraryFilter::setAlbumMatch(const QStringList& match, const QStringList& notMatch) {
    albumMatch = match;
    albumNotMatch = notMatch;
}

void xMusicLibraryFilter::setSearchMatch(const std::tuple<QString,QString,QString>& match) {
    artistSearchMatch = std::get<0>(match);
    albumSearchMatch = std::get<1>(match);
    trackNameSearchMatch = std::get<2>(match);
}

void xMusicLibraryFilter::setDatabaseMatch(const std::map<QString,std::set<QString>>& databaseMatch, bool databaseNotMatch) {
    useArtistAlbumMatch = true;
    artistAlbumMatch = databaseMatch;
    artistAlbumNotMatch = databaseNotMatch;
}

void xMusicLibraryFilter::clearAlbumMatch() {
    albumMatch.clear();
    albumNotMatch.clear();
}

void xMusicLibraryFilter::clearDatabaseMatch() {
    useArtistAlbumMatch = false;
    artistAlbumMatch.clear();
    artistAlbumNotMatch = false;
}

void xMusicLibraryFilter::clearSearchMatch() {
    artistSearchMatch.clear();
    albumSearchMatch.clear();
    trackNameSearchMatch.clear();
}

void xMusicLibraryFilter::clearMatch() {
    clearAlbumMatch();
    clearSearchMatch();
    clearDatabaseMatch();
}


