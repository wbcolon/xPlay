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
#include <sqlite3.h>
#include <set>

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
    int getPlayCount(int bitsPerSample, int sampleRate, qint64 after=0);
    /**
     * Return the sum of the play count based on artist and album.
     *
     * @param artist specified artist of empty string as wildcard.
     * @param album specified album or empty string as wildcard.
     * @param after the time stamp after which played files are considered.
     * @return the sum of the play count as integer.
     */
    int getPlayCount(const QString& artist, const QString& album, qint64 after=0);
    /**
     * Return a list of played artists.
     *
     * @param after the time stamp used in the query the played tracks.
     * @return a list of artists played.
     */
    QStringList getPlayedArtists(qint64 after);
    /**
     * Return a list of played albums.
     *
     * @param artist the artist used the query of played tracks must match.
     * @param after the time stamp used in the query the played tracks.
     * @return a list of albums played.
     */
    QStringList getPlayedAlbums(const QString& artist, qint64 after);
    /**
     * Return a list of played tracks, their play count and the last time played.
     *
     * @param artist the artist used to query the played tracks must match.
     * @param album the album used to query the played tracks must match
     * @param after the time stamp used in the query the played tracks.
     * @return a list of tuples of tracks played with play count and time stamp.
     */
    QList<std::tuple<QString,int,qint64>> getPlayedTracks(const QString& artist, const QString& album, qint64 after);
    /**
     * Return a list of played tags.
     *
     * @param after the time stamp used in the query the played movies.
     * @return a list of tags played.
     */
    QStringList getPlayedTags(qint64 after);
    /**
     * Return a list of played directories.
     *
     * @param tag the tag used the query of played movies must match.
     * @param after the time stamp used in the query the played movies.
     * @return a list of directories played.
     */
    QStringList getPlayedDirectories(const QString& tag, qint64 after);
    /**
     * Return a list of played movies, their play count and the last time played.
     *
     * @param tag the tag used the query of played movies must match.
     * @param directory the directory the query of played movies must match.
     * @param after the time stamp used in the query the played movies.
     * @return a list of tuples of movies played with play count and time stamp.
     */
    QList<std::tuple<QString,int,qint64>> getPlayedMovies(const QString& tag, const QString& directory, qint64 after);
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
     * @return the pair of play count and time stamp for updated track.
     */
    std::pair<int,qint64> updateMusicFile(const QString& artist, const QString& album, const QString& track, int sampleRate, int bitsPerSample);
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
     * @return the pair of play count and time stamp for updated movie.
     */
    std::pair<int,qint64> updateMovieFile(const QString& movie, const QString& tag, const QString& directory);
    /**
     * Remove the playlist and its entries from the database.
     *
     * @param name the name of the playlist to be removed.
     */
    bool removeMusicPlaylist(const QString& name);
    /**
     * Return the names of the playlists.
     *
     * @return the list of playlists stored in the database as strings.
     */
    QStringList getMusicPlaylists();
    /**
     * Return the entries of the playlist with the given name.
     *
     * @param name the name of the playlist in the database.
     * @return the list of entries as tuples of artist, album and track.
     */
    std::vector<std::tuple<QString,QString,QString>> getMusicPlaylist(const QString& name);
    /**
     * Save the current queue to the database.
     *
     * A hash is generated from the combination of the artist/album/track. This
     * hash is used to update the music table is update with artist/album/track
     * entries and playCount of 0 and timeStamp of -1 if the entry is not yet
     * in the music table. The computed hash is also used to record the entry in
     * the playlistSongs database using the ID of the playlist table for name.
     * The update for both tables has been optimized to one insert operation each.
     *
     * @param name the name for the playlist.
     * @param entries the queue as list of tuples of artist/album/track.
     * @return true if the playlist was saved, false otherwise.
     */
    bool updateMusicPlaylist(const QString& name, const std::vector<std::tuple<QString,QString,QString>>& entries);
    /**
     * Return all tracks stored in the music table of the database.
     *
     * The play count and time stamp are ignored for this function. The output
     * can be used to verify the validity of the database entries against the
     * scanned music library.
     *
     * @return a list of tuples of artist, album and track as strings.
     */
    std::list<std::tuple<QString,QString,QString>> getAllTracks();
    /**
     * Return all movies stored in the movie table of the database.
     *
     * The play count and time stamp are ignored for this function. The output
     * can be used to verify the validity of the database entries against the
     * scanned movie library.
     *
     * @return a list of tuples of tag, directory and movie as strings.
     */
    std::list<std::tuple<QString,QString,QString>> getAllMovies();
    /**
     * Remove the list of entries from the music table of the database.
     *
     * Not only the entries of the music table are removed, but the also
     * the corresponding entries of the playlists stored in the playlistSongs
     * table. The hash ID is use to identify the entries.
     *
     * @param entries a list of tuples of artist, album and track.
     */
    void removeTracks(const std::list<std::tuple<QString,QString,QString>>& entries);
    /**
     * Remove the list of entries from the movie table of the database.
     *
     * @param entries a list of tuples of tag, directory and movie.
     */
    void removeMovies(const std::list<std::tuple<QString,QString,QString>>& entries);
    /**
     * Update the url for the given artist in the artistInfo table of the database.
     *
     * @param artist the artist name as string.
     * @param url the new url as string.
     */
    void updateArtistURL(const QString& artist, const QString& url);
    /**
     * Return the url for the given artist.
     *
     * @param artist the artist name as string.
     * @return the url as string if existent, an empty string otherwise.
     */
    QString getArtistURL(const QString& artist);
    /**
     * Remove the url stored for the given artist.
     *
     * @param artist the artist name as string.
     */
    void removeArtistURL(const QString& artist);
    /**
     * Update the transitions between artists and albums.
     *
     * @param fromArtist name of artist played before.
     * @param fromAlbum name of the album played before.
     * @param toArtist name of the artist currently played
     * @param toAlbum name of the album currently played.
     * @param shuffleMode state of the shuffle mode.
     * @return a pair of transition count and time stamp.
     */
    std::pair<int,qint64> updateTransition(const QString& fromArtist, const QString& fromAlbum,
                                            const QString& toArtist, const QString& toAlbum,
                                            bool shuffleMode);
    /**
     * Get the transitions from or to a given artist.
     *
     * @param artist name of the artist in the transition.
     * @return a sorted vector of pairs of artist name and transition count.
     */
    std::vector<std::pair<QString,int>> getArtistTransitions(const QString& artist);
    /**
     * Add tag to the specified song.
     *
     * @param artist the artist name for the song.
     * @param album the album name for the song.
     * @param track the track name for the song.
     * @param tag the new tag assigned to the song.
     */
    void addTag(const QString& artist, const QString& album, const QString& track, const QString& tag);
    /**
     * Remove tag from the specified song.
     *
     * @param artist the artist name for the song.
     * @param album the album name for the song.
     * @param track the track name for the song.
     * @param tags all tags assigned to the song.
     */
    void removeTag(const QString& artist, const QString& album, const QString& track, const QString& tag);
    /**
     * Remove all tag from the specified song.
     *
     * @param artist the artist name for the song.
     * @param album the album name for the song.
     * @param track the track name for the song.
     */
    void removeAllTags(const QString& artist, const QString& album, const QString& track);
    /**
     * Update the tags for the specified song.
     *
     * @param artist the artist name for the song.
     * @param album the album name for the song.
     * @param track the track name for the song.
     * @param tags all tags assigned to the song.
     */
    void updateTags(const QString& artist, const QString& album, const QString& track, const QStringList& tags);
    /**
     * Get the tags assigned to the specified song
     *
     * @param artist the artist name for the song.
     * @param album the album name for the song.
     * @param track the track name for the song.
     * @return a list of tags assigned to the song, or an empty list.
     */
    QStringList getTags(const QString& artist, const QString& album, const QString& track);
    /**
     * Get all songs for the specified tag.
     *
     * @param tag the tag used in the database query.
     * @return a vector of tuples of artist, album and track name.
     */
    std::vector<std::tuple<QString,QString,QString>> getAllForTag(const QString& tag);
    /**
     * Retrieve all played artist and albums with their latest timestamp.
     *
     * @param after the time stamp used in the query the played movies.
     *
     * @return a map from artist to a set of album.
     */
    std::map<QString,std::set<QString>> getAllAlbums(qint64 after);

signals:
    /**
     * Signal emitted in a database error occured.
     */
    void databaseUpdateError();

    void databaseError();

private slots:
    /**
     * Called upon changing the directory where the database is stored.
     */
    void updatedDatabaseDirectory();

private:
    explicit xPlayerDatabase(QObject* parent=nullptr);
    ~xPlayerDatabase() noexcept override;
    /**
     * Load the database from the path stored in the configuration.
     */
    void loadDatabase();
    /**
     * Wrapper that converts return results into runtime_errors.
     *
     * @param result the result of the sqlite3 command.
     * @param expectedResult the expected result.
     */
    void dbCheck(int result, int expectedResult=SQLITE_OK);
    /**
     * Generic function to remove entries from a table in the database.
     *
     * @param tableName the table name the entries are removed from.
     * @param whereArgument the where argument that is attached to the delete statement.
     */
    void removeFromTable(const std::string& tableName, const std::string& whereArgument);
    /**
     * Convert the a list of entries into a list of where arguments with hashes.
     *
     * The list needs to be split up if we have more than 500 entries due to the
     * limitations on the number of OR in a where argument.
     *
     * @param entries the list of entries to be converted to where arguments.
     * @return a list of where arguments with hashes in OR concatenation.
     */
    static std::list<std::string> convertEntriesToWhereArguments(const std::list<std::tuple<QString,QString,QString>>& entries);

    static xPlayerDatabase* playerDatabase;
    sqlite3* sqlDatabase;
};

#endif
