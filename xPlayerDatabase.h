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
    ~xPlayerDatabase() override = default;

    static xPlayerDatabase* playerDatabase;
    soci::session sqlDatabase;
};

#endif
