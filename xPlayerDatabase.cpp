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
    try {
        sqlDatabase.open(soci::sqlite3, xPlayerConfiguration::configuration()->getDatabasePath().toStdString());
        // The following create table commands will fail if database already exists.
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

xPlayerDatabase::~xPlayerDatabase() {
    // Close database.
    try {
        sqlDatabase.close();
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
        qCritical() << "Unable to query database for played movies for tag and directory, error: " << e.what();
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

void xPlayerDatabase::updateMusicFile(const QString& artist, const QString& album, const QString& track, int sampleRate, int bitsPerSample) {
    auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();
    try {
        int playCount = -1;
        soci::indicator playCountIndicator;
        sqlDatabase << "SELECT playCount FROM music WHERE hash=:hash", soci::into(playCount, playCountIndicator), soci::use(hash);
        if ((playCountIndicator == soci::i_ok) && (playCount > 0)) {
            sqlDatabase << "UPDATE music SET playCount=:playCount,timeStamp=:timeStamp WHERE hash=:hash",
                    soci::use(playCount+1), soci::use(timeStamp), soci::use(hash);
            qDebug() << "xPlayerDatabase: update: " << artist+"/"+album+"/"+track << QString("(%1)").arg(playCount);
        } else {
            // Insert into the database if no element exists.
            sqlDatabase << "INSERT INTO music VALUES (:hash,:playCount,:timeStamp,:artist,:album,:track,:sampleRate,:bitsPerSample)",
                    soci::use(hash), soci::use(1), soci::use(timeStamp), soci::use(artist.toStdString()),
                    soci::use(album.toStdString()), soci::use(track.toStdString()), soci::use(sampleRate),
                    soci::use(bitsPerSample);
            qDebug() << "xPlayerDatabase: insert: " << artist + "/" + album + "/" + track;
        }
    } catch (soci::soci_error& e) {
        qCritical() << "xPlayerDatabase::updateMusicFile: error: " << e.what();
    }
}

void xPlayerDatabase::updateMovieFile(const QString& movie, const QString& tag, const QString& directory) {
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
        } else {
            // Insert into the database if no element exists.
            sqlDatabase << "INSERT INTO movie VALUES (:hash,:playCount,:timeStamp,:tag,:directory,:movie)",
                    soci::use(hash), soci::use(1), soci::use(timeStamp), soci::use(tag.toStdString()),
                    soci::use(directory.toStdString()), soci::use(movie.toStdString());
            qDebug() << "xPlayerDatabase: insert: " << tag + "/" + directory + "/" + movie;
        }
    } catch (soci::soci_error& e) {
        qCritical() << "xPlayerDatabase::updateMusicFile: error: " << e.what();
    }
}
