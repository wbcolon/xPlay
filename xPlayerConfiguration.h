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

#ifndef __XPLAYERCONFIGURATION_H__
#define __XPLAYERCONFIGURATION_H__

#include <QSettings>
#include <QString>
#include <filesystem>
#include <list>

class xPlayerConfiguration {

public:
    static const QString ApplicationName;
    static const QString OrganisationName;

    xPlayerConfiguration() = default;
    ~xPlayerConfiguration() = default;

    static void setMusicLibraryDirectory(const QString& directory);
    static void setMusicLibraryExtensions(const QString& extensions);
    static void setRotelNetworkAddress(const QString& address);
    static void setRotelNetworkPort(int port);
    static void setMovieLibraryTagAndDirectory(const QStringList& tagDir);
    static void setMovieLibraryExtensions(const QString& extensions);

    static QString getMusicLibraryDirectory();
    static std::filesystem::path getMusicLibraryDirectoryPath();
    static QString getMusicLibraryExtensions();
    static QStringList getMusicLibraryExtensionList();
    static QString getRotelNetworkAddress();
    static int getRotelNetworkPort();
    static QStringList getMovieLibraryTagAndDirectory();
    static std::list<std::pair<QString,std::filesystem::path>> getMovieLibraryTagAndDirectoryPath();
    static QString getMovieLibraryExtensions();

    static std::pair<QString,QString> splitMovieLibraryTagAndDirectory(const QString& tagDir);

private:
    static QSettings* settings;
};

#endif
