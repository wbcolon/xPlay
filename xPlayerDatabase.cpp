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
        QObject(parent),
        sqlDatabase(nullptr) {
    loadDatabase();
    // Connect configuration to database file.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseDirectory,
            this, &xPlayerDatabase::updatedDatabaseDirectory);
}

xPlayerDatabase::~xPlayerDatabase() noexcept {
    // Close database.
    sqlite3_close(sqlDatabase);
}

void xPlayerDatabase::updatedDatabaseDirectory() {
    // Close database.
    sqlite3_close(sqlDatabase);
    loadDatabase();
}

void xPlayerDatabase::loadDatabase() {

    if (sqlite3_open(xPlayerConfiguration::configuration()->getDatabasePath().toStdString().c_str(), &sqlDatabase) != SQLITE_OK) {
        qCritical() << "Unable to open database: " << sqlite3_errmsg(sqlDatabase);
        return;
    }

    // The following create table commands will fail if database already exists.
    // Create taggedSongs table.
    sqlite3_exec(sqlDatabase, "CREATE TABLE taggedSongs (ID INTEGER PRIMARY KEY AUTOINCREMENT, tag VARCHAR, hash VARCHAR)",
                 nullptr, nullptr, nullptr);

    // Create artistInfo table.
    sqlite3_exec(sqlDatabase, "CREATE TABLE artistInfo (artist VARCHAR PRIMARY KEY, url VARCHAR)",
                 nullptr, nullptr, nullptr);
    // Create artist transition table.
    sqlite3_exec(sqlDatabase, "CREATE TABLE transition (ID INTEGER PRIMARY KEY AUTOINCREMENT, fromArtist VARCHAR, fromAlbum VARCHAR, "
                   "toArtist VARCHAR, toAlbum VARCHAR, transitionCount INTEGER, timeStamp BIGINT)",
                 nullptr, nullptr, nullptr);
    // Create playlist and playlistSongs table.
    sqlite3_exec(sqlDatabase, "CREATE TABLE playlist (ID INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR NOT NULL UNIQUE)",
                 nullptr, nullptr, nullptr);
    sqlite3_exec(sqlDatabase, "CREATE TABLE playlistSongs (ID INTEGER PRIMARY KEY AUTOINCREMENT, playlistID INTEGER, hash VARCHAR)",
                 nullptr, nullptr, nullptr);
    // Create music table.
    sqlite3_exec(sqlDatabase, "CREATE TABLE music (hash VARCHAR PRIMARY KEY, playCount INT, timeStamp BIGINT, "
                   "artist VARCHAR, album VARCHAR, track VARCHAR, sampleRate INT, bitsPerSample INT)",
                 nullptr, nullptr, nullptr);
    // Create movie table.
    sqlite3_exec(sqlDatabase, "CREATE TABLE movie (hash VARCHAR PRIMARY KEY, playCount INT, timeStamp BIGINT, "
                   "tag VARCHAR, directory VARCHAR, movie VARCHAR)", nullptr, nullptr, nullptr);
}

void xPlayerDatabase::dbCheck(int result, int expected) {
    if (result != expected) {
        throw std::runtime_error(sqlite3_errmsg(sqlDatabase));
    }
}

xPlayerDatabase* xPlayerDatabase::database() {
    // Create and return singleton.
    if (playerDatabase == nullptr) {
        playerDatabase = new xPlayerDatabase();
    }
    return playerDatabase;
}

int xPlayerDatabase::getPlayCount(int bitsPerSample, int sampleRate, qint64 after) {
    sqlite3_stmt* sqlStatement;
    try {
        if (bitsPerSample > 0) {
            if (sampleRate > 0) {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE bitsPerSample = ? "
                                                        "AND sampleRate = ? AND timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int(sqlStatement, 1, bitsPerSample));
                dbCheck(sqlite3_bind_int(sqlStatement, 2, sampleRate));
                dbCheck(sqlite3_bind_int64(sqlStatement, 3, after));
            } else {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE bitsPerSample = ? "
                                                        "AND timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int(sqlStatement, 1, bitsPerSample));
                dbCheck(sqlite3_bind_int64(sqlStatement, 2, after));
            }
        } else {
            if (sampleRate > 0) {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE sampleRate = ? "
                                                        "AND timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int(sqlStatement, 1, sampleRate));
                dbCheck(sqlite3_bind_int64(sqlStatement, 2, after));
            } else {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int64(sqlStatement, 1, after));
            }
        }
        auto playCount = 0;
        if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            playCount = sqlite3_column_int(sqlStatement, 0);
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        return playCount;
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for play count, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        return -1;
    }
}

int xPlayerDatabase::getPlayCount(const QString& artist, const QString& album, qint64 after) {
    sqlite3_stmt* sqlStatement;
    try {
        auto artistStd = artist.toStdString();
        auto albumStd = album.toStdString();
        if (artist.isEmpty()) {
            if (album.isEmpty()) {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int64(sqlStatement, 1, after));
            } else {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE album = ? "
                                                        "AND timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_text(sqlStatement, 1, albumStd.c_str(), static_cast<int>(albumStd.size()), nullptr));
                dbCheck(sqlite3_bind_int64(sqlStatement, 2, after));
            }
        } else {
            if (album.isEmpty()) {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE artist = ? "
                                                        "AND timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
                dbCheck(sqlite3_bind_int64(sqlStatement, 2, after));
            } else {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT SUM(playCount) FROM music WHERE artist = ? "
                                                        "AND album = ? AND timeStamp >= ?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
                dbCheck(sqlite3_bind_text(sqlStatement, 2, albumStd.c_str(), static_cast<int>(albumStd.size()), nullptr));
                dbCheck(sqlite3_bind_int64(sqlStatement, 3, after));
            }
        }
        auto playCount = 0;
        if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            playCount = sqlite3_column_int(sqlStatement, 0);
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        return playCount;
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for play count, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        return -1;
    }
}

QStringList xPlayerDatabase::getPlayedArtists(qint64 after) {
    QStringList artists;
    sqlite3_stmt* sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT DISTINCT(artist) FROM music WHERE timeStamp >= ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 1, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto artist = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0)));
            if (!artist.isEmpty()) {
                artists.push_back(artist);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        artists.sort();
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played artists, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        artists.clear();
    }
    return artists;
}

QStringList xPlayerDatabase::getPlayedAlbums(const QString& artist, qint64 after) {
    QStringList albums;
    sqlite3_stmt *sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT DISTINCT(album) FROM music WHERE artist == ? AND timeStamp >= ?",
                                   -1, &sqlStatement, nullptr));
        auto artistStd = artist.toStdString();
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 2, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto album = reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0));
            if (album != nullptr) {
                albums.push_back(QString::fromUtf8(album));
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        albums.sort();
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played albums for an artist, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        albums.clear();
    }
    return albums;
}

QList<std::tuple<QString,int,qint64>> xPlayerDatabase::getPlayedTracks(const QString& artist, const QString& album, qint64 after) {
    QList<std::tuple<QString,int,qint64>> tracks;
    sqlite3_stmt *sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT track, playCount, timeStamp FROM music WHERE artist = ? "
                                                " AND album = ? AND timeStamp >= ? GROUP BY track",
                                   -1, &sqlStatement, nullptr));
        auto artistStd = artist.toStdString();
        auto albumStd = album.toStdString();
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, albumStd.c_str(), static_cast<int>(albumStd.size()), nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 3, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto track = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            auto playCount = sqlite3_column_int(sqlStatement, 1);
            auto timeStamp = sqlite3_column_int64(sqlStatement, 2);
            if (!track.isEmpty()) {
                tracks.push_back(std::make_tuple(track, playCount, timeStamp));
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played tracks for artist and album, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        tracks.clear();
    }
    return tracks;
}

QStringList xPlayerDatabase::getPlayedTags(qint64 after) {
    QStringList tags;
    sqlite3_stmt* sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT DISTINCT(tag) FROM movie WHERE timeStamp >= ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 1, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto tag = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0)));
            if (!tag.isEmpty()) {
                tags.push_back(tag);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        tags.sort();
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played tags, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        tags.clear();
    }
    return tags;
}

QStringList xPlayerDatabase::getPlayedDirectories(const QString& tag, qint64 after) {
    QStringList directories;
    sqlite3_stmt *sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase,
                                   "SELECT DISTINCT(directory) FROM movie WHERE tag = ? AND timeStamp >= ?",
                                   -1, &sqlStatement, nullptr));
        auto tagStd = tag.toStdString();
        dbCheck(sqlite3_bind_text(sqlStatement, 1, tagStd.c_str(), static_cast<int>(tagStd.size()), nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 2, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto directory = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            if (!directory.isEmpty()) {
                directories.push_back(directory);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        directories.sort();
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played directories for a tag, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        directories.clear();
    }
    return directories;
}

QList<std::tuple<QString,int,qint64>> xPlayerDatabase::getPlayedMovies(const QString& tag, const QString& directory, qint64 after) {
    QList<std::tuple<QString,int,qint64>> movies;
    sqlite3_stmt *sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT movie, playCount, timeStamp FROM movie WHERE tag = ? "
                                                "AND directory = ? AND timeStamp >= ?",
                                   -1, &sqlStatement, nullptr));
        auto tagStd = tag.toStdString();
        auto directoryStd = directory.toStdString();
        dbCheck(sqlite3_bind_text(sqlStatement, 1, tagStd.c_str(), static_cast<int>(tagStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, directoryStd.c_str(), static_cast<int>(directoryStd.size()),
                                  nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 3, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto movie = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            auto playCount = sqlite3_column_int(sqlStatement, 1);
            auto timeStamp = sqlite3_column_int64(sqlStatement, 2);
            if (!movie.isEmpty()) {
                movies.push_back(std::make_tuple(movie, playCount, timeStamp));
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played movies for tag and directory, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        movies.clear();
    }
    return movies;
}

std::pair<int,qint64> xPlayerDatabase::updateMusicFile(const QString& artist, const QString& album, const QString& track, int sampleRate, int bitsPerSample) {
    auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();
    sqlite3_stmt *sqlStatement;

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT playCount FROM music WHERE hash = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto playCount = sqlite3_column_int(sqlStatement, 0);
            dbCheck(sqlite3_finalize(sqlStatement));
            if (playCount > 0) {
                dbCheck(sqlite3_prepare_v2(sqlDatabase, "UPDATE music SET playCount=?,timeStamp=? WHERE hash=?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int(sqlStatement, 1, playCount + 1));
                dbCheck(sqlite3_bind_int64(sqlStatement, 2, timeStamp));
                dbCheck(sqlite3_bind_text(sqlStatement, 3, hash.c_str(), static_cast<int>(hash.size()), nullptr));
            } else {
                // Update entries that were put in for the playlist but have not been played so far.
                dbCheck(sqlite3_prepare_v2(sqlDatabase,
                                           "UPDATE music SET playCount=?,timeStamp=?,sampleRate=?,bitsPerSample=? WHERE hash=?",
                                           -1, &sqlStatement, nullptr));
                dbCheck(sqlite3_bind_int(sqlStatement, 1, playCount + 1));
                dbCheck(sqlite3_bind_int64(sqlStatement, 2, timeStamp));
                dbCheck(sqlite3_bind_int(sqlStatement, 3, sampleRate));
                dbCheck(sqlite3_bind_int(sqlStatement, 4, bitsPerSample));
                dbCheck(sqlite3_bind_text(sqlStatement, 5, hash.c_str(), static_cast<int>(hash.size()), nullptr));
            }
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));

            return std::make_pair(playCount + 1, timeStamp);
        } else {
            auto artistStd = artist.toStdString();
            auto albumStd = album.toStdString();
            auto trackStd = track.toStdString();

            // Insert into the database if no element exists.
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO music VALUES (?,?,?,?,?,?,?,?)",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 2, 1));
            dbCheck(sqlite3_bind_int64(sqlStatement, 3, timeStamp));
            dbCheck(sqlite3_bind_text(sqlStatement, 4, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 5, albumStd.c_str(), static_cast<int>(albumStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 6, trackStd.c_str(), static_cast<int>(trackStd.size()), nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 7, sampleRate));
            dbCheck(sqlite3_bind_int(sqlStatement, 8, bitsPerSample));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
            return std::make_pair(1, timeStamp);
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::updateMusicFile: error: " << e.what();
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
    return std::make_pair(0, 0);
}

void xPlayerDatabase::renameMusicFile(const QString& artist, const QString& album, const QString& track, const QString& newTrack) {
    auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto newHash = QCryptographicHash::hash((artist+"/"+album+"/"+newTrack).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    sqlite3_stmt *sqlStatement;

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT playCount,timeStamp,sampleRate,bitsPerSample FROM music WHERE hash = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            // Bind results.
            auto playCount = sqlite3_column_int(sqlStatement, 0);
            auto timeStamp = sqlite3_column_int64(sqlStatement, 1);
            auto sampleRate = sqlite3_column_int(sqlStatement, 2);
            auto bitsPerSample = sqlite3_column_int(sqlStatement, 3);
            dbCheck(sqlite3_finalize(sqlStatement));
            // Insert new track.
            auto artistStd = artist.toStdString();
            auto albumStd = album.toStdString();
            auto newTrackStd = newTrack.toStdString();
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO music VALUES (?,?,?,?,?,?,?,?)",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, newHash.c_str(), static_cast<int>(newHash.size()), nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 2, playCount));
            dbCheck(sqlite3_bind_int64(sqlStatement, 3, timeStamp));
            dbCheck(sqlite3_bind_text(sqlStatement, 4, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 5, albumStd.c_str(), static_cast<int>(albumStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 6, newTrackStd.c_str(), static_cast<int>(newTrackStd.size()), nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 7, sampleRate));
            dbCheck(sqlite3_bind_int(sqlStatement, 8, bitsPerSample));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
            // Remove old track.
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM music WHERE hash = ?", -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
        } else {
            qCritical() << "xPlayerDatabase::renameMusicFile: entry does not exist: "
                        << artist << "," << album << "," << track;
            emit databaseUpdateError();
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::renameMusicFile: error: " << e.what();
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }

}

void xPlayerDatabase::renameMusicFiles(const QString& artist, const QString& album, const QString& newAlbum) {
    sqlite3_stmt *sqlStatement;

    try {
        std::vector<std::tuple<std::string, int, int64_t, std::string, int, int>> renameEntries;
        auto artistStd = artist.toStdString();
        auto albumStd = album.toStdString();
        auto newAlbumStd = newAlbum.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT hash,playCount,timeStamp,track,sampleRate,bitsPerSample FROM music WHERE artist = ? and album = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, albumStd.c_str(), static_cast<int>(albumStd.size()), nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            // Bind results.
            auto hash = std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0)));
            auto playCount = sqlite3_column_int(sqlStatement, 1);
            auto timeStamp = sqlite3_column_int64(sqlStatement, 2);
            auto track = std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3)));
            auto sampleRate = sqlite3_column_int(sqlStatement, 4);
            auto bitsPerSample = sqlite3_column_int(sqlStatement, 5);
            // Store entries to be renamed.
            renameEntries.emplace_back(std::make_tuple(hash, playCount, timeStamp, track, sampleRate, bitsPerSample));
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        // Create insert strings and delete strings.
        std::string addRenameEntries, removeOldEntries;
        for (const auto& renameEntry : renameEntries) {
            auto playCount = std::get<1>(renameEntry);
            qint64 timeStamp = std::get<2>(renameEntry);
            auto track = std::get<3>(renameEntry);
            auto sampleRate = std::get<4>(renameEntry);
            auto bitsPerSample = std::get<5>(renameEntry);
            auto hash = QCryptographicHash::hash(
                    (artist + "/" + newAlbum + "/" + QString::fromStdString(track)).toUtf8(),
                    QCryptographicHash::Sha256).toBase64().toStdString();
            if (!addRenameEntries.empty()) {
                addRenameEntries += ", ";
            }
            if (!removeOldEntries.empty()) {
                removeOldEntries += " OR ";
            }
            // Multi-argument arg seems to do strange things here.
            addRenameEntries += QString(R"(("%1", %2, %3, "%4", "%5", "%6", %7, %8))").  // clazy:exclude=qstring-arg
                    arg(QString::fromStdString(hash)).arg(playCount).arg(timeStamp).arg(artist).arg(newAlbum).  // clazy:exclude=qstring-arg
                    arg(QString::fromStdString(track)).arg(sampleRate).arg(bitsPerSample).toStdString();
            removeOldEntries += "hash = \"" + std::get<0>(renameEntry) +"\"";
        }
        // Insert new entries.
        if (!addRenameEntries.empty()) {
            auto sqlAddRenameEntries = "INSERT INTO music VALUES " + addRenameEntries;
            qDebug() << "xPlayerDatabase::renameMusicFiles: add entries: " << QString::fromStdString(sqlAddRenameEntries);
            dbCheck(sqlite3_exec(sqlDatabase, sqlAddRenameEntries.c_str(), nullptr, nullptr, nullptr));
        }
        // Delete old entries.
        if (!removeOldEntries.empty()) {
            auto sqlRemoveOldEntries = "DELETE FROM music WHERE " + removeOldEntries;
            qDebug() << "xPlayerDatabase::renameMusicFiles: remove entries: " << QString::fromStdString(sqlRemoveOldEntries);
            dbCheck(sqlite3_exec(sqlDatabase, sqlRemoveOldEntries.c_str(), nullptr, nullptr, nullptr));
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::renameMusicFiles: error: " << e.what();
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
}

void xPlayerDatabase::renameMusicFiles(const QString& artist, const QString& newArtist) {
    sqlite3_stmt *sqlStatement;

    try {
        std::vector<std::tuple<std::string, int, int64_t, std::string, std::string, int, int>> renameEntries;
        auto artistStd = artist.toStdString();
        auto newArtistStd = newArtist.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT hash,playCount,timeStamp,album,track,sampleRate,bitsPerSample FROM music WHERE artist = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            // Bind results.
            auto hash = std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0)));
            auto playCount = sqlite3_column_int(sqlStatement, 1);
            auto timeStamp = sqlite3_column_int64(sqlStatement, 2);
            auto album = std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 3)));
            auto track = std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 4)));
            auto sampleRate = sqlite3_column_int(sqlStatement, 5);
            auto bitsPerSample = sqlite3_column_int(sqlStatement, 6);
            // Store entries to be renamed.
            renameEntries.emplace_back(std::make_tuple(hash, playCount, timeStamp, album, track, sampleRate, bitsPerSample));
        }
        dbCheck(sqlite3_finalize(sqlStatement));
        // Create insert strings and delete strings.
        std::string addRenameEntries, removeOldEntries;
        for (const auto& renameEntry : renameEntries) {
            auto playCount = std::get<1>(renameEntry);
            qint64 timeStamp = std::get<2>(renameEntry);
            auto album = std::get<3>(renameEntry);
            auto track = std::get<4>(renameEntry);
            auto sampleRate = std::get<5>(renameEntry);
            auto bitsPerSample = std::get<6>(renameEntry);
            auto hash = QCryptographicHash::hash(
                    (newArtist + "/" + QString::fromStdString(album) + "/" + QString::fromStdString(track)).toUtf8(),
                    QCryptographicHash::Sha256).toBase64().toStdString();
            if (!addRenameEntries.empty()) {
                addRenameEntries += ", ";
            }
            if (!removeOldEntries.empty()) {
                removeOldEntries += " OR ";
            }
            // Multi-argument arg seems to do strange things here.
            addRenameEntries += QString(R"(("%1", %2, %3, "%4", "%5", "%6", %7, %8))").  // clazy:exclude=qstring-arg
                    arg(QString::fromStdString(hash)).arg(playCount).arg(timeStamp).arg(newArtist).  // clazy:exclude=qstring-arg
                    arg(QString::fromStdString(album)).arg(QString::fromStdString(track)).  // clazy:exclude=qstring-arg
                    arg(sampleRate).arg(bitsPerSample).toStdString();
            removeOldEntries += "hash = \"" + std::get<0>(renameEntry) +"\"";
        }
        // Insert new entries.
        if (!addRenameEntries.empty()) {
            auto sqlAddRenameEntries = "INSERT INTO music VALUES " + addRenameEntries;
            qDebug() << "xPlayerDatabase::renameMusicFiles: add entries: " << QString::fromStdString(sqlAddRenameEntries);
            dbCheck(sqlite3_exec(sqlDatabase, sqlAddRenameEntries.c_str(), nullptr, nullptr, nullptr));
        }
        qCritical() << "REMOVE";
        // Delete old entries.
        if (!removeOldEntries.empty()) {
            auto sqlRemoveOldEntries = "DELETE FROM music WHERE " + removeOldEntries;
            qDebug() << "xPlayerDatabase::renameMusicFiles: remove entries: " << QString::fromStdString(sqlRemoveOldEntries);
            dbCheck(sqlite3_exec(sqlDatabase, sqlRemoveOldEntries.c_str(), nullptr, nullptr, nullptr));
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::renameMusicFiles: error: " << e.what();
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
}

std::pair<int,qint64> xPlayerDatabase::updateMovieFile(const QString& movie, const QString& tag, const QString& directory) {
    auto hash = QCryptographicHash::hash((tag+"/"+directory+"/"+movie).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();

    sqlite3_stmt* sqlStatement;
    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT playCount FROM movie WHERE hash = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto playCount = sqlite3_column_int(sqlStatement, 0);
            dbCheck(sqlite3_finalize(sqlStatement));
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "UPDATE movie SET playCount=?,timeStamp=? WHERE hash=?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 1, playCount + 1));
            dbCheck(sqlite3_bind_int64(sqlStatement, 2, timeStamp));
            dbCheck(sqlite3_bind_text(sqlStatement, 3, hash.c_str(), static_cast<int>(hash.size()), nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
            return std::make_pair(playCount + 1, timeStamp);
        } else {
            auto tagStd = tag.toStdString();
            auto directoryStd = directory.toStdString();
            auto movieStd = movie.toStdString();

            // Insert into the database if no element exists.
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO movie VALUES (?,?,?,?,?,?)",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 2, 1));
            dbCheck(sqlite3_bind_int64(sqlStatement, 3, timeStamp));
            dbCheck(sqlite3_bind_text(sqlStatement, 4, tagStd.c_str(), static_cast<int>(tagStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 5, directoryStd.c_str(), static_cast<int>(directoryStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 6, movieStd.c_str(), static_cast<int>(movieStd.size()), nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
            return std::make_pair(1, timeStamp);
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::updateMovieFile: error: " << e.what();
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
    return std::make_pair(0, 0);
}

bool xPlayerDatabase::removeMusicPlaylist(const QString& name) {
    sqlite3_stmt* sqlStatement;

    try {
        auto nameStd = name.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT ID FROM playlist WHERE name = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, nameStd.c_str(), static_cast<int>(nameStd.size()), nullptr));
        if (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            auto playlistId = sqlite3_column_int(sqlStatement, 0);
            dbCheck(sqlite3_finalize(sqlStatement));
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM playlistSongs WHERE playlistID = ?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 1, playlistId));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM playlist WHERE name = ?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, nameStd.c_str(), static_cast<int>(nameStd.size()), nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
            return true;
        }
        return false;
    } catch (const std::runtime_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to remove playlist, error: " << e.what();
        return false;
    }
}

QStringList xPlayerDatabase::getMusicPlaylists() {
    QStringList names;
    sqlite3_stmt* sqlStatement;

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT name FROM playlist",
                                   -1, &sqlStatement, nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0)));
            if (!name.isEmpty()) {
                names.push_back(name);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to remove playlist, error: " << e.what();
        names.clear();
    }
    return names;
}

std::vector<std::tuple<QString,QString,QString>> xPlayerDatabase::getMusicPlaylist(const QString& name) {
    std::vector<std::tuple<QString,QString,QString>> entries;
    sqlite3_stmt* sqlStatement;

    try {
        auto nameStd = name.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT ID FROM playlist WHERE name = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, nameStd.c_str(), static_cast<int>(nameStd.size()), nullptr));
        if (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            auto playlistId = sqlite3_column_int(sqlStatement, 0);
            dbCheck(sqlite3_finalize(sqlStatement));
            // Use inner join to properly retrieve playlist entries.
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT music.artist, music.album, music.track FROM playlistSongs INNER JOIN "
                                       "music ON music.hash = playlistSongs.hash WHERE playlistID = ?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 1, playlistId));
            while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
                auto artist = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
                auto album = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 1)));
                auto track = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 2)));
                if ((!artist.isEmpty()) && (!album.isEmpty()) && (!track.isEmpty())) {
                    entries.emplace_back(std::make_tuple(artist, album, track));
                }
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to remove playlist, error: " << e.what();
        entries.clear();
    }
    return entries;
}

bool xPlayerDatabase::updateMusicPlaylist(const QString& name, const std::vector<std::tuple<QString,QString,QString>>& entries) {
    sqlite3_stmt* sqlStatement;
    auto nameStd = name.toStdString();
    auto playlistId = 0;

    try {
        // Insert playlist name.
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO playlist (name) VALUES (?)",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, nameStd.c_str(), static_cast<int>(nameStd.size()), nullptr));
        dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        // Ignore insert errors, just finalize statement.
        sqlite3_finalize(sqlStatement);
    }

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT ID FROM playlist WHERE name = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, nameStd.c_str(), static_cast<int>(nameStd.size()), nullptr));
        if (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            // Retrieve playlistId.
            playlistId = sqlite3_column_int(sqlStatement, 0);
            dbCheck(sqlite3_finalize(sqlStatement));
            // Clear playlistSongs entries for current playlist.
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM playlistSongs WHERE playlistID = ?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 1, playlistId));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
        }
    } catch (const std::runtime_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve playlist, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        return false;
    }

    qDebug() << "xPlayerDatabase::updateMusicPlaylist: prepare list: existing hashes.";
    std::vector<std::string> hashExisting;
    try {
        // Get the names for playlist.
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT hash FROM music", -1, &sqlStatement, nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto hash = std::string(reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0)));
            if (!hash.empty()) {
                hashExisting.emplace_back(hash);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        // Return on error.
        qCritical() << "xPlayerDatabase: unable to retrieve hashes, error: " << e.what();
        sqlite3_finalize(sqlStatement);
        return false;
    }

    qDebug() << "xPlayerDatabase::updateMusicPlaylist: prepare list: compute hashes.";
    std::sort(hashExisting.begin(), hashExisting.end());
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
            addMusicEntries += QString(R"(("%1", 0, -1, "%2", "%3", "%4", 0, 0))").arg(QString::fromStdString(hash), artist, album, track).toStdString();
        } else {
            qDebug() << "xPlayerDatabase::updateMusicPlaylist: skip: " << QString::fromStdString(hash);
        }
        if (!addPlaylistEntries.empty()) {
            addPlaylistEntries += ", ";
        }
        addPlaylistEntries += QString("(%1, \"%2\")").arg(playlistId).arg(QString::fromStdString(hash)).toStdString();
    }

    qDebug() << "xPlayerDatabase::updateMusicPlaylist: insert";
    // Update music and playlistSongs.
    try {
        // Insert into the music table if element need to be added.
        if (!addMusicEntries.empty()) {
            auto sqlAddMusicEntries = "INSERT INTO music VALUES " + addMusicEntries;
            dbCheck(sqlite3_exec(sqlDatabase, sqlAddMusicEntries.c_str(), nullptr, nullptr, nullptr));
        }
        // Insert into playlistSongs
        auto sqlAddPlaylistEntries = "INSERT INTO playlistSongs (playlistID,hash) VALUES "+addPlaylistEntries;
        dbCheck(sqlite3_exec(sqlDatabase, sqlAddPlaylistEntries.c_str(), nullptr, nullptr, nullptr));
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase: unable to insert, error: " << e.what();
        return false;
    }

    return true;
}

std::list<std::tuple<QString,QString,QString>> xPlayerDatabase::getAllTracks() {
    std::list<std::tuple<QString,QString,QString>> tracks;
    sqlite3_stmt* sqlStatement;

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT artist, album, track FROM music ORDER BY artist, album",
                                   -1, &sqlStatement, nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto artist = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            auto album = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 1)));
            auto track = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 2)));
            if ((!artist.isEmpty()) && (!album.isEmpty()) && (!track.isEmpty())) {
                tracks.emplace_back(std::make_tuple(artist, album, track));
                qDebug() << "Entries: " << artist << "," << album << "," << track;
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to get all tracks, error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        tracks.clear();
    }
    return tracks;
}

std::list<std::tuple<QString,QString,QString>> xPlayerDatabase::getAllMovies() {
    std::list<std::tuple<QString,QString,QString>> movies;
    sqlite3_stmt* sqlStatement;

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT tag, directory, movie FROM movie ORDER BY tag, directory",
                                   -1, &sqlStatement, nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto tag = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            auto directory = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 1)));
            auto movie = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 2)));
            if ((!tag.isEmpty()) && (!directory.isEmpty()) && (!movie.isEmpty())) {
                movies.emplace_back(std::make_tuple(tag, directory, movie));
                qDebug() << "Entries: " << tag << "," << directory << "," << movie;
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to get all movies, error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        movies.clear();
    }
    return movies;
}

void xPlayerDatabase::removeTracks(const std::list<std::tuple<QString, QString, QString>>& entries) {
    // We do not only need to remove the tracks in the music table,
    // but also the corresponding hashes in the playlistSongs table.
    try {
        auto whereArguments = convertEntriesToWhereArguments(entries);
        for (const auto &whereArgument : whereArguments) {
            removeFromTable("music", whereArgument);
            removeFromTable("playlistSongs", whereArgument);
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to remove tracks from the database, error: " << e.what();
        emit databaseUpdateError();
    }
}

void xPlayerDatabase::removeMovies(const std::list<std::tuple<QString, QString, QString>>& entries) {
    // We only need to remove the movies from the movie table.
    try {
        auto whereArguments = convertEntriesToWhereArguments(entries);
        for (const auto &whereArgument : whereArguments) {
            removeFromTable("movie", whereArgument);
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to remove movies from the database, error: " << e.what();
        emit databaseUpdateError();
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

QString xPlayerDatabase::getArtistURL(const QString& artist) {
    sqlite3_stmt* sqlStatement;
    auto artistStd = artist.toStdString();
    dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT url FROM artistInfo WHERE artist = ?",
                               -1, &sqlStatement, nullptr));
    dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
    if (sqlite3_step(sqlStatement) != SQLITE_DONE) {
        auto url = reinterpret_cast<const char*>(sqlite3_column_text(sqlStatement, 0));
        if (url != nullptr) {
            return QString::fromUtf8(url);
        }
    }
    return {};
}

void xPlayerDatabase::updateArtistURL(const QString& artist, const QString& url) {
    sqlite3_stmt* sqlStatement;
    try {
        auto artistStd = artist.toStdString();
        auto urlStd = url.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT url FROM artistInfo WHERE artist=? LIMIT 1",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        if (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            dbCheck(sqlite3_finalize(sqlStatement));
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "UPDATE artistInfo SET url=? WHERE artist=?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, urlStd.c_str(), static_cast<int>(urlStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 2, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
        } else {
            dbCheck(sqlite3_finalize(sqlStatement));
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO artistInfo VALUES (?,?)",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 2, urlStd.c_str(), static_cast<int>(urlStd.size()), nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::updateArtistURL: error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        emit databaseUpdateError();
    }
}

void xPlayerDatabase::removeArtistURL(const QString& artist) {
    sqlite3_stmt* sqlStatement;
    try {
        auto artistStd = artist.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM artistInfo WHERE artist=?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::removeArtistURL: error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        emit databaseUpdateError();
    }
}

std::pair<int,qint64> xPlayerDatabase::updateTransition(const QString& fromArtist, const QString& fromAlbum,
                                                         const QString& toArtist, const QString& toAlbum,
                                                         bool shuffleMode) {
    sqlite3_stmt* sqlStatement;

    try {
        auto timeStamp = QDateTime::currentMSecsSinceEpoch();
        auto fromArtistStd = fromArtist.toStdString();
        auto fromAlbumStd = fromAlbum.toStdString();
        auto toArtistStd = toArtist.toStdString();
        auto toAlbumStd = toAlbum.toStdString();

        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT transitionCount FROM transition WHERE fromArtist=? "
                                                "AND fromAlbum=? AND toArtist=? AND toAlbum=?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, fromArtistStd.c_str(), static_cast<int>(fromArtistStd.size()),
                                  nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, fromAlbumStd.c_str(), static_cast<int>(fromAlbumStd.size()),
                                  nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 3, toArtistStd.c_str(), static_cast<int>(toArtistStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 4, toAlbumStd.c_str(), static_cast<int>(toAlbumStd.size()), nullptr));
        if (sqlite3_step(sqlStatement) == SQLITE_ROW) {
            auto transitionCount = sqlite3_column_int(sqlStatement, 0);
            dbCheck(sqlite3_finalize(sqlStatement));
            if (shuffleMode) {
                return std::make_pair(transitionCount, timeStamp);
            }
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "UPDATE transition SET transitionCount=?,timeStamp=? WHERE "
                                                    " fromArtist=? AND fromAlbum=? AND toArtist=? AND toAlbum=?",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 1, transitionCount + 1));
            dbCheck(sqlite3_bind_int64(sqlStatement, 2, timeStamp));
            dbCheck(sqlite3_bind_text(sqlStatement, 3, fromArtistStd.c_str(), static_cast<int>(fromArtistStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 4, fromAlbumStd.c_str(), static_cast<int>(fromAlbumStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 5, toArtistStd.c_str(), static_cast<int>(toArtistStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 6, toAlbumStd.c_str(), static_cast<int>(toAlbumStd.size()),
                                      nullptr));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));

            return std::make_pair(transitionCount + 1, timeStamp);
        } else {
            dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO transition (fromArtist, fromAlbum, "
                                                    "toArtist, toAlbum, transitionCount, timeStamp) VALUES (?,?,?,?,?,?)",
                                       -1, &sqlStatement, nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 1, fromArtistStd.c_str(), static_cast<int>(fromArtistStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 2, fromAlbumStd.c_str(), static_cast<int>(fromAlbumStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 3, toArtistStd.c_str(), static_cast<int>(toArtistStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_text(sqlStatement, 4, toAlbumStd.c_str(), static_cast<int>(toAlbumStd.size()),
                                      nullptr));
            dbCheck(sqlite3_bind_int(sqlStatement, 5, 1));
            dbCheck(sqlite3_bind_int64(sqlStatement, 6, timeStamp));
            dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
            dbCheck(sqlite3_finalize(sqlStatement));

            return std::make_pair(1, timeStamp);
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::updateTransition: error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        emit databaseUpdateError();
    }

    return std::make_pair(0, 0);
}

std::vector<std::pair<QString,int>> xPlayerDatabase::getArtistTransitions(const QString& artist) {
    std::vector<std::pair<QString,int>> artistTransitions;
    sqlite3_stmt* sqlStatement;
    try {
        auto artistStd = artist.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase,
                                   "SELECT artist, SUM(count) FROM ( "
                                   "SELECT toArtist as artist, SUM(transitionCount) as count FROM transition WHERE fromArtist == ? GROUP BY toArtist "
                                   "UNION ALL "
                                   "SELECT fromArtist as artist, SUM(transitionCount) as count FROM transition WHERE toArtist == ? GROUP BY fromArtist "
                                   "ORDER BY 1 "
                                   ") GROUP BY artist ORDER BY 2 DESC",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, artistStd.c_str(), static_cast<int>(artistStd.size()), nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto transitionArtist = reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0));
            auto transitionCount = sqlite3_column_int(sqlStatement, 1);
            if (transitionArtist != nullptr) {
                artistTransitions.emplace_back(std::make_pair(QString::fromUtf8(transitionArtist), transitionCount));
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::getArtistTransitions: error: " << sqlite3_errmsg(sqlDatabase);
        artistTransitions.clear();
        sqlite3_finalize(sqlStatement);
    }
    return artistTransitions;
}

void xPlayerDatabase::addTag(const QString& artist, const QString& album, const QString& track, const QString& tag) {
    sqlite3_stmt* sqlStatement;

    try {
        auto hash = QCryptographicHash::hash((artist + "/" + album + "/" + track).toUtf8(),
                                             QCryptographicHash::Sha256).toBase64().toStdString();
        auto tagStd = tag.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "INSERT INTO taggedSongs (tag, hash) VALUES (?,?)",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, tagStd.c_str(), static_cast<int>(tagStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::addTag: error: " << sqlite3_errmsg(sqlDatabase);
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
}

void xPlayerDatabase::removeTag(const QString& artist, const QString& album, const QString& track, const QString& tag) {
    sqlite3_stmt* sqlStatement;

    try {
        auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(),
                                             QCryptographicHash::Sha256).toBase64().toStdString();
        auto tagStd = tag.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM taggedSongs WHERE tag == ? and hash == ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, tagStd.c_str(), static_cast<int>(tagStd.size()), nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 2, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::removeTag: error: " << sqlite3_errmsg(sqlDatabase);
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
}

void xPlayerDatabase::removeAllTags(const QString& artist, const QString& album, const QString& track) {
    sqlite3_stmt* sqlStatement;

    try {
        auto hash = QCryptographicHash::hash((artist + "/" + album + "/" + track).toUtf8(),
                                             QCryptographicHash::Sha256).toBase64().toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "DELETE FROM taggedSongs WHERE hash = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        dbCheck(sqlite3_step(sqlStatement), SQLITE_DONE);
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "xPlayerDatabase::removeTag: error: " << sqlite3_errmsg(sqlDatabase);
        emit databaseUpdateError();
        sqlite3_finalize(sqlStatement);
    }
}

void xPlayerDatabase::updateTags(const QString& artist, const QString& album, const QString& track,
                                 const QStringList& tags) {
    try {
        auto hash = QCryptographicHash::hash((artist + "/" + album + "/" + track).toUtf8(),
                                             QCryptographicHash::Sha256).toBase64().toStdString();
        // Remove old tags.
        removeFromTable("taggedSongs", " WHERE hash == \"" + hash + "\"");
        // Add new tags.
        std::string insertTags;
        for (const auto &tag : tags) {
            insertTags += "(\"" + tag.toStdString() + "\", \"" + hash + "\")";
        }
        if (!insertTags.empty()) {
            auto sqlInsertTags = "INSERT INTO taggedSongs (tag, hash) VALUES " + insertTags;
            dbCheck(sqlite3_exec(sqlDatabase, sqlInsertTags.c_str(), nullptr, nullptr, nullptr), SQLITE_OK);
        }
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to update tags for track, error: " << sqlite3_errmsg(sqlDatabase);
        emit databaseUpdateError();
    }
}

QStringList xPlayerDatabase::getTags(const QString& artist, const QString& album, const QString& track) {
    QStringList tags;
    sqlite3_stmt* sqlStatement;
    try {
        auto hash = QCryptographicHash::hash((artist + "/" + album + "/" + track).toUtf8(),
                                             QCryptographicHash::Sha256).toBase64().toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT tag FROM taggedSongs WHERE hash = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, hash.c_str(), static_cast<int>(hash.size()), nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto tag = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            if (!tag.isEmpty()) {
                tags.push_back(tag);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to get tags for track, error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        tags.clear();
    }
    return tags;
}

std::vector<std::tuple<QString, QString, QString>> xPlayerDatabase::getAllForTag(const QString& tag) {
    std::vector<std::tuple<QString,QString,QString>> entries;
    sqlite3_stmt* sqlStatement;

    try {
        auto tagStd = tag.toStdString();
        dbCheck(sqlite3_prepare_v2(sqlDatabase, "SELECT music.artist, music.album, music.track FROM taggedSongs "
                                                "INNER JOIN music ON music.hash = taggedSongs.hash WHERE tag = ?",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_text(sqlStatement, 1, tagStd.c_str(), static_cast<int>(tagStd.size()), nullptr));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto artist = reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0));
            auto album = reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 1));
            auto track = reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 2));
            if ((artist != nullptr) && (album != nullptr) && (track != nullptr)) {
                entries.emplace_back(std::make_tuple(QString::fromUtf8(artist), QString::fromUtf8(album), QString::fromUtf8(track)));
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to get all tracks for given tag, error: " << sqlite3_errmsg(sqlDatabase);
        sqlite3_finalize(sqlStatement);
        entries.clear();
    }
    return entries;
}

std::map<QString,std::set<QString>> xPlayerDatabase::getAllAlbums(qint64 after) {
    std::map<QString,std::set<QString>> mapArtistAlbum;
    sqlite3_stmt* sqlStatement;

    try {
        dbCheck(sqlite3_prepare_v2(sqlDatabase,
                                   "SELECT artist, album FROM music WHERE timeStamp >= ? GROUP BY artist, album",
                                   -1, &sqlStatement, nullptr));
        dbCheck(sqlite3_bind_int64(sqlStatement, 1, after));
        while (sqlite3_step(sqlStatement) != SQLITE_DONE) {
            auto artist = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 0)));
            auto album = QString::fromUtf8(reinterpret_cast<const char *>(sqlite3_column_text(sqlStatement, 1)));
            if ((!artist.isEmpty()) && (!album.isEmpty())) {
                if (mapArtistAlbum.find(artist) == mapArtistAlbum.end()) {
                    mapArtistAlbum[artist] = std::set<QString>();
                }
                mapArtistAlbum[artist].insert(album);
            }
        }
        dbCheck(sqlite3_finalize(sqlStatement));
    } catch (const std::runtime_error& e) {
        qCritical() << "Unable to query database for played artists and albums, error: " << e.what();
        mapArtistAlbum.clear();
    }
    return mapArtistAlbum;
}

void xPlayerDatabase::removeFromTable(const std::string& tableName, const std::string& whereArgument) {
    // Remove entries from given table.
    auto sqlRemove = "DELETE FROM " + tableName + whereArgument;
    if (sqlite3_exec(sqlDatabase, sqlRemove.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        qCritical() << "Unable to remove entries from " << QString::fromStdString(tableName)
                    << " table, error: " << sqlite3_errmsg(sqlDatabase);
        throw std::runtime_error(sqlite3_errmsg(sqlDatabase));
    }
}

