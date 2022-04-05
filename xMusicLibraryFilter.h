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

#ifndef __XMUSICLIBRARYFILTER_H__
#define __XMUSICLIBRARYFILTER_H__

#include <QStringList>
#include <QString>

#include <map>
#include <set>


class xMusicLibraryFilter {
public:
    xMusicLibraryFilter();
    ~xMusicLibraryFilter() = default;
    /**
     * Check if the filter has defined any artist related filters.
     *
     * @return true if any artist filter have been specified, false otherwise.
     */
    [[nodiscard]] bool hasArtistFilter() const;
    /**
     * Check if the filter has defined any album related filters.
     *
     * @return true if any album filter have been specified, false otherwise.
     */
    [[nodiscard]] bool hasAlbumFilter() const;
    /**
     * Check if the filter has defined any track name related filters.
     *
     * @return true if any track name filter have been been specified, false otherwise.
     */
    [[nodiscard]] bool hasTrackNameFilter() const;
    /**
     * Check if the given artist passes the artist filter.
     *
     * @param artist the given artist to be verified    bool rename(const QString& newEntryName);.
     * @return true if the artist passes the artist filter, false otherwise.
     */
    [[nodiscard]] bool isMatchingArtist(const QString& artist) const;
    /**
     * Check if the given album passes the album filter.
     *
     * @param album the given album to be verified.
     * @return true if the album passes the album filter, false otherwise.
     */
    [[nodiscard]] bool isMatchingAlbum(const QString& album) const;
    /**
     * Check if the given track name passes the track name filter.
     *
     * @param trackName the given track name to be verified.
     * @return true if the track name passes the track name filter, false otherwise.
     */
    [[nodiscard]] bool isMatchingTrackName(const QString& trackName) const;
    /**
     * Check if the given artist and album match the database mapping filter.
     *
     * @param artist the given artist to be verified.
     * @param album the given album to be verified.
     * @return artistAlbumNotMatch if the artist/album are not in the mapping, not artistAlbumNotMatch otherwise.
     */
    [[nodiscard]] bool isMatchingDatabaseArtistAndAlbum(const QString& artist, const QString& album) const;
    /**
     * Set the album match and not match. Overwrite old setting.
     *
     * @param match list of strings that must match.
     * @param notMatch list of strings that are not allowed to match.
     */
    void setAlbumMatch(const QStringList& match, const QStringList& notMatch);
    /**
     * Set the search match.
     *
     * @param match a tuple of artist, album and track name match.
     */
    void setSearchMatch(const std::tuple<QString,QString,QString>& match);
    /**
     * Add the database album match and not match.
     *
     * @param databaseMatch the artist and albums recorded in the database.
     * @param databaseNotMatch use databaseMatch to invert matching if true.
     */
    void setDatabaseMatch(const std::map<QString,std::set<QString>>& databaseMatch, bool databaseNotMatch);
    /**
     * Clear the album match.
     */
    void clearAlbumMatch();
    /**
     * Clear the database album match.
     */
    void clearDatabaseMatch();
    /**
     * Clear the search match.
     */
    void clearSearchMatch();
    /**
     * Clear all match and not match strings.
     */
    void clearMatch();

private:
    // Substrings that artist,album,track match or not match.
    QStringList albumMatch;
    QStringList albumNotMatch;
    QString artistSearchMatch;
    QString albumSearchMatch;
    QString trackNameSearchMatch;
    bool useArtistAlbumMatch;
    std::map<QString,std::set<QString>> artistAlbumMatch;
    bool artistAlbumNotMatch;
};


#endif
