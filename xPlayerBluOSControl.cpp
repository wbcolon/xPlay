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

#include "xPlayerBluOSControl.h"

#include <QThread>
#include <QFileInfo>
#include <QDebug>

#include <iostream>


xPlayerBluOSControls* xPlayerBluOSControls::bluOSControls = nullptr;

xPlayerBluOSControls::xPlayerBluOSControls():
        QObject() {
    bluOSRequests = curl_easy_init();
    bluOSStatus = new QTimer(this);
    bluOSReIndexing = new QTimer(this);
    QObject::connect(bluOSStatus, &QTimer::timeout, [=]() {
        parsePlayerStatus(sendCommand(QUrl(bluOSUrl+"/Status")));
    });
    QObject::connect(bluOSReIndexing, &QTimer::timeout, [=]() {
        parseIndexing(sendCommand(QUrl(bluOSUrl+"/Status")));
    });
    bluOSTrackInfoRegExpr = new QRegularExpression("sample.*rate.*>(?<samplerate>\\d+).*\n.*sample.*size.*>(?<bitspersample>\\d+)");
}

xPlayerBluOSControls::~xPlayerBluOSControls() {
    curl_easy_cleanup(bluOSRequests);
    if (bluOSStatus) {
        bluOSStatus->stop();
    }
    delete bluOSTrackInfoRegExpr;
}

xPlayerBluOSControls* xPlayerBluOSControls::controls() {
    // Create and return singleton controls.
    if (bluOSControls == nullptr) {
        bluOSControls = new xPlayerBluOSControls;
    }
    return bluOSControls;
}

void xPlayerBluOSControls::reIndex() {
    // Stop the player.
    stop();
    // Force the reindexing.
    sendCommand(QUrl(bluOSUrl+"/Reindex"));
    if (!bluOSReIndexing->isActive()) {
        bluOSReIndexing->start(2000);
    }
}

void xPlayerBluOSControls::play() {
    sendCommand(QUrl(bluOSUrl+"/Play"));
    if (!bluOSStatus->isActive()) {
        bluOSStatus->start(1000);
    }
}

void xPlayerBluOSControls::play(int index) {
    sendCommand(QUrl(bluOSUrl+QString("/Play?id=%1").arg(index)));
    if (!bluOSStatus->isActive()) {
        bluOSStatus->start(1000);
    }
}

void xPlayerBluOSControls::pause() {
    sendCommand(QUrl(bluOSUrl+"/Pause"));
    bluOSStatus->stop();
}

void xPlayerBluOSControls::stop() {
    sendCommand(QUrl(bluOSUrl+"/Stop"));
    bluOSStatus->stop();
}

void xPlayerBluOSControls::seek(qint64 position) {
    sendCommand(QUrl(bluOSUrl+QString("/Play?seek=%1").arg(position/1000)));
    // Seek will start playing the current track.
    if (!bluOSStatus->isActive()) {
        bluOSStatus->start(1000);
    }
}

void xPlayerBluOSControls::prev() {
    sendCommand(QUrl(bluOSUrl+"/Back"));
    if (!bluOSStatus->isActive()) {
        bluOSStatus->start(1000);
    }
}

void xPlayerBluOSControls::next() {
    sendCommand(QUrl(bluOSUrl+"/Skip"));
    if (!bluOSStatus->isActive()) {
        bluOSStatus->start(1000);
    }
}

QString xPlayerBluOSControls::state() {
    return parseState(sendCommand(QUrl(bluOSUrl+"/Status")));
}

void xPlayerBluOSControls::addQueue(const QString& path) {
    sendCommand(QUrl(bluOSUrl+"/Add?file="+path));
}

void xPlayerBluOSControls::removeQueue(int index) {
    sendCommand(QUrl(bluOSUrl+QString("/Delete?id=%1").arg(index)));
}

QStringList xPlayerBluOSControls::queue() {
    return parsePlaylist(sendCommand(QUrl(bluOSUrl+"/Playlist")));
}

void xPlayerBluOSControls::clearQueue() {
    sendCommand(QUrl(bluOSUrl+"/Clear"));
}

void xPlayerBluOSControls::setVolume(int vol) {
    sendCommand(QUrl(bluOSUrl+QString("/Volume?level=%1").arg(vol)));
    emit volume(vol);
}

int xPlayerBluOSControls::getVolume() {
    return parseVolume(sendCommand(QUrl(bluOSUrl+"/Volume")));
}

void xPlayerBluOSControls::setMuted(bool mute) {
    sendCommand(QUrl(bluOSUrl+QString("/Volume?mute=%1").arg(static_cast<int>(mute))));
    emit muted(mute);
}

bool xPlayerBluOSControls::isMuted() {
    return parseMute(sendCommand(QUrl(bluOSUrl+QString("/Status"))));
}

void xPlayerBluOSControls::setShuffle(bool shuffle) {
    sendCommand(QUrl(bluOSUrl+QString("/Shuffle?state=%1").arg(static_cast<int>(shuffle))));
}

bool xPlayerBluOSControls::isShuffle() {
    return parseShuffle(sendCommand(QUrl(bluOSUrl+QString("/Status"))));
}

std::vector<std::tuple<QUrl,QString>> xPlayerBluOSControls::getArtists() {
    std::vector<std::tuple<QUrl,QString>> artists;
    auto artistsPath = bluOSUrl+"/Folders?&service=LocalMusic&path="+bluOSBasePath;
    qDebug() << "xPlayerBluOSControls::getArtists: " << artistsPath;
    auto artistNames = parseFolders(sendCommand(artistsPath));
    for (const auto& artistName : artistNames) {
        artists.emplace_back(QUrl(artistsPath+"/"+artistName), artistName);
    }
    return artists;
}

std::vector<std::tuple<QUrl,QString>> xPlayerBluOSControls::getAlbums(const QString& artist) {
    std::vector<std::tuple<QUrl,QString>> albums;
    auto albumsPath = bluOSUrl+"/Folders?&service=LocalMusic&path="+bluOSBasePath+"/"+artist;
    qDebug() << "xPlayerBluOSControls::getAlbums: " << albumsPath;
    auto albumNames = parseFolders(sendCommand(albumsPath));
    for (const auto& albumName : albumNames) {
        albums.emplace_back(QUrl(albumsPath+"/"+albumName), albumName);
    }
    return albums;
}

std::vector<std::tuple<QUrl,QString,qint64,QString>> xPlayerBluOSControls::getTracks(const QString& artist, const QString& album) {
    std::vector<std::tuple<QUrl,QString,qint64,QString>> tracks;
    auto tracksPath = bluOSUrl+"/Folders?&service=LocalMusic&path="+bluOSBasePath+"/"+artist+"/"+album;
    qDebug() << "xPlayerBluOSControls::getTracks: " << tracksPath;
    auto trackInfos = parseTracks(sendCommand(tracksPath));
    for (const auto& trackInfo : trackInfos) {
        auto trackName = std::get<0>(trackInfo);
        tracks.emplace_back(QUrl(tracksPath+"/"+trackName), trackName, std::get<1>(trackInfo), std::get<2>(trackInfo));
    }
    return tracks;
}

std::tuple<int,int> xPlayerBluOSControls::getTrackInfo(const QString& path) {
    auto trackId = parsePlaylistTrackId(sendCommand(QUrl(bluOSUrl+"/Playlist")), path);
    auto trackInfoPath = bluOSUrl + QString("/Info?service=LocalMusic&category=technical&songid=%1&service=library").arg(trackId);
    return parseTrackInfo(sendCommand(trackInfoPath));
}

// Helper function for sendCommand. Callback that stores the result into a string.
static size_t CurlWriteMemoryCallback(void* contents, size_t mSize, size_t nMembers, void* userPointer) {
  auto response = static_cast<QString*>(userPointer);
  size_t realSize = mSize * nMembers;
  response->append(QString::fromStdString(std::string(static_cast<char*>(contents), realSize)));
  return realSize;
}

QString xPlayerBluOSControls::sendCommand(const QUrl& url) {
    if (bluOSUrl.isEmpty()) {
        qWarning() << "xPlayerBluOSControls::sendCommand: not connected, ignoring command: " << url;
        return {};
    }
    QString curlResponse;
    bluOSMutex.lock();
    qDebug() << "xPlayerBluOSControls::sendCommand: url: " << url.toEncoded();
    if (bluOSRequests) {
        curl_easy_setopt(bluOSRequests, CURLOPT_URL, url.toEncoded().toStdString().c_str());
        /* send all data to this function  */
        curl_easy_setopt(bluOSRequests, CURLOPT_WRITEFUNCTION, CurlWriteMemoryCallback);
        /* we pass a string to the callback function */
        curl_easy_setopt(bluOSRequests, CURLOPT_WRITEDATA, (void *)&curlResponse);
        if (curl_easy_perform(bluOSRequests) != CURLE_OK) {
            bluOSMutex.unlock();
            return {};
        }
    }
    qDebug() << "xPlayerBluOSControls::sendCommand: " << curlResponse;
    bluOSMutex.unlock();
    return curlResponse;
}

QString xPlayerBluOSControls::parseBasePath(const QString& commandResult) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        return QString::fromStdString(bluOSResponse.child("folders").child("subfolders").child("folder").child_value());
    } else {
        qCritical() << "Unable to parse result for base path: " << result.description();
    }
    return {};
}

QString xPlayerBluOSControls::parseState(const QString& commandResult) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        return QString::fromStdString(bluOSResponse.child("status").child("state").child_value());
    } else {
        qCritical() << "Unable to parse result for state: " << result.description();
    }
    return {};
}

int xPlayerBluOSControls::parseVolume(const QString &commandResult) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        return QString::fromStdString(bluOSResponse.child("volume").child_value()).toInt();
    } else {
        qCritical() << "Unable to parse result for volume: " << result.description();
    }
    return -1;
}

bool xPlayerBluOSControls::parseMute(const QString& commandResult) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        return static_cast<bool>(QString::fromStdString(bluOSResponse.child("status").child("mute").child_value()).toInt());
    } else {
        qCritical() << "Unable to parse result for state(mute): " << result.description();
    }
    return false;
}

bool xPlayerBluOSControls::parseShuffle(const QString& commandResult) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        return static_cast<bool>(QString::fromStdString(bluOSResponse.child("status").child("shuffle").child_value()).toInt());
    } else {
        qCritical() << "Unable to parse result for state(shuffle): " << result.description();
    }
    return false;
}

int xPlayerBluOSControls::parsePlaylistTrackId(const QString &commandResult, const QString& path) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        // Parse through all elements. Find the track ID.
        for (auto song : bluOSResponse.child("playlist").children()) {
            if (path == song.child("fn").child_value()) {
                return QString(song.attribute("songid").value()).toInt();
            }
        }
    } else {
        qCritical() << "Unable to parse result for playlist: " << result.description();
    }
    return -1;
}

int xPlayerBluOSControls::parseIndexing(const QString &commandResult) {
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        auto noTracks = QString::fromStdString(bluOSResponse.child("status").child("indexing").child_value()).toInt();
        emit playerReIndexing(noTracks);
        if (!noTracks) {
            bluOSReIndexing->stop();
        }
    } else {
        qCritical() << "Unable to parse result for state(shuffle): " << result.description();
    }
    return 0;
}

void xPlayerBluOSControls::parsePlayerStatus(const QString &commandResult) {
    pugi::xml_parse_result result = bluOSStatusResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        auto status = bluOSStatusResponse.child("status");
        emit playerStatus(status.child("fn").child_value(),
                          QString::fromStdString(status.child("secs").child_value()).toInt()*1000,
                          status.child("quality").child_value());
        // Try to detect stop at the end of the queue.
        if (status.child("quality").empty() && status.child("canSeek").empty()) {
            emit playerStopped();
        }
    } else {
        qCritical() << "Unable to parse result for state: " << result.description();
    }
}

std::vector<QString> xPlayerBluOSControls::parseFolders(const QString& commandResult) {
    std::vector<QString> folders;
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        // Parse through all subfolders.
        for (auto subfolder : bluOSResponse.child("folders").child("subfolders").children()) {
            folders.emplace_back(subfolder.child_value());
        }
    } else {
        qCritical() << "Unable to parse result for folders: " << result.description();
    }
    return folders;
}

std::vector<std::tuple<QString,qint64,QString>> xPlayerBluOSControls::parseTracks(const QString& commandResult) {
    std::vector<std::tuple<QString,qint64,QString>> tracks;
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        // Parse through all subfolders.
        for (auto song : bluOSResponse.child("folders").child("songs").children()) {
            QString songTime(song.child("time").child_value());
            QFileInfo songInfo(song.child("fn").child_value());
            tracks.emplace_back(songInfo.fileName(), songTime.toInt()*1000, song.child("fn").child_value());
        }
    } else {
        qCritical() << "Unable to parse result for tracks: " << result.description();
    }
    return tracks;
}

std::tuple<int,int> xPlayerBluOSControls::parseTrackInfo(const QString& commandResult) {
    // The result is a mini HTTP page. Use simple regular expression for parsing.
    QRegularExpressionMatch match;
    match = bluOSTrackInfoRegExpr->match(commandResult.toLower());
    if (match.hasMatch()) {
        return std::make_tuple(match.captured("samplerate").toInt(), match.captured("bitspersample").toInt());
    } else {
        return std::make_tuple(-1, -1);
    }
}

QStringList xPlayerBluOSControls::parsePlaylist(const QString& commandResult) {
    QStringList queue;
    pugi::xml_parse_result result = bluOSResponse.load_string(commandResult.toStdString().c_str());
    if (result) {
        // Parse through all subfolders.
        for (auto song : bluOSResponse.child("playlist").children()) {
            queue.push_back(song.child("fn").child_value());
        }
    } else {
        qCritical() << "Unable to parse result for tracks: " << result.description();
    }
    return queue;
}

void xPlayerBluOSControls::connect(const QUrl& url) {
    bluOSUrl = url.toEncoded();
    bluOSBasePath = parseBasePath(sendCommand(QUrl(bluOSUrl+"/Folders?&service=LocalMusic")));
    qDebug() << "xPlayerBluOSControls::connect: " << bluOSUrl << "," << bluOSBasePath;
    // Disable repeat of the queue.
    sendCommand(QUrl(bluOSUrl+"/Repeat?&state=2"));
}

void xPlayerBluOSControls::disconnect() {
    // Clear queue also stops the player.
    clearQueue();
    bluOSUrl.clear();
    bluOSBasePath.clear();
}

