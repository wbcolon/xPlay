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

class xPlayerConfiguration:public QObject {
    Q_OBJECT

public:
    /**
     * Return the Application Configuration.
     *
     * @return pointer to a singleton of the configuration.
     */
    static xPlayerConfiguration* configuration();
    /**
     * Set the base path to the music library.
     *
     * @param directory the absolute path as string.
     */
    void setMusicLibraryDirectory(const QString& directory);
    /**
     * Set the set of accepted extensions for music files.
     *
     * @param extensions a space separated list of extensions.
     */
    void setMusicLibraryExtensions(const QString& extensions);
    /**
     * Set the network address and port used to connect the Rotel amp.
     *
     * @param address the IP address of dns name as string.
     * @param port the port number as integer.
     */
    void setRotelNetworkAddress(const QString& address, int port);
    /**
     * Set a list of tag and directories.
     *
     * @param tagDir list of strings each containing the tag and corresponding directory.
     */
    void setMovieLibraryTagAndDirectory(const QStringList& tagDir);
    /**
     * Set the set of accepted extensions for movie files.
     *
     * @param extensions a space separated list of extensions.
     */
    void setMovieLibraryExtensions(const QString& extensions);
    /**
     * Get the base directory of the music library.
     *
     * @return the music library directory as string.
     */
    QString getMusicLibraryDirectory();
    /**
     * Get the base directory of the music library.
     *
     * @return the music library directory as filesystem::path.
     */
    std::filesystem::path getMusicLibraryDirectoryPath();
    /**
     * Get the list of accepted extensions for music files.
     *
     * @return the list of extensions as space separated string.
     */
    QString getMusicLibraryExtensions();
    /**
     * Get the list of accepted extensions for music files.
     *
     * @return the list of extensions.
     */
    QStringList getMusicLibraryExtensionList();
    /**
     * Get the network configuration for the Rotel amp.
     *
     * @return the pair of network address and port.
     */
    std::pair<QString,int> getRotelNetworkAddress();
    /**
     * Get the list of tag and directory strings.
     *
     * @return the list of strings containing the tag and directory.
     */
    QStringList getMovieLibraryTagAndDirectory();
    /**
     * Get the list of tag and directory paths.
     *
     * @return the list of pairs of tag and corresponding directory as filesystem::path.
     */
    std::list<std::pair<QString,std::filesystem::path>> getMovieLibraryTagAndDirectoryPath();
    /**
     * Get the list of accepted extensions for movie files.
     *
     * @return the list of extensions as space separated string.
     */
    QString getMovieLibraryExtensions();
    /**
     * Get the list of accepted extensions for movie files.
     *
     * @return the list of extensions.
     */
    QStringList getMovieLibraryExtensionList();

    /**
     * Trigger all update configuration signals.
     *
     * Useful on application start, when the entire configuration is initially read.
     */
    void updatedConfiguration();

signals:
    /**
     * Signal an update of the music library directory.
     */
    void updatedMusicLibraryDirectory();
    /**
     * Signal an update of the accepted music file extensions.
     */
    void updatedMusicLibraryExtensions();
    /**
     * Signal an update of the movie library directory.
     */
    void updatedMovieLibraryTagsAndDirectories();
    /**
     * Signal an update of the accepted movie file extensions.
     */
    void updatedMovieLibraryExtensions();
    /**
     * Signal an update of the Rotel amp configuration.
     */
    void updatedRotelNetworkAddress();

private:
    xPlayerConfiguration();
    ~xPlayerConfiguration() = default;

    static std::pair<QString,QString> splitMovieLibraryTagAndDirectory(const QString& tagDir);

    const QString ApplicationName = "xPlay";
    const QString OrganisationName = "wbcolon";

    static xPlayerConfiguration* playerConfiguration;
    QSettings* settings;
};

#endif
