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

#include "xPlayerDatabase.h"
#include "xPlayerConfiguration.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

// singleton object.
xPlayerDatabase* xPlayerDatabase::playerDatabase = nullptr;

xPlayerDatabase::xPlayerDatabase(QObject* parent):
        QObject(parent) {
    loadDatabase();
    // Connect configuration to database file.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseDirectory,
            this, &xPlayerDatabase::updatedDatabaseDirectory);
}

xPlayerDatabase::~xPlayerDatabase() {
    // Close database.
    try {
        sqlDatabase.close();
    } catch (soci::soci_error& e) {
        // Ignore error.
    }
}

void xPlayerDatabase::updatedDatabaseDirectory() {
    // Close database.
    try {
        sqlDatabase.close();
    } catch (soci::soci_error& e) {
        // Ignore error.
    }
    loadDatabase();
}

void xPlayerDatabase::loadDatabase() {
    try {
        sqlDatabase.open(soci::sqlite3, xPlayerConfiguration::configuration()->getDatabasePath().toStdString());
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to open database: " << e.what();
        return;
    }
    try {
        // The following create table commands will fail if database already exists.
        // Create artist transition table.
        sqlDatabase << "CREATE TABLE transition (ID INTEGER PRIMARY KEY AUTOINCREMENT, fromArtist VARCHAR, fromAlbum VARCHAR, "
                       "toArtist VARCHAR, toAlbum VARCHAR, transitionCount INTEGER, timeStamp BIGINT)";
        // Create playlist and playlistSongs table.
        sqlDatabase << "CREATE TABLE playlist (ID INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR NOT NULL UNIQUE)";
        sqlDatabase << "CREATE TABLE playlistSongs (ID INTEGER PRIMARY KEY AUTOINCREMENT, playlistID INTEGER, hash VARCHAR)";
        // Create music table.
        sqlDatabase << "CREATE TABLE music (hash VARCHAR PRIMARY KEY, playCount INT, timeStamp BIGINT, "
                       "artist VARCHAR, album VARCHAR, track VARCHAR, sampleRate INT, bitsPerSample INT)";
        // Create movie table.
        sqlDatabase << "CREATE TABLE movie (hash VARCHAR PRIMARY KEY, playCount INT, timeStamp BIGINT, "
                       "tag VARCHAR, directory VARCHAR, movie VARCHAR)";
    } catch (soci::soci_error& e) {
        // Ignore error.
    }
}

xPlayerDatabase* xPlayerDatabase::database() {
    // Create and return singleton.
    if (playerDatabase == nullptr) {
        playerDatabase = new xPlayerDatabase();
    }
    return playerDatabase;
}

int xPlayerDatabase::getPlayCount(int bitsPerSample, int sampleRate, quint64 after) {
    int playCount = -1;
    soci::indicator playCountIndicator;
    try {
        if (bitsPerSample > 0) {
            if (sampleRate > 0) {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE bitsPerSample = :bitsPerSample AND sampleRate = :sampleRate AND timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(bitsPerSample), soci::use(sampleRate), soci::use(after);
            } else {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE bitsPerSample = :bitsPerSample AND timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(sampleRate), soci::use(after);
            }
        } else {
            if (sampleRate > 0) {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE sampleRate = :sampleRate AND timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(sampleRate), soci::use(after);
            } else {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(after);
            }
        }
        if ((playCountIndicator == soci::i_ok) && (playCount > 0)) {
            return playCount;
        } else {
            // No play count found.
            return 0;
        }
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for play count, error: " << e.what();
        return -1;
    }
}

int xPlayerDatabase::getPlayCount(const QString& artist, const QString& album, quint64 after) {
    int playCount = -1;
    soci::indicator playCountIndicator;
    try {
        if (artist.isEmpty()) {
            if (album.isEmpty()) {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(after);
            } else {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE album = :album AND timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(album.toStdString()), soci::use(after);
            }
        } else {
            if (album.isEmpty()) {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE artist = :artist AND timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(artist.toStdString()), soci::use(after);
            } else {
                sqlDatabase << "SELECT SUM(playCount) FROM music WHERE artist = :artist AND album = :album AND timeStamp >= :after",
                        soci::into(playCount, playCountIndicator), soci::use(artist.toStdString()), soci::use(album.toStdString()), soci::use(after);
            }
        }
        if ((playCountIndicator == soci::i_ok) && (playCount > 0)) {
            return playCount;
        } else {
            // No play count found.
            return 0;
        }
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for play count, error: " << e.what();
        return -1;
    }
}

QStringList xPlayerDatabase::getPlayedArtists(quint64 after) {
    QStringList artists;
    try {
        soci::rowset<std::string> distinctArtists = (sqlDatabase.prepare <<
                "SELECT DISTINCT(artist) FROM music WHERE timeStamp >= :after", soci::use(after));
        for (const auto& distinctArtist : distinctArtists) {
            artists.push_back(QString::fromStdString(distinctArtist));
        }
        artists.sort();
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for played artists, error: " << e.what();
        artists.clear();
    }
    return artists;
}

QStringList xPlayerDatabase::getPlayedAlbums(const QString& artist, quint64 after) {
    QStringList albums;
    try {
        soci::rowset<std::string> distinctAlbums = (sqlDatabase.prepare <<
                "SELECT DISTINCT(album) FROM music WHERE artist = :artist AND timeStamp >= :after",
                soci::use(artist.toStdString()), soci::use(after));
        for (const auto& distinctAlbum : distinctAlbums) {
            albums.push_back(QString::fromStdString(distinctAlbum));
        }
        albums.sort();
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for played albums for an artist, error: " << e.what();
        albums.clear();
    }
    return albums;
}

QList<std::tuple<QString,int,quint64>> xPlayerDatabase::getPlayedTracks(const QString& artist, const QString& album, quint64 after) {
    QList<std::tuple<QString,int,quint64>> tracks;
    try {
        soci::rowset<soci::row> playedTracks = (sqlDatabase.prepare <<
                "SELECT track, playCount, timeStamp FROM music WHERE artist = :artist AND album = :album AND timeStamp >= :after",
                soci::use(artist.toStdString()), soci::use(album.toStdString()), soci::use(after));
        for (const auto& playedTrack : playedTracks) {
            auto track = playedTrack.get<std::string>(0);
            auto playCount = playedTrack.get<int>(1);
            auto timeStamp = playedTrack.get<qint64>(2);
            tracks.push_back(std::make_tuple(QString::fromStdString(track), playCount, timeStamp));
        }
    } catch (soci::soci_error& e)  {
        qCritical() << "Unable to query database for played tracks for artist and album, error: " << e.what();
        tracks.clear();
    }
    return tracks;
}

QStringList xPlayerDatabase::getPlayedTags(quint64 after) {
    QStringList tags;
    try {
        soci::rowset<std::string> distinctTags = (sqlDatabase.prepare <<
                "SELECT DISTINCT(tag) FROM movie WHERE timeStamp >= :after", soci::use(after));
        for (const auto& distinctTag : distinctTags) {
            tags.push_back(QString::fromStdString(distinctTag));
        }
        tags.sort();
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for played artists, error: " << e.what();
        tags.clear();
    }
    return tags;
}

QStringList xPlayerDatabase::getPlayedDirectories(const QString& tag, quint64 after) {
    QStringList directories;
    try {
        soci::rowset<std::string> distinctDirectories = (sqlDatabase.prepare <<
                "SELECT DISTINCT(directory) FROM movie WHERE tag = :tag AND timeStamp >= :after",
                soci::use(tag.toStdString()), soci::use(after));
        for (const auto& distinctDirectory : distinctDirectories) {
            directories.push_back(QString::fromStdString(distinctDirectory));
        }
        directories.sort();
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for played albums for an artist, error: " << e.what();
        directories.clear();
    }
    return directories;
}

QList<std::tuple<QString,int,quint64>> xPlayerDatabase::getPlayedMovies(const QString& tag, const QString& directory, quint64 after) {
    QList<std::tuple<QString,int,quint64>> movies;
    try {
        soci::rowset<soci::row> playedMovies = (sqlDatabase.prepare <<
                "SELECT movie, playCount, timeStamp FROM movie WHERE tag = :tag AND directory = :directory AND timeStamp >= :after",
                soci::use(tag.toStdString()), soci::use(directory.toStdString()), soci::use(after));
        for (const auto& playedMovie : playedMovies) {
            auto movie = playedMovie.get<std::string>(0);
            auto playCount = playedMovie.get<int>(1);
            auto timeStamp = playedMovie.get<qint64>(2);
            movies.push_back(std::make_tuple(QString::fromStdString(movie), playCount, timeStamp));
        }
    } catch (soci::soci_error& e)  {
        qCritical() << "Unable to query database for played movies for tag and directory, error: " << e.what();
        movies.clear();
    }
    return movies;
}

std::pair<int,quint64> xPlayerDatabase::updateMusicFile(const QString& artist, const QString& album, const QString& track, int sampleRate, int bitsPerSample) {
    auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();
    try {
        int playCount = -1;
        soci::indicator playCountIndicator;
        sqlDatabase << "SELECT playCount FROM music WHERE hash=:hash", soci::into(playCount, playCountIndicator), soci::use(hash);
        if ((playCountIndicator == soci::i_ok) && (playCount >= 0)) {
            if (playCount > 0) {
                sqlDatabase << "UPDATE music SET playCount=:playCount,timeStamp=:timeStamp WHERE hash=:hash",
                        soci::use(playCount+1), soci::use(timeStamp), soci::use(hash);
            } else {
                // Update entries that were put in for the playlist but have not been played so far.
                sqlDatabase << "UPDATE music SET playCount=:playCount,timeStamp=:timeStamp,sampleRate=:sampleRate,bitsPerSample=:bitsPerSample WHERE hash=:hash",
                        soci::use(1), soci::use(timeStamp), soci::use(sampleRate), soci::use(bitsPerSample), soci::use(hash);
            }
            qDebug() << "xPlayerDatabase: update: " << artist+"/"+album+"/"+track << QString("(%1)").arg(playCount+1);
            return std::make_pair(playCount+1,timeStamp);
        } else {
            // Insert into the database if no element exists.
            sqlDatabase << "INSERT INTO music VALUES (:hash,:playCount,:timeStamp,:artist,:album,:track,:sampleRate,:bitsPerSample)",
                    soci::use(hash), soci::use(1), soci::use(timeStamp), soci::use(artist.toStdString()),
                    soci::use(album.toStdString()), soci::use(track.toStdString()), soci::use(sampleRate),
                    soci::use(bitsPerSample);
            qDebug() << "xPlayerDatabase: insert: " << artist + "/" + album + "/" + track;
            return std::make_pair(1,timeStamp);
        }
    } catch (soci::soci_error& e) {
        qCritical() << "xPlayerDatabase::updateMusicFile: error: " << e.what();
        emit databaseUpdateError();
    }
    return std::make_pair(0,0);
}

std::pair<int,quint64> xPlayerDatabase::updateMovieFile(const QString& movie, const QString& tag, const QString& directory) {
    auto hash = QCryptographicHash::hash((tag+"/"+directory+"/"+movie).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();
    try {
        int playCount = -1;
        soci::indicator playCountIndicator;
        sqlDatabase << "SELECT playCount FROM movie WHERE hash=:hash", soci::into(playCount, playCountIndicator), soci::use(hash);
        if ((playCountIndicator == soci::i_ok) && (playCount > 0)) {
            sqlDatabase << "UPDATE movie SET playCount=:playCount,timeStamp=:timeStamp WHERE hash=:hash",
                    soci::use(playCount+1), soci::use(timeStamp), soci::use(hash);
            qDebug() << "xPlayerDatabase: update: " << tag+"/"+directory+"/"+movie << QString("(%1)").arg(playCount);
            return std::make_pair(playCount+1,timeStamp);
        } else {
            // Insert into the database if no element exists.
            sqlDatabase << "INSERT INTO movie VALUES (:hash,:playCount,:timeStamp,:tag,:directory,:movie)",
                    soci::use(hash), soci::use(1), soci::use(timeStamp), soci::use(tag.toStdString()),
                    soci::use(directory.toStdString()), soci::use(movie.toStdString());
            qDebug() << "xPlayerDatabase: insert: " << tag + "/" + directory + "/" + movie;
            return std::make_pair(1,timeStamp);
        }
    } catch (soci::soci_error& e) {
        qCritical() << "xPlayerDatabase::updateMusicFile: error: " << e.what();
        emit databaseUpdateError();
    }
    return std::make_pair(0,0);
}

bool xPlayerDatabase::removeMusicPlaylist(const QString& name) {
    // Get ID for playlist name.
    qDebug() << "REMOVE: " << name;
    int playlistID = -1;
    try {
        // Retrieve ID for playlist name.
        soci::indicator playlistIDIndicator;
        sqlDatabase << "SELECT ID FROM playlist WHERE name=:name",
                soci::into(playlistID, playlistIDIndicator), soci::use(name.toStdString());
        if ((playlistIDIndicator != soci::i_ok) || (playlistID <= 0)) {
            qCritical() << "xPlayerDatabase: unable to retrieve playlist.";
            return false;
        }
        // Clear playlistSongs entries for current playlist and the name from the playlist.
        sqlDatabase << "DELETE FROM playlistSongs WHERE playlistID=:playlistID", soci::use(playlistID);
        sqlDatabase << "DELETE FROM playlist WHERE name=:name", soci::use(name.toStdString());
    } catch (soci::soci_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve playlist, error: " << e.what();
        return false;
    }
    return true;
}

QStringList xPlayerDatabase::getMusicPlaylists() {
    QStringList names;
    try {
        // Get the names for playlist.
        soci::rowset<std::string> playlistNames = (sqlDatabase.prepare << "SELECT name FROM playlist");
        for (const auto& playlistName : playlistNames) {
            names.push_back(QString::fromStdString(playlistName));
        }
    } catch (soci::soci_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve playlist names, error: " << e.what();
        return names;
    }
    return names;
}

std::vector<std::tuple<QString,QString,QString>> xPlayerDatabase::getMusicPlaylist(const QString& name) {
    std::vector<std::tuple<QString,QString,QString>> entries;
    // Get ID for playlist name.
    int playlistID = -1;
    try {
        // Retrieve ID for playlist name.
        soci::indicator playlistIDIndicator;
        sqlDatabase << "SELECT ID FROM playlist WHERE name=:name",
                soci::into(playlistID, playlistIDIndicator), soci::use(name.toStdString());
        if ((playlistIDIndicator != soci::i_ok) || (playlistID <= 0)) {
            qCritical() << "xPlayerDatabase: unable to retrieve playlist.";
            return entries;
        }
    } catch (soci::soci_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve playlist, error: " << e.what();
        return entries;
    }
    // Use inner join to properly retrieve playlist entries.
    try {
        soci::rowset<soci::row> playlistEntries = (sqlDatabase.prepare <<
                "SELECT music.artist, music.album, music.track FROM playlistSongs INNER JOIN "
                "music ON music.hash = playlistSongs.hash WHERE playlistID = :playlistID", soci::use(playlistID));
        for (const auto& playlistEntry : playlistEntries) {
            auto artist = playlistEntry.get<std::string>(0);
            auto album = playlistEntry.get<std::string>(1);
            auto track = playlistEntry.get<std::string>(2);
            entries.emplace_back(std::make_tuple(QString::fromStdString(artist), QString::fromStdString(album), QString::fromStdString(track)));
        }
    } catch (soci::soci_error& e)  {
        qCritical() << "Unable to query database for playlist, error: " << e.what();
        entries.clear();
    }
    return entries;
}

bool xPlayerDatabase::updateMusicPlaylist(const QString& name, const std::vector<std::tuple<QString,QString,QString>>& entries) {
    // Update playlist
    try {
        // Insert playlist name.
        sqlDatabase << "INSERT INTO playlist (name) VALUES (:name)", soci::use(name.toStdString());
    } catch (soci::soci_error& e) {
        // Ignore insert errors.
    }
    // Get ID for playlist name.
    int playlistID = -1;
    try {
        // Retrieve ID for playlist name.
        soci::indicator playlistIDIndicator;
        sqlDatabase << "SELECT ID FROM playlist WHERE name=:name",
                soci::into(playlistID, playlistIDIndicator), soci::use(name.toStdString());
        if ((playlistIDIndicator != soci::i_ok) || (playlistID <= 0)) {
            qCritical() << "xPlayerDatabase: unable to retrieve playlist.";
            return false;
        }
        // Clear playlistSongs entries for current playlist.
        sqlDatabase << "DELETE FROM playlistSongs WHERE playlistID=:playlistID", soci::use(playlistID);
    } catch (soci::soci_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve playlist, error: " << e.what();
        return false;
    }
    qDebug() << "xPlayerDatabase::updateMusicPlaylist: prepare list: existing hashes.";
    std::vector<std::string> hashExisting;
    try {
        // Get the names for playlist.
        soci::rowset<std::string> musicHashes = (sqlDatabase.prepare << "SELECT hash FROM music");
        for (const auto& hash : musicHashes) {
            hashExisting.emplace_back(hash);
        }
    } catch (soci::soci_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve hashes, error: " << e.what();
        return false;
    }
    std::sort(hashExisting.begin(), hashExisting.end());
    qDebug() << "xPlayerDatabase::updateMusicPlaylist: prepare list: compute hashes.";
    std::vector<std::string> hashEntries(entries.size());
    std::string addMusicEntries;
    std::string addPlaylistEntries;
    for (const auto& entry : entries) {
        auto artist = std::get<0>(entry);
        auto album = std::get<1>(entry);
        auto track = std::get<2>(entry);
        auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();

        if (!std::binary_search(hashExisting.begin(), hashExisting.end(), hash)) {
            if (!addMusicEntries.empty()) {
                addMusicEntries += ", ";
            }
            addMusicEntries += QString(R"(("%1", 0, -1, "%2", "%3", "%4", 0, 0))").arg(QString::fromStdString(hash)).arg(artist).arg(album).arg(track).toStdString();
        } else {
           qDebug() << "xPlayerDatabase::updateMusicPlaylist: skip: " << QString::fromStdString(hash);
        }
        if (!addPlaylistEntries.empty()) {
            addPlaylistEntries += ", ";
        }
        addPlaylistEntries += QString("(%1, \"%2\")").arg(playlistID).arg(QString::fromStdString(hash)).toStdString();
    }
    qDebug() << "xPlayerDatabase::updateMusicPlaylist: insert";
    // Update music and playlistSongs.
    try {
        // Insert into the music table if element need to be added.
        if (!addMusicEntries.empty()) {
            sqlDatabase << "INSERT INTO music VALUES " + addMusicEntries;
        }
        // Insert into playlistSongs
        sqlDatabase << "INSERT INTO playlistSongs (playlistID,hash) VALUES "+addPlaylistEntries;
    } catch (soci::soci_error& e) {
        qCritical() << "xPlayerDatabase: unable to insert, error: " << e.what();
        return false;
    }
    return true;
}

std::list<std::tuple<QString,QString,QString>> xPlayerDatabase::getAllTracks() {
    std::list<std::tuple<QString,QString,QString>> tracks;
    try {
        soci::rowset<soci::row> dbTracks = (sqlDatabase.prepare <<
                "SELECT artist, album, track FROM music ORDER BY artist, album");
        for (const auto& dbTrack : dbTracks) {
            auto artist = QString::fromStdString(dbTrack.get<std::string>(0));
            auto album = QString::fromStdString(dbTrack.get<std::string>(1));
            auto track = QString::fromStdString(dbTrack.get<std::string>(2));
            tracks.emplace_back(std::make_tuple(artist, album, track));
            qDebug() << "Entries: " << artist << "," << album << "," << track;
        }
    } catch (soci::soci_error& e)  {
        qCritical() << "Unable to query database for played tracks, error: " << e.what();
        tracks.clear();
    }
    return tracks;
}
std::list<std::tuple<QString,QString,QString>> xPlayerDatabase::getAllMovies() {
    std::list<std::tuple<QString,QString,QString>> movies;
    try {
        soci::rowset<soci::row> dbMovies = (sqlDatabase.prepare <<
                "SELECT tag, directory, movie FROM movie ORDER BY tag, directory");
        for (const auto& dbMovie : dbMovies) {
            auto tag = QString::fromStdString(dbMovie.get<std::string>(0));
            auto directory = QString::fromStdString(dbMovie.get<std::string>(1));
            auto movie = QString::fromStdString(dbMovie.get<std::string>(2));
            movies.emplace_back(std::make_tuple(tag, directory, movie));
            qDebug() << "Entries: " << tag << "," << directory << "," << movie;
        }
    } catch (soci::soci_error& e)  {
        qCritical() << "Unable to query database for all movies, error: " << e.what();
        movies.clear();
    }
    return movies;
}

void xPlayerDatabase::removeTracks(const std::list<std::tuple<QString, QString, QString>>& entries) {
    // We do not only need to remove the tracks in the music table,
    // but also the corresponding hashes in the playlistSongs table.
    auto whereArguments = convertEntriesToWhereArguments(entries);
    for (const auto& whereArgument : whereArguments) {
        removeFromTable("music", whereArgument);
        removeFromTable("playlistSongs", whereArgument);
    }
}

void xPlayerDatabase::removeMovies(const std::list<std::tuple<QString, QString, QString>>& entries) {
    // We only need to remove the movies from the movie table.
    auto whereArguments = convertEntriesToWhereArguments(entries);
    for (const auto& whereArgument : whereArguments) {
        removeFromTable("movie", whereArgument);
    }
}

std::list<std::string> xPlayerDatabase::convertEntriesToWhereArguments(const std::list<std::tuple<QString, QString, QString>>& entries) {
    std::list<std::string> whereArguments;
    std::string entryHash, whereArgument;
    int removeHashesCount = 0;
    for (const auto& entry : entries) {
        entryHash = QCryptographicHash::hash((std::get<0>(entry)+"/"+std::get<1>(entry)+"/"+std::get<2>(entry)).toUtf8(),
                                             QCryptographicHash::Sha256).toBase64().toStdString();
        whereArgument += " OR hash == \"" + entryHash + "\"";
        // We need to split the commands if we have too many entries.
        if (removeHashesCount >= 500) {
            whereArgument.replace(0, 3, " WHERE");
            whereArguments.push_back(whereArgument);
            whereArgument.clear();
            removeHashesCount=0;
        } else {
            ++removeHashesCount;
        }
    }
    if (!whereArgument.empty()) {
        whereArgument.replace(0, 3, " WHERE");
        whereArguments.push_back(whereArgument);
    }
    return whereArguments;
}

void xPlayerDatabase::removeFromTable(const std::string& tableName, const std::string& whereArgument) {
    // Remove entries from given table.
    try {
        // Insert playlist name.
        sqlDatabase << "DELETE FROM " + tableName + whereArgument;
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to remove tracks from " << QString::fromStdString(tableName) << " table, error: " << e.what();
    }
}
std::pair<int,quint64> xPlayerDatabase::updateTransition(const QString& fromArtist, const QString& fromAlbum,
                                                         const QString& toArtist, const QString& toAlbum,
                                                         bool shuffleMode) {
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();
    auto fromArtistStd = fromArtist.toStdString();
    auto fromAlbumStd = fromAlbum.toStdString();
    auto toArtistStd = toArtist.toStdString();
    auto toAlbumStd = toAlbum.toStdString();
    try {
        int transitionCount = -1;
        soci::indicator transitionCountIndicator;
        sqlDatabase << "SELECT transitionCount FROM transition WHERE fromArtist=:fromArtist AND fromAlbum=:fromAlbum "
                       "AND toArtist=:toArtist AND toAlbum=:toAlbum", soci::into(transitionCount, transitionCountIndicator),
                soci::use(fromArtistStd), soci::use(fromAlbumStd), soci::use(toArtistStd), soci::use(toAlbumStd);
        if ((transitionCountIndicator == soci::i_ok) && (transitionCount > 0)) {
            // We do not increase the transition count if in shuffle mode.
            if (shuffleMode) {
                return std::make_pair(transitionCount,timeStamp);
            }
            sqlDatabase << "UPDATE transition SET transitionCount=:transitionCount,timeStamp=:timeStamp WHERE "
                           " fromArtist=:fromArtist AND fromAlbum=:fromAlbum AND toArtist=:toArtist AND toAlbum=:toAlbum",
                    soci::use(transitionCount+1), soci::use(timeStamp), soci::use(fromArtistStd), soci::use(fromAlbumStd),
                    soci::use(toArtistStd), soci::use(toAlbumStd);
            qDebug() << "xPlayerDatabase: update: " << fromArtist << "/" << fromAlbum << "->"
                     << toArtist << "/" << toAlbum << ": " << QString("(%1)").arg(transitionCount+1);
            return std::make_pair(transitionCount+1,timeStamp);
        } else {
            // Insert into the database if no element exists.
            sqlDatabase << "INSERT INTO transition (fromArtist, fromAlbum, toArtist, toAlbum, transitionCount, timeStamp) VALUES "
                           "(:fromArtist,:fromAlbum,:toArtist,:toAlbum,:transitionCount,:timeStamp)",
                    soci::use(fromArtistStd), soci::use(fromAlbumStd), soci::use(toArtistStd), soci::use(toAlbumStd),
                    soci::use(1), soci::use(timeStamp);
            qDebug() << "xPlayerDatabase: insert: " << fromArtist << "/" << fromAlbum << "->" << toArtist << "/" << toAlbum;
            return std::make_pair(1,timeStamp);
        }
    } catch (soci::soci_error& e) {
        qCritical() << "xPlayerDatabase::updateTransition: error: " << e.what();
        emit databaseUpdateError();
    }
    return std::make_pair(0,0);
}

std::map<QString,std::set<QString>> xPlayerDatabase::getAllAlbums(quint64 after) {
    std::map<QString,std::set<QString>> mapArtistAlbum;
    try {
        soci::rowset<soci::row> artistAlbumRows = (sqlDatabase.prepare <<
            "SELECT artist, album FROM music WHERE timeStamp >= :after GROUP BY artist, album", soci::use(after));
        for (const auto& artistAlbumRow : artistAlbumRows) {
            auto artist = QString::fromStdString(artistAlbumRow.get<std::string>(0));
            auto album = QString::fromStdString(artistAlbumRow.get<std::string>(1));
            if (mapArtistAlbum.find(artist) == mapArtistAlbum.end()) {
                mapArtistAlbum[artist] = std::set<QString>();
            }
            mapArtistAlbum[artist].insert(album);
        }
    } catch (soci::soci_error& e) {
        qCritical() << "Unable to query database for played artists and albums, error: " << e.what();
        mapArtistAlbum.clear();
    }
    return mapArtistAlbum;

}
