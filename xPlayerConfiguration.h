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
     * Set the url to the BluOS player library.
     *
     * @param url the url as string.
     */
    void setMusicLibraryBluOS(const QString& url);
    /**
     * Enable or disable the use of the BluOS player library.
     *
     * @param enabled use BluOS library if true, local library otherwise.
     */
    void useMusicLibraryBluOS(bool enabled);
    /**
     * Set the set of accepted extensions for music files.
     *
     * @param extensions a space separated list of extensions.
     */
    void setMusicLibraryExtensions(const QString& extensions);
    /**
     * Set the set of album selectors for filtering albums.
     *
     * @param selectors a space separated list of album selectors.
     */
    void setMusicLibraryAlbumSelectors(const QString& selectors);
    /**
     * Set the list of tags for music files.
     *
     * @param tags list of tags as strings.
     */
    void setMusicLibraryTags(const QStringList& tags);
    /**
     * Enable or disable the use of lltag for music library tagging.
     *
     * Main difference between lltag and taglib is the handling of track
     * numbers. lltag allows the use of strings for track numbers instead
     * of integers.
     *
     * @param enabled use lltag if true, taglib otherwise.
     */
    void useLLTag(bool enabled);
    /**
     * Set the path to the lltag binary.
     *
     * @param lltag the path to the binary as string.
     */
    void setLLTag(const QString& lltag);
    /**
     * Set the visibility status of the selectors widget.
     *
     * @param visible show selectors if true, hide otherwise.
     */
    void setMusicViewSelectors(bool visible);
    /**
     * Set the visibility status of the filters widget.
     *
     * @param visible show filters if true, hide otherwise.
     */
    void setMusicViewFilters(bool visible);
    /**
     * Set the visibility status of the music visualization.
     *
     * @param visible show the music visualization of true, hide otherwise.
     */
    void setMusicViewVisualization(bool visible);
    /**
     * Set the music visualization mode.
     * - 0: small window
     * - 1: central window
     *
     * @param mode the music visualization mode as integer.
     */
    void setMusicViewVisualizationMode(int mode);
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
     * Set the default language for movie audio channels.
     *
     * @param language the language as string. No default if empty.
     */
    void setMovieDefaultAudioLanguage(const QString& language);
    /**
     * Set the default language for movie subtitles.
     *
     * @param language the language as string. Disable if empty.
     */
    void setMovieDefaultSubtitleLanguage(const QString& language);
    /**
     * Set the mode of the audio compression for movies.
     *
     * @param enabled use audio compression plugin if true, do not use otherwise.
     */
    void setMovieAudioCompression(bool enabled);
    /**
     * Set the visibility status of the movie filters widget.
     *
     * @param visible show movie filters if true, hide otherwise.
     */
    void setMovieViewFilters(bool visible);
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
     * Set the visibility status of the sidebar widget.
     *
     * @param visible show sidebar if true, hide otherwise.
     */
    void setStreamingViewSidebar(bool visible);
    /**
     * Set the visibility status of the navigation bar widget.
     *
     * @param visible show navigation bar if true, hide otherwise.
     */
    void setStreamingViewNavigation(bool visible);
    /**
     * Set the directory to the database file.
     *
     * @param dir the directory as string.
     */
    void setDatabaseDirectory(const QString& dir);
    /**
     * Set the cut-off time stamp used in database queries.
     *
     * @param cutOff the time stamp if ms since epoch.
     */
    void setDatabaseCutOff(qint64 cutOff);
    /**
     * Set the mode for the database overlay for the music view.
     *
     * @param enabled if true then enable overlay, disable otherwise.
     */
    void setDatabaseMusicOverlay(bool enabled);
    /**
     * Set the amount of time when music is considered played.
     *
     * @param played the amount of time in ms.
     */
    void setDatabaseMusicPlayed(qint64 played);
    /**
     * Set the mode for the database overlay for the movie view.
     *
     * @param enabled if true then enable overlay, disable otherwise.
     */
    void setDatabaseMovieOverlay(bool enabled);
    /**
     * Set the amount of time when movie is considered played.
     *
     * @param played the amount of time in ms.
     */
    void setDatabaseMoviePlayed(qint64 played);
    /**
     * Set the mode for handling database update errors.
     *
     * The mode will not be stored as part of the permanent settings.
     * It will always default to false on the program startup.
     *
     * @param enabled if true then ignore errors, display otherwise.
     */
    void setDatabaseIgnoreUpdateErrors(bool enabled);
    /**
     * Set the path where to find the configuration for projectM visualization.
     *
     * @param path absolute path as string.
     */
    void setVisualizationConfigPath(const QString& path);
    /**
     * Set the name of the projectM visualization preset.
     *
     * @param preset the name of the preset as string.
     */
    void setVisualizationPreset(const QString& preset);
    /**
     * Set the index for the zoom factor for websites.
     *
     * @param index the new index for the zoom factor as integer.
     */
    void setWebsiteZoomFactorIndex(int index);
    /**
     * Set the user agent used for websites (browser identification)
     *
     * @param userAgent the user agent as string.
     */
    void setWebsiteUserAgent(const QString& userAgent);
    /**
     * Get the base directory of the music library.
     *
     * @return the music library directory as string.
     */
    [[nodiscard]] QString getMusicLibraryDirectory();
    /**
     * Get the url of the BluOS player library.
     *
     * @return the url as string.
     */
    [[nodiscard]] QString getMusicLibraryBluOS();
    /**
     * Get which music library to use.
     *
     * @return true if BluOS player library is enabled, false otherwise.
     */
    [[nodiscard]] bool useMusicLibraryBluOS();
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
     * Get the list of album selectors for filtering albums.
     *
     * @return the list of selectors as space separated string.
     */
    [[nodiscard]] QString getMusicLibraryAlbumSelectors();
    /**
     * Get the list of album selectors for filtering albums.
     *
     * @return the list of selectors.
     */
    [[nodiscard]] QStringList getMusicLibraryAlbumSelectorList();
    /**
     * Get the list of tags for the tracks.
     *
     * @return the list of tags as strings.
     */
    [[nodiscard]] QStringList getMusicLibraryTags();
    /**
     * Get tagging mode for music library.
     *
     * @return true if lltag is enabled, false otherwise.
     */
    [[nodiscard]] bool useLLTag();
    /**
     * Get the path to the lltag binary.
     *
     * @return the path to the lltag binary as string.
     */
    [[nodiscard]] QString getLLTag();
    /**
     * Get the visibility of the selectors.
     *
     * @return true if the selectors are visible, false otherwise.
     */
    [[nodiscard]] bool getMusicViewSelectors();
    /**
     * Get the visibility of the filters.
     *
     * @return true if the filters are visible, false otherwise.
     */
    [[nodiscard]] bool getMusicViewFilters();
    /**
     * Get the visibility of the music visualization.
     *
     * @return true if the music visualization is visible, false otherwise.
     */
    [[nodiscard]] bool getMusicViewVisualization();
    /**
     * Get the music visualization mode.
     * - 0: small window
     * - 1: central window
     *
     * @param mode the music visualization mode as integer.
     */
    int getMusicViewVisualizationMode();
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
     * Get the default language for movie audio channels.
     *
     * @return the language as string, empty if movie default.
     */
    [[nodiscard]] QString getMovieDefaultAudioLanguage();
    /**
     * Get the default subtitle language for movie subtitles.
     *
     * @return the language as string, empty if disabled.
     */
    [[nodiscard]] QString getMovieDefaultSubtitleLanguage();
    /**
     * Get the list of languages used for default audio channel and subtitles.
     *
     * @return the languages as string.
     */
    [[nodiscard]] static const QStringList& getMovieDefaultLanguages();
    /**
     * Get the mode of the audio compression for movies.
     *
     * @return true if audio compression for movies is used, false otherwise.
     */
    [[nodiscard]] bool getMovieAudioCompression();
    /**
     * Get the visibility of the movie filters.
     *
     * @return true if the movie filters are visible, false otherwise.
     */
    [[nodiscard]] bool getMovieViewFilters();
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
     * Get the visibility of the sidebar.
     *
     * @return true if the sidebar is visible, false otherwise.
     */
    [[nodiscard]] bool getStreamingViewSidebar();
    /**
     * Get the visibility of the navigation bar.
     *
     * @return true if the navigation bar is visible, false otherwise.
     */
    [[nodiscard]] bool getStreamingViewNavigation();
    /**
     * Get the directory to the database file.
     *
     * @return the absolute path as string.
     */
    [[nodiscard]] QString getDatabaseDirectory();
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
    [[nodiscard]] qint64 getDatabaseCutOff();
    /**
     * Get the mode for the database overlay for the music view.
     *
     * @return true, if the overlay is enabled, false otherwise.
     */
    [[nodiscard]] bool getDatabaseMusicOverlay();
    /**
     * Get the time when music is considered played.
     *
     * @return amount of time in ms.
     */
    [[nodiscard]] qint64 getDatabaseMusicPlayed();
    /**
     * Get the mode for the database overlay for the movie view.
     *
     * @return true, if the overlay is enabled, false otherwise.
     */
    [[nodiscard]] bool getDatabaseMovieOverlay();
    /**
     * Get the time when movie is considered played.
     *
     * @return amount of time in ms.
     */
    [[nodiscard]] qint64 getDatabaseMoviePlayed();
    /**
     * Get the mode for handling database update errors.
     *
     * @return true, if update errors are ignored, false otherwise.
     */
    [[nodiscard]] bool getDatabaseIgnoreUpdateErrors() const;
    /**
     * Get the configuration path for the projectM visualization.
     *
     * @return the absolute path as string.
     */
    [[nodiscard]] QString getVisualizationConfigPath() const;
    /**
     * Get the visualization preset for projectM.
     *
     * @return the name of the preset as string.
     */
    [[nodiscard]] QString getVisualizationPreset() const;
    /**
     * Get the default zoom factor for websites.
     *
     * @return the index for the zoom factor.
     */
    [[nodiscard]] int getWebsiteZoomFactorIndex();
    /**
     * Get the available zoom factors for websites in percent.
     *
     * @return list of integers.
     */
    [[nodiscard]] static const QList<int>& getWebsiteZoomFactors();
    /**
     * Get the user agent used for websites.
     *
     * @return the user agent as string.
     */
    [[nodiscard]] QString getWebsiteUserAgent();
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
     * Signal an update of the BluOS player library.
     */
    void updatedMusicLibraryBluOS();
    /**
     * Signal an update of the music library use.
     */
    void updatedUseMusicLibraryBluOS();
    /**
     * Signal an update of the accepted music file extensions.
     */
    void updatedMusicLibraryExtensions();
    /**
     * Signal an update of the album selectors.
     */
    void updatedMusicLibraryAlbumSelectors();
    /**
     * Signal an update of the music library tagging mode.
     */
    void updatedUseLLTag();
    /**
     * Signal an update of the lltag binary.
     */
    void updatedLLTag();
    /**
     * Signal an update of the tag list.
     */
    void updatedMusicLibraryTags();
    /**
     * Signal an update of the selectors visibility.
     */
    void updatedMusicViewSelectors();
    /**
     * Signal an update of the filters visibility.
     */
    void updatedMusicViewFilters();
    /**
     * Signal an update of the visualization visibility.
     */
    void updatedMusicViewVisualization();
    /**
     * Signal an update of the visualization mode.
     */
    void updatedMusicViewVisualizationMode();
    /**
     * Signal an update of the movie library directory.
     */
    void updatedMovieLibraryTagsAndDirectories();
    /**
     * Signal an update of the accepted movie file extensions.
     */
    void updatedMovieLibraryExtensions();
    /**
     * Signal an update of the default movie audio channel language.
     */
    void updatedMovieDefaultAudioLanguage();
    /**
     * Signal and update of the default movie subtitle language.
     */
    void updatedMovieDefaultSubtitleLanguage();
    /**
     * Signal an update of the audio compression mode for movies.
     */
    void updatedMovieAudioCompression();
    /**
     * Signal an update of the movie filters visibility.
     */
    void updatedMovieViewFilters();
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
     * Signal an update of the sidebar visibility.
     */
    void updatedStreamingViewSidebar();
    /**
     * Signal an update of the navigation visibility.
     */
    void updatedStreamingViewNavigation();
    /**
     * Signal an update of the database overlay for the music view.
     */
    void updatedDatabaseDirectory();
    /**
     * Signal an update of the database overlay for the music view.
     */
    void updatedDatabaseMusicOverlay();
    /**
     * Signal an update of the time the music is played.
     */
    void updatedDatabaseMusicPlayed();
    /**
     * Signal an update of the database overlay for the movie view.
     */
    void updatedDatabaseMovieOverlay();
    /**
     * Signal an update of the time the movie is played.
     */
    void updatedDatabaseMoviePlayed();
    /**
     * Signal an update of the projectM visualization config path.
     */
    void updatedVisualizationConfigPath();
    /**
     * Signal an update of the zoom factor.
     */
    void updatedWebsiteZoomFactor();
    /**
     * Signal an update of the website user agent.
     */
    void updatedWebsiteUserAgent();

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
    QString dataBaseFile;
    bool databaseIgnoreUpdateErrors;
};

#endif
