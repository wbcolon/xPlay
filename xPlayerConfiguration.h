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
#include <QList>
#include <QUrl>
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
    [[nodiscard]] static xPlayerConfiguration* configuration();
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
     * Set availability of the Rotel amp widget.
     *
     * @param enabled show Rotel widget after restart if true, disable otherwise.
     */
    void setRotelWidget(bool enabled);
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
     * Set the list of sites available in the streaming view.
     *
     * @param sites list of pairs of short name and URL.
     */
    void setStreamingSites(const QList<std::pair<QString,QUrl>>& sites);
    /**
     * Set the default streaming site for the streaming view.
     *
     * @param site a pairs of short name and URL.
     */
    void setStreamingSitesDefault(const std::pair<QString,QUrl>& site);
    /**
     * Set the cut-off time stamp used in database queries.
     *
     * @param cutOff the time stamp if ms since epoch.
     */
    void setDatabaseCutOff(quint64 cutOff);
    /**
     * Set the mode for the database overlay for the music view.
     *
     * @param enabled if true then enable overlay, disable otherwise.
     */
    void setDatabaseMusicOverlay(bool enabled);
    /**
     * Set the mode for the database overlay for the movie view.
     *
     * @param enabled if true then enable overlay, disable otherwise.
     */
    void setDatabaseMovieOverlay(bool enabled);
    /**
     * Get the base directory of the music library.
     *
     * @return the music library directory as string.
     */
    [[nodiscard]] QString getMusicLibraryDirectory();
    /**
     * Get the base directory of the music library.
     *
     * @return the music library directory as filesystem::path.
     */
    [[nodiscard]] std::filesystem::path getMusicLibraryDirectoryPath();
    /**
     * Get the list of accepted extensions for music files.
     *
     * @return the list of extensions as space separated string.
     */
    [[nodiscard]] QString getMusicLibraryExtensions();
    /**
     * Get the list of accepted extensions for music files.
     *
     * @return the list of extensions.
     */
    [[nodiscard]] QStringList getMusicLibraryExtensionList();
    /**
     * Return the availability of the Rotel amp widget.
     *
     * @return true if the Rotel amp widget is displayed, false otherwise.
     */
    [[nodiscard]] bool rotelWidget();
    /**
     * Get the network configuration for the Rotel amp.
     *
     * @return the pair of network address and port.
     */
    [[nodiscard]] std::pair<QString,int> getRotelNetworkAddress();
    /**
     * Get the list of tag and directory strings.
     *
     * @return the list of strings containing the tag and directory.
     */
    [[nodiscard]] QStringList getMovieLibraryTagAndDirectory();
    /**
     * Get the list of tag and directory paths.
     *
     * @return the list of pairs of tag and corresponding directory as filesystem::path.
     */
    [[nodiscard]] std::list<std::pair<QString,std::filesystem::path>> getMovieLibraryTagAndDirectoryPath();
    /**
     * Get the list of accepted extensions for movie files.
     *
     * @return the list of extensions as space separated string.
     */
    [[nodiscard]] QString getMovieLibraryExtensions();
    /**
     * Get the list of accepted extensions for movie files.
     *
     * @return the list of extensions.
     */
    [[nodiscard]] QStringList getMovieLibraryExtensionList();
    /**
     * Get the list of streaming sites.
     *
     * @return list of pair of short name and URL.
     */
    [[nodiscard]] QList<std::pair<QString,QUrl>> getStreamingSites();
    /**
     * Get the default streaming site.
     *
     * @return a pair of short name and URL.
     */
    [[nodiscard]] std::pair<QString,QUrl> getStreamingSitesDefault();
    /**
     * Get the path to the database file.
     *
     * @return the absolute path as string.
     */
    [[nodiscard]] QString getDatabasePath();
    /**
     * Get the time stamp used as cut-off in the database queries.
     *
     * @return the cut-off time stamp in ms since epoch.
     */
    [[nodiscard]] quint64 getDatabaseCutOff();
    /**
     * Get the mode for the database overlay for the music view.
     *
     * @return true, if the overlay is enabled, false otherwise.
     */
    [[nodiscard]] bool getDatabaseMusicOverlay();
    /**
     * Get the mode for the database overlay for the movie view.
     *
     * @return true, if the overlay is enabled, false otherwise.
     */
    [[nodiscard]] bool getDatabaseMovieOverlay();
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
     * Signal an update of the visibility of the Rotel amp widget.
     */
    void updatedRotelWidget();
    /**
     * Signal an update of the Rotel amp configuration.
     */
    void updatedRotelNetworkAddress();
    /**
     * Signal an update of the list of streaming sites.
     */
    void updatedStreamingSites();
    /**
     * Signal an update of the default streaming site.
     */
    void updatedStreamingSitesDefault();
    /**
     * Signal an update of the database overlay for the music view.
     */
    void updatedDatabaseMusicOverlay();
    /**
     * Signal an update of the database overlay for the movie view.
     */
    void updatedDatabaseMovieOverlay();

private:
    xPlayerConfiguration();
    ~xPlayerConfiguration() override = default;
    /**
     * Helper function that splits item string into pair of tag and directory.
     *
     * @param tagDir the string stored in the list widget item.
     * @return the pair of extracted tag and directory.
     */
    [[nodiscard]] static std::pair<QString,QString> splitMovieLibraryTagAndDirectory(const QString& tagDir);
    /**
     *  Helper function that splits item string into pair of name and URL.
     *
     * @param nameUrl the string stored in the list widget item.
     * @return the pair of extracted name and URL.
     */
    [[nodiscard]] static std::pair<QString,QUrl> splitStreamingShortNameAndUrl(const QString& nameUrl);

    // Use different application names for debug/release in order to have separate settings and databases.
    const QString ApplicationName = XPLAY_APP_NAME;
    const QString OrganisationName = "wbcolon";

    static xPlayerConfiguration* playerConfiguration;
    QSettings* settings;
    QString dataBasePath;
};

#endif
