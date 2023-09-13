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
const QString xPlayerConfiguration_MusicLibraryDirectory { "xPlay/MusicLibraryDirectory" }; // NOLINT
const QString xPlayerConfiguration_MusicLibraryBluOS { "xPlay/MusicLibraryBluOS" }; // NOLINT
const QString xPlayerConfiguration_UseMusicLibraryBluOS { "xPlay/UseMusicLibraryBluOS" }; // NOLINT
const QString xPlayerConfiguration_MusicLibraryExtensions { "xPlay/MusicLibraryExtensions" }; // NOLINT
const QString xPlayerConfiguration_MusicLibraryAlbumSelectors { "xPlay/MusicLibraryAlbumSelectors" }; // NOLINT
const QString xPlayerConfiguration_MusicLibraryTags { "xPlay/MusicLibraryTags" }; // NOLINT
const QString xPlayerConfiguration_UseLLTag {"xPlay/UseLLTag" }; // NOLINT
const QString xPlayerConfiguration_LLTag {"xPlay/LLTag" }; // NOLINT
const QString xPlayerConfiguration_MusicViewSelectors { "xPlay/MusicViewSelectors" }; // NOLINT
const QString xPlayerConfiguration_MusicViewFilters { "xPlay/MusicViewFilters" }; // NOLINT
const QString xPlayerConfiguration_MusicViewVisualization { "xPlay/MusicViewVisualization" }; // NOLINT
const QString xPlayerConfiguration_MusicViewVisualizationMode { "xPlay/MusicViewVisualizationMode" }; // NOLINT
const QString xPlayerConfiguration_RotelWidget { "xPlay/RotelWidget" }; // NOLINT
const QString xPlayerConfiguration_RotelNetworkAddress { "xPlay/RotelNetworkAddress" }; // NOLINT
const QString xPlayerConfiguration_RotelNetworkPort { "xPlay/RotelNetworkPort" }; // NOLINT
const QString xPlayerConfiguration_MovieLibraryDirectory { "xPlay/MovieLibraryDirectory" }; // NOLINT
const QString xPlayerConfiguration_MovieLibraryExtensions { "xPlay/MovieLibraryExtensions" }; // NOLINT
const QString xPlayerConfiguration_MovieDefaultAudioLanguage { "xPlay/MovieDefaultAudioLanguage" }; // NOLINT
const QString xPlayerConfiguration_MovieDefaultSubtitleLanguage { "xPlay/MovieSubtitleAudioLanguage" }; // NOLINT
const QString xPlayerConfiguration_MovieAudioCompression { "xPlay/MovieAudioCompression" }; // NOLINT
const QString xPlayerConfiguration_MovieViewFilters { "xPlay/MovieViewFilters" }; // NOLINT
const QString xPlayerConfiguration_StreamingSites { "xPlay/StreamingSites" }; // NOLINT
const QString xPlayerConfiguration_StreamingSitesDefault { "xPlay/StreamingSitesDefault" }; // NOLINT
const QString xPlayerConfiguration_StreamingViewSidebar { "xPlay/StreamingViewSidebar" }; // NOLINT
const QString xPlayerConfiguration_StreamingViewNavigation { "xPlay/StreamingViewNavigation" }; // NOLINT
const QString xPlayerConfiguration_DatabaseDirectory { "xPlay/DatabaseDirectory" }; // NOLINT
const QString xPlayerConfiguration_DatabaseUsePlayedLevels { "xPlay/DatabaseUsePlayedLevels" }; // NOLINT
const QString xPlayerConfiguration_DatabasePlayedLevels { "xPlay/DatabasePlayedLevels" }; // NOLINT
const QString xPlayerConfiguration_DatabaseCutOff { "xPlay/DatabaseCutOff" }; // NOLINT
const QString xPlayerConfiguration_DatabaseMusicOverlay { "xPlay/DatabaseMusicOverlay" }; // NOLINT
const QString xPlayerConfiguration_DatabaseMusicPlayed { "xPlay/DatabaseMusicPlayed" }; // NOLINT
const QString xPlayerConfiguration_DatabaseMovieOverlay { "xPlay/DatabaseMovieOverlay" }; // NOLINT
const QString xPlayerConfiguration_DatabaseMoviePlayed { "xPlay/DatabaseMoviePlayed" }; // NOLINT
const QString xPlayerConfiguration_VisualizationConfigPath { "xPlay/VisualizationConfigPath" }; // NOLINT
const QString xPlayerConfiguration_VisualizationPreset { "xPlay/VisualizationPreset" }; // NOLINT
const QString xPlayerConfiguration_WebsiteZoomFactor { "xPlay/WebsiteZoomFactor" }; // NOLINT
const QString xPlayerConfiguration_WebsiteUserAgent { "xPlay/WebsiteUserAgent" }; // NOLINT

// Configuration defaults.
const bool xPlayerConfiguration_UseMusicLibraryBluOS_Default = false; // NOLINT
const QString xPlayerConfiguration_MusicLibraryExtensions_Default { ".flac .ogg .mp3" }; // NOLINT
const QString xPlayerConfiguration_MusicLibraryAlbumSelectors_Default { "(live) [hd] [mp3]" }; // NOLINT
const QString xPlayerConfiguration_MusicLibraryTags_Default { "[ballads] [epics] [favorites]" }; // NOLINT
const QString xPlayerConfiguration_LLTag_Default {"/usr/bin/lltag" }; // NOLINT
const bool xPlayerConfiguration_MusicViewSelectors_Default = true; // NOLINT
const bool xPlayerConfiguration_MusicViewFilters_Default = false; // NOLINT
const bool xPlayerConfiguration_MusicViewVisualization_Default = false; // NOLINT
const int xPlayerConfiguration_MusicViewVisualizationMode_Default = 0; // NOLINT
const QString xPlayerConfiguration_MovieLibraryExtensions_Default { ".mkv .mp4 .avi .mov .wmv" }; // NOLINT
const bool xPlayerConfiguration_MovieAudioCompression_Default = true; // NOLINT
const bool xPlayerConfiguration_MovieViewFilters_Default = true; // NOLINT
const bool xPlayerConfiguration_DatabaseUsePlayedLevels_Default = false; // NOLINT
const std::tuple<int,int,int> xPlayerConfiguration_DatabasePlayedLevels_Default { 5, 10, 15 }; // NOLINT
const QList<std::pair<QString,QUrl>> xPlayerConfiguration_StreamingDefaultSites = { // NOLINT
        { "qobuz", QUrl("https://play.qobuz.com/login") },
        { "youtube", QUrl("https://www.youtube.com") },
};
const bool xPlayerConfiguration_StreamingViewSidebar_Default = true; // NOLINT
const bool xPlayerConfiguration_StreamingViewNavigation_Default = true; // NOLINT
const QStringList xPlayerConfiguration_MovieDefaultLanguages { "english", "german" }; // NOLINT
const QString xPlayerConfiguration_VisualizationConfigPathDefault { "/usr/share/projectM/config.inp" }; // NOLINT
const QList<int> xPlayerConfiguration_WebsiteZoomFactors { 50, 75, 100, 125, 150, 175, 200 }; // NOLINT
const QString xPlayerConfiguration_WebsiteUserAgent_Default { "Mozilla/5.0 (X11; Linux x86_64; rv:103.0) Gecko/20100101 Firefox/103.0" }; // NOLINT


// singleton object.
xPlayerConfiguration* xPlayerConfiguration::playerConfiguration = nullptr;

xPlayerConfiguration::xPlayerConfiguration():
        QObject(),
        databaseIgnoreUpdateErrors(false) {
    // Settings.
    settings = new QSettings(xPlayerConfiguration::OrganisationName, xPlayerConfiguration::ApplicationName, this);
    databaseFile = QString("%1.db").arg(xPlayerConfiguration::ApplicationName);
    // Cache the played levels and the used played mode.
    auto [_bronze, _silver, _gold] = getDatabasePlayedLevels();
    databasePlayedBronze = _bronze;
    databasePlayedSilver = _silver;
    databasePlayedGold = _gold;
    databaseUsePlayed = useDatabasePlayedLevels();
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

void xPlayerConfiguration::setMusicLibraryBluOS(const QString& url) {
    if (url != getMusicLibraryBluOS()) {
        settings->setValue(xPlayerConfiguration_MusicLibraryBluOS, url);
        settings->sync();
        emit updatedMusicLibraryBluOS();
    }
}

void xPlayerConfiguration::useMusicLibraryBluOS(bool enabled) {
    if (enabled != useMusicLibraryBluOS()) {
        // Disable visualization if we enable the BluOS player library.
        if (enabled) {
            setMusicViewVisualization(false);
        }
        settings->setValue(xPlayerConfiguration_UseMusicLibraryBluOS, enabled);
        settings->sync();
        emit updatedUseMusicLibraryBluOS();
    }
}

void xPlayerConfiguration::setMusicLibraryExtensions(const QString& extensions) {
    if (extensions != getMusicLibraryExtensions()) {
        settings->setValue(xPlayerConfiguration_MusicLibraryExtensions, extensions);
        settings->sync();
        emit updatedMovieLibraryExtensions();
    }
}

void xPlayerConfiguration::setMusicLibraryAlbumSelectors(const QString& selectors) {
    if (selectors != getMusicLibraryAlbumSelectors()) {
        settings->setValue(xPlayerConfiguration_MusicLibraryAlbumSelectors, selectors);
        settings->sync();
        emit updatedMusicLibraryAlbumSelectors();
    }
}

void xPlayerConfiguration::setMusicLibraryTags(const QStringList& tags) {
    if (tags != getMusicLibraryTags()) {
        settings->setValue(xPlayerConfiguration_MusicLibraryTags, tags.join(" "));
        settings->sync();
        emit updatedMusicLibraryTags();
    }
}

void xPlayerConfiguration::useLLTag(bool enabled) {
    if (enabled != useLLTag()) {
        settings->setValue(xPlayerConfiguration_UseLLTag, enabled);
        settings->sync();
        emit updatedUseLLTag();
    }
}

void xPlayerConfiguration::setLLTag(const QString& lltag) {
    if (lltag != getLLTag()) {
        settings->setValue(xPlayerConfiguration_LLTag, lltag);
        settings->sync();
        emit updatedLLTag();
    }
}

void xPlayerConfiguration::setMusicViewSelectors(bool visible) {
    if (visible != getMusicViewSelectors()) {
        settings->setValue(xPlayerConfiguration_MusicViewSelectors, visible);
        settings->sync();
        emit updatedMusicViewSelectors();
    }
}

void xPlayerConfiguration::setMusicViewFilters(bool visible) {
    if (visible != getMusicViewFilters()) {
        settings->setValue(xPlayerConfiguration_MusicViewFilters, visible);
        settings->sync();
        emit updatedMusicViewFilters();
    }
}

void xPlayerConfiguration::setMusicViewVisualization(bool visible) {
    // Visualization is disabled for BluOS player libraries.
    if (useMusicLibraryBluOS()) {
        emit updatedMusicViewVisualization();
        return;
    }
    if (visible != getMusicViewVisualization()) {
        settings->setValue(xPlayerConfiguration_MusicViewVisualization, visible);
        settings->sync();
        emit updatedMusicViewVisualization();
    }
}

void xPlayerConfiguration::setMusicViewVisualizationMode(int mode) {
    if (mode != getMusicViewVisualizationMode()) {
        settings->setValue(xPlayerConfiguration_MusicViewVisualizationMode, mode);
        settings->sync();
        emit updatedMusicViewVisualizationMode();
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

void xPlayerConfiguration::setMovieDefaultAudioLanguage(const QString& language) {
    if (language != getMovieDefaultAudioLanguage()) {
        settings->setValue(xPlayerConfiguration_MovieDefaultAudioLanguage, language);
        settings->sync();
        emit updatedMovieDefaultAudioLanguage();
    }
}

void xPlayerConfiguration::setMovieDefaultSubtitleLanguage(const QString& language) {
    if (language != getMovieDefaultSubtitleLanguage()) {
        settings->setValue(xPlayerConfiguration_MovieDefaultSubtitleLanguage, language);
        settings->sync();
        emit updatedMovieDefaultSubtitleLanguage();
    }
}

void xPlayerConfiguration::setMovieAudioCompression(bool enabled) {
    if (enabled != getMovieAudioCompression()) {
        settings->setValue(xPlayerConfiguration_MovieAudioCompression, enabled);
        settings->sync();
        emit updatedMovieAudioCompression();
    }
}

void xPlayerConfiguration::setMovieViewFilters(bool visible) {
    if (visible != getMovieViewFilters()) {
        settings->setValue(xPlayerConfiguration_MovieViewFilters, visible);
        settings->sync();
        emit updatedMovieViewFilters();
    }
}

void xPlayerConfiguration::setStreamingSites(const QList<std::pair<QString,QUrl>>& sites) {
    if ((sites != getStreamingSites()) || (sites == xPlayerConfiguration_StreamingDefaultSites)) {
        QString nameUrlString;
        if (!sites.isEmpty()) {
            nameUrlString = QString("(%1) - %2").arg(sites.at(0).first, sites.at(0).second.toString());
            for (int i = 1; i < sites.size(); ++i) {
                nameUrlString += QString("|(%1) - %2").arg(sites.at(i).first, sites.at(i).second.toString());
            }
        }
        settings->setValue(xPlayerConfiguration_StreamingSites, nameUrlString);
        settings->sync();
        emit updatedStreamingSites();
    }
}

void xPlayerConfiguration::setStreamingViewSidebar(bool visible) {
    if (visible != getStreamingViewSidebar()) {
        settings->setValue(xPlayerConfiguration_StreamingViewSidebar, visible);
        settings->sync();
        emit updatedStreamingViewSidebar();
    }
}

void xPlayerConfiguration::setStreamingViewNavigation(bool visible) {
    if (visible != getStreamingViewNavigation()) {
        settings->setValue(xPlayerConfiguration_StreamingViewNavigation, visible);
        settings->sync();
        emit updatedStreamingViewNavigation();
    }
}

void xPlayerConfiguration::setDatabaseDirectory(const QString& dir) {
    // Make sure that dir ends with "/".
    auto dataDir = (dir.endsWith('/')) ? dir : dir+"/";
    if (dataDir != getDatabaseDirectory()) {
        settings->setValue(xPlayerConfiguration_DatabaseDirectory, dataDir);
        settings->sync();
        emit updatedDatabaseDirectory();
        // Trigger reconfiguration of music and movie overlay.
        emit updatedDatabaseMusicOverlay();
        emit updatedDatabaseMovieOverlay();
    }
}

void xPlayerConfiguration::setDatabaseCutOff(qint64 cutOff) {
    if (cutOff != getDatabaseCutOff()) {
        settings->setValue(xPlayerConfiguration_DatabaseCutOff, cutOff);
        settings->sync();
        // Trigger reconfiguration of music and movie overlay.
        emit updatedDatabaseMusicOverlay();
        emit updatedDatabaseMovieOverlay();
    }
}

void xPlayerConfiguration::useDatabasePlayedLevels(bool enabled) {
    if (enabled != useDatabasePlayedLevels()) {
        databaseUsePlayed = enabled;
        settings->setValue(xPlayerConfiguration_DatabaseUsePlayedLevels, enabled);
        settings->sync();
        emit updatedDatabaseMusicOverlay();
        emit updatedDatabaseMovieOverlay();
    }
}

void xPlayerConfiguration::setDatabasePlayedLevels(int bronze, int silver, int gold) {
    auto playedLevels = std::make_tuple(bronze, silver, gold);
    if (playedLevels != getDatabasePlayedLevels()) {
        // Cache the played levels
        databasePlayedBronze = bronze;
        databasePlayedSilver = silver;
        databasePlayedGold = gold;
        settings->setValue(xPlayerConfiguration_DatabasePlayedLevels+"/Bronze", bronze);
        settings->setValue(xPlayerConfiguration_DatabasePlayedLevels+"/Silver", silver);
        settings->setValue(xPlayerConfiguration_DatabasePlayedLevels+"/Gold", gold);
        settings->sync();
        emit updatedDatabaseMusicOverlay();
        emit updatedDatabaseMovieOverlay();
    }
}

void xPlayerConfiguration::setDatabaseMusicOverlay(bool enabled) {
    if (enabled != getDatabaseMusicOverlay()) {
        settings->setValue(xPlayerConfiguration_DatabaseMusicOverlay, enabled);
        settings->sync();
        emit updatedDatabaseMusicOverlay();
    }
}

void xPlayerConfiguration::setDatabaseMusicPlayed(qint64 played) {
    if (played != getDatabaseMusicPlayed()) {
        settings->setValue(xPlayerConfiguration_DatabaseMusicPlayed, played);
        settings->sync();
        emit updatedDatabaseMusicPlayed();
    }
}

void xPlayerConfiguration::setDatabaseMovieOverlay(bool enabled) {
    if (enabled != getDatabaseMovieOverlay()) {
       settings->setValue(xPlayerConfiguration_DatabaseMovieOverlay, enabled);
       settings->sync();
       emit updatedDatabaseMovieOverlay();
    }
}

void xPlayerConfiguration::setDatabaseMoviePlayed(qint64 played) {
    if (played != getDatabaseMoviePlayed()) {
        settings->setValue(xPlayerConfiguration_DatabaseMoviePlayed, played);
        settings->sync();
        emit updatedDatabaseMoviePlayed();
    }
}

void xPlayerConfiguration::setDatabaseIgnoreUpdateErrors(bool enabled) {
    databaseIgnoreUpdateErrors = enabled;
}

void xPlayerConfiguration::setStreamingSitesDefault(const std::pair<QString,QUrl>& site) {
    if (site != getStreamingSitesDefault()) {
        auto nameUrlString = QString("(%1) - %2").arg(site.first, site.second.toString());
        settings->setValue(xPlayerConfiguration_StreamingSitesDefault, nameUrlString);
        settings->sync();
        emit updatedStreamingSitesDefault();
    }
}

void xPlayerConfiguration::setVisualizationConfigPath(const QString& path) {
    if (path != getVisualizationConfigPath()) {
        settings->setValue(xPlayerConfiguration_VisualizationConfigPath, path);
        settings->sync();
        emit updatedVisualizationConfigPath();
    }
}

void xPlayerConfiguration::setVisualizationPreset(const QString& preset) {
    if (preset != getVisualizationPreset()) {
        settings->setValue(xPlayerConfiguration_VisualizationPreset, preset);
        settings->sync();
    }
}

void xPlayerConfiguration::setWebsiteZoomFactorIndex(int index) {
    if (index != getWebsiteZoomFactorIndex()) {
        settings->setValue(xPlayerConfiguration_WebsiteZoomFactor, index);
        settings->sync();
        emit updatedWebsiteZoomFactor();
    }
}

void xPlayerConfiguration::setWebsiteUserAgent(const QString &userAgent) {
    if (userAgent != getWebsiteUserAgent()) {
        settings->setValue(xPlayerConfiguration_WebsiteUserAgent, userAgent);
        settings->sync();
        emit updatedWebsiteUserAgent();
    }
}

QString xPlayerConfiguration::getMusicLibraryDirectory() {
    return settings->value(xPlayerConfiguration_MusicLibraryDirectory, "").toString();
}

QString xPlayerConfiguration::getMusicLibraryBluOS() {
    return settings->value(xPlayerConfiguration_MusicLibraryBluOS, "").toString();
}

bool xPlayerConfiguration::useMusicLibraryBluOS() {
    return settings->value(xPlayerConfiguration_UseMusicLibraryBluOS, xPlayerConfiguration_UseMusicLibraryBluOS_Default).toBool();
}

std::filesystem::path xPlayerConfiguration::getMusicLibraryDirectoryPath() {
    return std::filesystem::path{ getMusicLibraryDirectory().toStdString() };
}

QString xPlayerConfiguration::getMusicLibraryExtensions() {
    return settings->value(xPlayerConfiguration_MusicLibraryExtensions,
                           xPlayerConfiguration_MusicLibraryExtensions_Default).toString();
}

QStringList xPlayerConfiguration::getMusicLibraryExtensionList() {
    auto extensions = getMusicLibraryExtensions();
    if (extensions.isEmpty()) {
        return QStringList{};
    } else {
        return extensions.split(" ");
    }
}

QString xPlayerConfiguration::getMusicLibraryAlbumSelectors() {
    return settings->value(xPlayerConfiguration_MusicLibraryAlbumSelectors,
                           xPlayerConfiguration_MusicLibraryAlbumSelectors_Default).toString();
}

QStringList xPlayerConfiguration::getMusicLibraryAlbumSelectorList() {
    auto selectors = getMusicLibraryAlbumSelectors();
    if (selectors.isEmpty()) {
        return QStringList{};
    } else {
        return selectors.split(" ");
    }
}

QStringList xPlayerConfiguration::getMusicLibraryTags() {
    auto tags = settings->value(xPlayerConfiguration_MusicLibraryTags,
                                xPlayerConfiguration_MusicLibraryTags_Default).toString();
    if (tags.isEmpty()) {
        return QStringList{};
    } else {
        return tags.split(" ");
    }
}

bool xPlayerConfiguration::useLLTag() {
    return settings->value(xPlayerConfiguration_UseLLTag, true).toBool();
}

QString xPlayerConfiguration::getLLTag() {
    return settings->value(xPlayerConfiguration_LLTag,
                           xPlayerConfiguration_LLTag_Default).toString();
}

bool xPlayerConfiguration::getMusicViewSelectors() {
    return settings->value(xPlayerConfiguration_MusicViewSelectors, xPlayerConfiguration_MusicViewSelectors_Default).toBool();
}

bool xPlayerConfiguration::getMusicViewFilters() {
    return settings->value(xPlayerConfiguration_MusicViewFilters, xPlayerConfiguration_MusicViewFilters_Default).toBool();
}

bool xPlayerConfiguration::getMusicViewVisualization() {
    return settings->value(xPlayerConfiguration_MusicViewVisualization, xPlayerConfiguration_MusicViewVisualization_Default).toBool();
}

int xPlayerConfiguration::getMusicViewVisualizationMode() {
    return settings->value(xPlayerConfiguration_MusicViewVisualizationMode, xPlayerConfiguration_MusicViewVisualizationMode_Default).toInt();
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
        return QStringList{};
    } else {
        return movieLibraryDirectory.split("|");
    }
}

bool xPlayerConfiguration::getMovieAudioCompression() {
    return settings->value(xPlayerConfiguration_MovieAudioCompression, xPlayerConfiguration_MovieAudioCompression_Default).toBool();
}

bool xPlayerConfiguration::getMovieViewFilters() {
    return settings->value(xPlayerConfiguration_MovieViewFilters, xPlayerConfiguration_MovieViewFilters_Default).toBool();
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

bool xPlayerConfiguration::getStreamingViewSidebar() {
    return settings->value(xPlayerConfiguration_StreamingViewSidebar, xPlayerConfiguration_StreamingViewSidebar_Default).toBool();
}

bool xPlayerConfiguration::getStreamingViewNavigation() {
    return settings->value(xPlayerConfiguration_StreamingViewNavigation, xPlayerConfiguration_StreamingViewNavigation_Default).toBool();
}

QString xPlayerConfiguration::getDatabaseDirectory() {
    auto path = settings->value(xPlayerConfiguration_DatabaseDirectory, "").toString();
    if (path.isEmpty()) {
        path = QString("%1/%2/").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
                                     xPlayerConfiguration::OrganisationName);
    }
    return path;
}

QString xPlayerConfiguration::getDatabasePath() {
    return getDatabaseDirectory()+databaseFile;
}

bool xPlayerConfiguration::useDatabasePlayedLevels() {
    return settings->value(xPlayerConfiguration_DatabaseUsePlayedLevels,
                           xPlayerConfiguration_DatabaseUsePlayedLevels_Default).toBool();
}

std::tuple<int,int,int> xPlayerConfiguration::getDatabasePlayedLevels() {
    auto playedBronze = settings->value(xPlayerConfiguration_DatabasePlayedLevels+"/Bronze",
                                        std::get<0>(xPlayerConfiguration_DatabasePlayedLevels_Default)).toInt();
    auto playedSilver = settings->value(xPlayerConfiguration_DatabasePlayedLevels+"/Silver",
                                        std::get<1>(xPlayerConfiguration_DatabasePlayedLevels_Default)).toInt();
    auto playedGold = settings->value(xPlayerConfiguration_DatabasePlayedLevels+"/Gold",
                                        std::get<2>(xPlayerConfiguration_DatabasePlayedLevels_Default)).toInt();
    return std::make_tuple(playedBronze, playedSilver, playedGold);
}

QString xPlayerConfiguration::getPlayedLevelIcon(int playCount) {
    // Use cached values to improve performance.
    if (databaseUsePlayed) {
        // We assume that bronze < silver < gold.
        if (playCount < databasePlayedBronze) {
            return ":images/xplay-star.svg";
        }
        if (playCount < databasePlayedSilver) {
            return ":images/xplay-star-bronze.svg";
        }
        if (playCount < databasePlayedGold) {
            return ":images/xplay-star-silver.svg";
        }
        return ":images/xplay-star-gold.svg";
    } else {
        return ":images/xplay-star.svg";
    }
}

qint64 xPlayerConfiguration::getDatabaseCutOff() {
    return settings->value(xPlayerConfiguration_DatabaseCutOff, 0).toLongLong();
}

bool xPlayerConfiguration::getDatabaseMusicOverlay() {
    return settings->value(xPlayerConfiguration_DatabaseMusicOverlay, true).toBool();
}

qint64 xPlayerConfiguration::getDatabaseMusicPlayed() {
    return settings->value(xPlayerConfiguration_DatabaseMusicPlayed, 0).toLongLong();
}

bool xPlayerConfiguration::getDatabaseMovieOverlay() {
    return settings->value(xPlayerConfiguration_DatabaseMovieOverlay, true).toBool();
}

qint64 xPlayerConfiguration::getDatabaseMoviePlayed() {
    return settings->value(xPlayerConfiguration_DatabaseMoviePlayed, 0).toLongLong();
}

bool xPlayerConfiguration::getDatabaseIgnoreUpdateErrors() const {
    return databaseIgnoreUpdateErrors;
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
        return QStringList{};
    } else {
        return extensions.split(" ");
    }
}

QString xPlayerConfiguration::getMovieDefaultAudioLanguage() {
    return settings->value(xPlayerConfiguration_MovieDefaultAudioLanguage, "").toString();
}

QString xPlayerConfiguration::getMovieDefaultSubtitleLanguage() {
    return settings->value(xPlayerConfiguration_MovieDefaultSubtitleLanguage, "").toString();
}

const QStringList& xPlayerConfiguration::getMovieDefaultLanguages() {
    return xPlayerConfiguration_MovieDefaultLanguages;
}

QString xPlayerConfiguration::getVisualizationConfigPath() const {
    return settings->value(xPlayerConfiguration_VisualizationConfigPath,
                           xPlayerConfiguration_VisualizationConfigPathDefault).toString();
}

QString xPlayerConfiguration::getVisualizationPreset() const {
    return settings->value(xPlayerConfiguration_VisualizationPreset, "").toString();
}

int xPlayerConfiguration::getWebsiteZoomFactorIndex() {
    // Default entry at index 2 is 100%
    return settings->value(xPlayerConfiguration_WebsiteZoomFactor, 2).toInt();
}

QString xPlayerConfiguration::getWebsiteUserAgent() {
    return settings->value(xPlayerConfiguration_WebsiteUserAgent, xPlayerConfiguration_WebsiteUserAgent_Default).toString();
}

const QList<int>& xPlayerConfiguration::getWebsiteZoomFactors() {
    return xPlayerConfiguration_WebsiteZoomFactors;
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

void xPlayerConfiguration::updatedConfiguration() {
    // Fire all update signals.
    emit updatedMusicLibraryDirectory();
    emit updatedMusicLibraryBluOS();
    emit updatedUseMusicLibraryBluOS();
    emit updatedMusicLibraryExtensions();
    emit updatedMusicLibraryAlbumSelectors();
    emit updatedUseLLTag();
    emit updatedLLTag();
    emit updatedMusicLibraryTags();
    emit updatedMusicViewSelectors();
    emit updatedMusicViewFilters();
    emit updatedMusicViewVisualization();
    emit updatedMusicViewVisualizationMode();
    emit updatedRotelNetworkAddress();
    emit updatedMovieLibraryTagsAndDirectories();
    emit updatedMovieLibraryExtensions();
    emit updatedMovieDefaultAudioLanguage();
    emit updatedMovieDefaultSubtitleLanguage();
    emit updatedMovieAudioCompression();
    emit updatedMovieViewFilters();
    emit updatedStreamingSites();
    emit updatedStreamingSitesDefault();
    emit updatedStreamingViewSidebar();
    emit updatedStreamingViewNavigation();
    emit updatedDatabaseMusicOverlay();
    emit updatedDatabaseMusicPlayed();
    emit updatedDatabaseMovieOverlay();
    emit updatedDatabaseMoviePlayed();
    emit updatedVisualizationConfigPath();
    emit updatedWebsiteZoomFactor();
    emit updatedWebsiteUserAgent();
}
