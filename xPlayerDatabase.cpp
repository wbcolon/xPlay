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
        // Create music table.
        sqlDatabase << "CREATE TABLE music (hash VARCHAR PRIMARY KEY, playCount INT, timeStamp BIGINT, "
                       "artist VARCHAR, album VARCHAR, track VARCHAR, sampleRate INT, bitsPerSample INT)";
        // Create movie table.
        sqlDatabase << "CREATE TABLE movie (hash VARCHAR PRIMARY KEY, playCount INT, timeStamp BIGINT, "
                       "tag VARCHAR, directory VARCHAR, movie VARCHAR)";
    } catch (soci::soci_error& e) {
        // Ignore error.
        qInfo() << "xPlay database already exists. Ignoring error.";
    }
}

xPlayerDatabase* xPlayerDatabase::database() {
    // Create and return singleton.
    if (playerDatabase == nullptr) {
        playerDatabase = new xPlayerDatabase();
    }
    return playerDatabase;
}

void xPlayerDatabase::updateMusicFile(const QString& artist, const QString& album, const QString& track, int sampleRate, int bitsPerSample) {
    auto hash = QCryptographicHash::hash((artist+"/"+album+"/"+track).toUtf8(), QCryptographicHash::Sha256).toBase64().toStdString();
    auto timeStamp = QDateTime::currentMSecsSinceEpoch();
    try {
        int playCount = -1;
        soci::indicator playCountIndicator;
        sqlDatabase << "SELECT playCount FROM music WHERE hash=:hash", soci::into(playCount, playCountIndicator), soci::use(hash);
        if (playCountIndicator == soci::i_ok) {
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
        if (playCountIndicator == soci::i_ok) {
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

#if 0
// TODO REMOVE
// Example query
soci::rowset<soci::row> movieRowSet = (sqlDatabase.prepare << "SELECT * FROM movie WHERE hash=:hash", soci::use(hash));
        for (const auto& movieRow : movieRowSet) {
            // Exit after the update since there can only be one result.
            auto playCount = movieRow.get<int>(1)+1;
            sqlDatabase << "UPDATE movie SET playCount=:playCount,timeStamp=:timeStamp WHERE hash=:hash",
                    soci::use(playCount), soci::use(timeStamp), soci::use(hash);
            qDebug() << "xPlayerDatabase: update: " << tag+"/"+directory+"/"+movie << QString("(%1)").arg(playCount);
            return;
        }

#endif