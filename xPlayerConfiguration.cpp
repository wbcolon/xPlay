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

#include <QRegularExpression>

// Configuration strings.
const QString xPlayerConfiguration_MusicLibraryDirectory { "xPlay/MusicLibraryDirectory" };
const QString xPlayerConfiguration_MusicLibraryExtensions { "xPlay/MusicLibraryExtensions" };
const QString xPlayerConfiguration_RotelNetworkAddress { "xPlay/RotelNetworkAddress" };
const QString xPlayerConfiguration_RotelNetworkPort { "xPlay/RotelNetworkPort" };
const QString xPlayerConfiguration_MovieLibraryDirectory { "xPlay/MovieLibraryDirectory" };
const QString xPlayerConfiguration_MovieLibraryExtensions { "xPlay/MovieLibraryExtensions" };

const QString xPlayerConfiguration_MusicLibraryExtensions_Default { ".flac .ogg .mp3" };
const QString xPlayerConfiguration_MovieLibraryExtensions_Default { ".mkv .mp4 .avi .mov .wmv" };

// singleton object.
xPlayerConfiguration* xPlayerConfiguration::playerConfiguration = nullptr;

xPlayerConfiguration::xPlayerConfiguration():
        QObject() {
    // Settings.
    settings = new QSettings(xPlayerConfiguration::OrganisationName, xPlayerConfiguration::ApplicationName, this);
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
        QString movieLibraryDirectory;
        if (tagDir.count() > 0) {
            movieLibraryDirectory = tagDir.at(0);
            for (int i = 1; i < tagDir.count(); ++i) {
                movieLibraryDirectory += QString("|") + tagDir.at(i);
            }
        }
        settings->setValue(xPlayerConfiguration_MovieLibraryDirectory, movieLibraryDirectory);
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

void xPlayerConfiguration::updatedConfiguration() {
    // Fire all update signals.
    emit updatedMusicLibraryDirectory();
    emit updatedMusicLibraryExtensions();
    emit updatedRotelNetworkAddress();
    emit updatedMovieLibraryTagsAndDirectories();
    emit updatedMovieLibraryExtensions();
}
