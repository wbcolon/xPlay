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

#include "xPlayerConfiguration.h"
#include "xPlayerConfig.h"

#include <QList>
#include <QUrl>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDebug>

// Configuration strings.
const QString xPlayerConfiguration_MusicLibraryDirectory { "xPlay/MusicLibraryDirectory" };
const QString xPlayerConfiguration_MusicLibraryExtensions { "xPlay/MusicLibraryExtensions" };
const QString xPlayerConfiguration_RotelWidget { "xPlay/RotelWidget" };
const QString xPlayerConfiguration_RotelNetworkAddress { "xPlay/RotelNetworkAddress" };
const QString xPlayerConfiguration_RotelNetworkPort { "xPlay/RotelNetworkPort" };
const QString xPlayerConfiguration_MovieLibraryDirectory { "xPlay/MovieLibraryDirectory" };
const QString xPlayerConfiguration_MovieLibraryExtensions { "xPlay/MovieLibraryExtensions" };
const QString xPlayerConfiguration_StreamingSites { "xPlay/StreamingSites" };
const QString xPlayerConfiguration_StreamingSitesDefault { "xPlay/StreamingSitesDefault" };

const QString xPlayerConfiguration_MusicLibraryExtensions_Default { ".flac .ogg .mp3" };
const QString xPlayerConfiguration_MovieLibraryExtensions_Default { ".mkv .mp4 .avi .mov .wmv" };

const QList<std::pair<QString,QUrl>> xPlayerConfiguration_StreamingDefaultSites = {
        { "qobuz", QUrl("https://play.qobuz.com/login") },
        { "youtube", QUrl("https://www.youtube.com") },
};


// singleton object.
xPlayerConfiguration* xPlayerConfiguration::playerConfiguration = nullptr;

xPlayerConfiguration::xPlayerConfiguration():
        QObject() {
    // Settings.
    settings = new QSettings(xPlayerConfiguration::OrganisationName, xPlayerConfiguration::ApplicationName, this);
    dataBasePath = QString("%1/%2/%3.db").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).
            arg(xPlayerConfiguration::OrganisationName).arg(xPlayerConfiguration::ApplicationName);
}

xPlayerConfiguration* xPlayerConfiguration::configuration() {
    // Create and return singleton.
    if (playerConfiguration == nullptr) {
        playerConfiguration = new xPlayerConfiguration();
    }
    return playerConfiguration;
}

void xPlayerConfiguration::setMusicLibraryDirectory(const QString& directory) {
    if (directory != getMusicLibraryDirectory()) {
        settings->setValue(xPlayerConfiguration_MusicLibraryDirectory, directory);
        settings->sync();
        emit updatedMusicLibraryDirectory();
    }
}

void xPlayerConfiguration::setMusicLibraryExtensions(const QString& extensions) {
    if (extensions != getMusicLibraryExtensions()) {
        settings->setValue(xPlayerConfiguration_MusicLibraryExtensions, extensions);
        settings->sync();
        emit updatedMovieLibraryExtensions();
    }
}

void xPlayerConfiguration::setRotelWidget(bool enable) {
    if (enable != rotelWidget()) {
        settings->setValue(xPlayerConfiguration_RotelWidget, enable);
        settings->sync();
        emit updatedRotelWidget();
        // Trigger the network connect if the widget is enabled again.
        if (enable) {
            emit updatedRotelNetworkAddress();
        }
    }
}

void xPlayerConfiguration::setRotelNetworkAddress(const QString& address, int port) {
    auto networkAddress = getRotelNetworkAddress();
    if ((networkAddress.first != address) || (networkAddress.second != port)) {
        settings->setValue(xPlayerConfiguration_RotelNetworkAddress, address);
        settings->setValue(xPlayerConfiguration_RotelNetworkPort, port);
        settings->sync();
        emit updatedRotelNetworkAddress();
    }
}

void xPlayerConfiguration::setMovieLibraryTagAndDirectory(const QStringList& tagDir) {
    if (tagDir != getMovieLibraryTagAndDirectory()) {
        settings->setValue(xPlayerConfiguration_MovieLibraryDirectory, tagDir.join(QChar('|')));
        settings->sync();
        emit updatedMovieLibraryTagsAndDirectories();
    }
}

void xPlayerConfiguration::setMovieLibraryExtensions(const QString& extensions) {
    if (extensions != getMovieLibraryExtensions()) {
        settings->setValue(xPlayerConfiguration_MovieLibraryExtensions, extensions);
        settings->sync();
        emit updatedMovieLibraryExtensions();
    }
}

void xPlayerConfiguration::setStreamingSites(const QList<std::pair<QString,QUrl>>& sites) {
    if ((sites != getStreamingSites()) || (sites == xPlayerConfiguration_StreamingDefaultSites)) {
        QString nameUrlString;
        if (!sites.isEmpty()) {
            nameUrlString = QString("(%1) - %2").arg(sites.at(0).first).arg(sites.at(0).second.toString());
            for (int i = 1; i < sites.size(); ++i) {
                nameUrlString += QString("|(%1) - %2").arg(sites.at(i).first).arg(sites.at(i).second.toString());
            }
        }
        settings->setValue(xPlayerConfiguration_StreamingSites, nameUrlString);
        settings->sync();
        emit updatedStreamingSites();
    }
}

void xPlayerConfiguration::setStreamingSitesDefault(const std::pair<QString,QUrl>& site) {
    if (site != getStreamingSitesDefault()) {
        auto nameUrlString = QString("(%1) - %2").arg(site.first).arg(site.second.toString());
        settings->setValue(xPlayerConfiguration_StreamingSitesDefault, nameUrlString);
        settings->sync();
        emit updatedStreamingSitesDefault();
    }
}

QString xPlayerConfiguration::getMusicLibraryDirectory() {
    return settings->value(xPlayerConfiguration_MusicLibraryDirectory, "").toString();
}

std::filesystem::path xPlayerConfiguration::getMusicLibraryDirectoryPath() {
    return std::filesystem::path(getMusicLibraryDirectory().toStdString());
}

QString xPlayerConfiguration::getMusicLibraryExtensions() {
    return settings->value(xPlayerConfiguration_MusicLibraryExtensions,
                           xPlayerConfiguration_MusicLibraryExtensions_Default).toString();
}

QStringList xPlayerConfiguration::getMusicLibraryExtensionList() {
    auto extensions = getMusicLibraryExtensions();
    if (extensions.isEmpty()) {
        return QStringList();
    } else {
        return extensions.split(" ");
    }
}

bool xPlayerConfiguration::rotelWidget() {
    return settings->value(xPlayerConfiguration_RotelWidget, true).toBool();
}

std::pair<QString,int> xPlayerConfiguration::getRotelNetworkAddress() {
    return std::make_pair(settings->value(xPlayerConfiguration_RotelNetworkAddress, "").toString(),
                          settings->value(xPlayerConfiguration_RotelNetworkPort, "").toInt());

}

QStringList xPlayerConfiguration::getMovieLibraryTagAndDirectory() {
    auto movieLibraryDirectory = settings->value(xPlayerConfiguration_MovieLibraryDirectory, "").toString();
    if (movieLibraryDirectory.isEmpty()) {
        return QStringList();
    } else {
        return movieLibraryDirectory.split("|");
    }
}

QList<std::pair<QString,QUrl>> xPlayerConfiguration::getStreamingSites() {
    auto streamingSites = settings->value(xPlayerConfiguration_StreamingSites, "").toString();
    QList<std::pair<QString,QUrl>> streamingList;
    if (streamingSites.isEmpty()) {
        // Nothing stored then return default sites.
        return xPlayerConfiguration_StreamingDefaultSites;
    } else {
        for (const auto& nameUrl : streamingSites.split("|")) {
            streamingList.push_back(splitStreamingShortNameAndUrl(nameUrl));
        }
        return streamingList;
    }
}

std::pair<QString,QUrl> xPlayerConfiguration::getStreamingSitesDefault() {
    auto streamingSitesDefault = settings->value(xPlayerConfiguration_StreamingSitesDefault, "").toString();
    if (streamingSitesDefault.isEmpty()) {
        // Nothing stored then return default sites.
        return std::make_pair("", QUrl(""));
    } else {
        return splitStreamingShortNameAndUrl(streamingSitesDefault);
    }
}

std::list<std::pair<QString,std::filesystem::path>> xPlayerConfiguration::getMovieLibraryTagAndDirectoryPath() {
    std::list<std::pair<QString,std::filesystem::path>> tagDirList;
    for  (const auto& entry : getMovieLibraryTagAndDirectory()) {
        auto splitEntry = xPlayerConfiguration::splitMovieLibraryTagAndDirectory(entry);
        tagDirList.emplace_back(splitEntry.first, std::filesystem::path(splitEntry.second.toStdString()));
    }
    return tagDirList;
}

QString xPlayerConfiguration::getMovieLibraryExtensions() {
    return settings->value(xPlayerConfiguration_MovieLibraryExtensions,
                           xPlayerConfiguration_MovieLibraryExtensions_Default).toString();
}

QStringList xPlayerConfiguration::getMovieLibraryExtensionList() {
    auto extensions = getMovieLibraryExtensions();
    if (extensions.isEmpty()) {
        return QStringList();
    } else {
        return extensions.split(" ");
    }
}

std::pair<QString,QString> xPlayerConfiguration::splitMovieLibraryTagAndDirectory(const QString& tagDir) {
    QRegularExpression regExp("\\((?<tag>.*)\\) - (?<directory>.*)");
    QRegularExpressionMatch regExpMatch = regExp.match(tagDir);
    if (regExpMatch.hasMatch()) {
        return std::make_pair(regExpMatch.captured("tag"), regExpMatch.captured("directory"));
    } else {
        return std::make_pair("","");
    }
}

std::pair<QString,QUrl> xPlayerConfiguration::splitStreamingShortNameAndUrl(const QString& nameUrl) {
    QRegularExpression regExp("\\((?<name>.*)\\) - (?<url>.*)");
    QRegularExpressionMatch regExpMatch = regExp.match(nameUrl);
    if (regExpMatch.hasMatch()) {
        return std::make_pair(regExpMatch.captured("name"), QUrl(regExpMatch.captured("url")));
    } else {
        return std::make_pair("",QUrl(""));
    }
}

QString xPlayerConfiguration::getDatabasePath() {
    return dataBasePath;
}

void xPlayerConfiguration::updatedConfiguration() {
    // Fire all update signals.
    emit updatedMusicLibraryDirectory();
    emit updatedMusicLibraryExtensions();
    emit updatedRotelNetworkAddress();
    emit updatedMovieLibraryTagsAndDirectories();
    emit updatedMovieLibraryExtensions();
    emit updatedStreamingSites();
    emit updatedStreamingSitesDefault();
}
