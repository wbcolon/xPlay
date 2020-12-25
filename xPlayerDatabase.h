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

#ifndef __XPLAYERDATABASE_H__
#define __XPLAYERDATABASE_H__

#include <QObject>
#include <QStringList>
#include <soci.h>
#include <sqlite3/soci-sqlite3.h>

class xPlayerDatabase:public QObject {
    Q_OBJECT

public:
    /**
     * Return the Application Database.
     *
     * @return pointer to a singleton of the database.
     */
    static xPlayerDatabase* database();
    /**
     * Return the sum of the play count based on bits per sample and sample rate.
     *
     * @param bitsPerSample specified bits per sample or 0 as wildcard.
     * @param sampleRate specified sample rate or 0 as wildcard.
     * @param after the time stamp after which played files are considered.
     * @return the sum of the play count as integer.
     */
    int getPlayCount(int bitsPerSample, int sampleRate, quint64 after=0);
    /**
     * Return the sum of the play count based on artist and album.
     *
     * @param artist specified artist of empty string as wildcard.
     * @param album specified album or empty string as wildcard.
     * @param after the time stamp after which played files are considered.
     * @return the sum of the play count as integer.
     */
    int getPlayCount(const QString& artist, const QString& album, quint64 after=0);
    /**
     * Return a list of played artists.
     *
     * @param after the time stamp used in the query the played tracks.
     * @return a list of artists played.
     */
    QStringList getPlayedArtists(quint64 after);
    /**
     * Return a list of played albums.
     *
     * @param artist the artist used the query of played tracks must match.
     * @param after the time stamp used in the query the played tracks.
     * @return a list of albums played.
     */
    QStringList getPlayedAlbums(const QString& artist, quint64 after);
    /**
     * Return a list of played tracks, their play count and the last time played.
     *
     * @param artist the artist used to query the played tracks must match.
     * @param album the album used to query the played tracks must match
     * @param after the time stamp used in the query the played tracks.
     * @return a list of tuples of tracks played with play count and time stamp.
     */
    QList<std::tuple<QString,int,quint64>> getPlayedTracks(const QString& artist, const QString& album, quint64 after);
    /**
     * Return a list of played tags.
     *
     * @param after the time stamp used in the query the played movies.
     * @return a list of tags played.
     */
    QStringList getPlayedTags(quint64 after);
    /**
     * Return a list of played directories.
     *
     * @param tag the tag used the query of played movies must match.
     * @param after the time stamp used in the query the played movies.
     * @return a list of directories played.
     */
    QStringList getPlayedDirectories(const QString& tag, quint64 after);
    /**
     * Return a list of played movies, their play count and the last time played.
     *
     * @param tag the tag used the query of played movies must match.
     * @param directory the directory the query of played movies must match.
     * @param after the time stamp used in the query the played movies.
     * @return a list of tuples of movies played with play count and time stamp.
     */
    QList<std::tuple<QString,int,quint64>> getPlayedMovies(const QString& tag, const QString& directory, quint64 after);

public slots:
    /**
     * Record the playing music file in the music table of the database.
     *
     * A hash is generated from the combination of the artist/album/track. This
     * hash is used as primary key in the database. If the music file is already
     * in the database then only its play count and time stamp are updated.
     *
     * @param artist the artist for the music file played.
     * @param album the album for the music file played.
     * @param track the track including the track number for the music file played.
     * @param sampleRate the sample rate for the music file played in Hz.
     * @param bitsPerSample the bits per sample for the music file played.
     */
    void updateMusicFile(const QString& artist, const QString& album, const QString& track, int sampleRate, int bitsPerSample);
    /**
     * Record the playing movie file in the movie table of the database.
     *
     * A hash is generated from the combination of tag/directory/movie. This
     * hash is used as primary key in the database. If the music file is already
     * in the database then only its play count and time stamp are updated.
     *
     * @param movie the file name displayed for the movie file played.
     * @param tag the tag the movie file played belongs to.
     * @param directory the directory the movie file played belongs to.
     */
    void updateMovieFile(const QString& movie, const QString& tag, const QString& directory);

private:
    explicit xPlayerDatabase(QObject* parent=nullptr);
    ~xPlayerDatabase() noexcept override;

    static xPlayerDatabase* playerDatabase;
    soci::session sqlDatabase;
};

#endif
