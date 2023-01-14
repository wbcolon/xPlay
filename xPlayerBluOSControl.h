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
#ifndef __XPLAYERBLUOSCONTROL_H__
#define __XPLAYERBLUOSCONTROL_H__

#include <QTimer>
#include <QMutex>
#include <QUrl>
#include <QRegularExpression>

#include <pugixml.hpp>
#include <curl/curl.h>


class xPlayerBluOSControls:public QObject {
    Q_OBJECT

public:
    /**
     * Return the BluOS controls.
     *
     * @return pointer to a singleton of the BluOS controls.
     */
    [[nodiscard]] static xPlayerBluOSControls* controls();
    /**
     * Connect controls via network to the BluOS player.
     *
     * @param url the url of the BluOS system (IP or DNS name and Port).
     */
    void connect(const QUrl& url);
    /**
     * Disconnect controls from BluOS player.
     */
    void disconnect();
    /**
     * Reindex the library of the BluOS player.
     */
    void reIndex();
    /**
     * Play the BluOS player.
     */
    void play();
    /**
     * Play a given entry of the current playlist.
     *
     * @param index the position of the track in the playlist.
     */
    void play(int index);
    /**
     * Pause the BluOS player.
     */
    void pause();
    /**
     * Stop the BluOS player.
     */
    void stop();
    /**
     * Move to the given position in the current track.
     *
     * @param position the position given in millisecond.
     */
    void seek(qint64 position);
    /**
     * Get the current play state of the BlueOS player.
     *
     * @return the state as string.
     */
    QString state();
    /**
     * Jump to the previous track in the playlist.
     */
    void prev();
    /**
     * Jump to the next track in the playlist.
     */
    void next();
    /**
     * Add song to the BluOS player queue.
     *
     * @param path the path to the track on the BluOS player as string.
     */
    void addQueue(const QString& path);
    /**
     * Remove song from the BluOS player queue.
     *
     * @param index the position of the track on the BluOS player.
     */
    void removeQueue(int index);
    /**
     * Get the current queue of the BluOS player.
     *
     * @return a list of strings of track paths.
     */
    std::vector<QString> getQueue();
    /**
     * Clear the BluOS player queue.
     */
    void clearQueue();
    /**
     * Set the volume
     *
     * @param vol integer value between 0 (silence) and 100 (full volume)
     */
    void setVolume(int vol);
    /**
     * Return the volume for the BluOS player
     *
     * @return integer value in between 0 and 100.
     */
    [[nodiscard]] int getVolume();
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute);
    /**
     * Return the mute mode for the BluOS player
     *
     * @return true if BluOS player is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted();
    /**
     * Set the mute mode.
     *
     * @param shuffle enable shuffle mode if true, disable otherwise.
     */
    void setShuffle(bool shuffle);
    /**
     * Return the shuffle mode for the BluOS player
     *
     * @return true if BluOS player is muted, false otherwise.
     */
    [[nodiscard]] bool isShuffle();
    /**
     * Return the artists for the BluOS player.
     *
     * @return vector of tuples of url and artist name.
     */
    std::vector<std::tuple<QUrl,QString>> getArtists();
    /**
     * Return the albums for a given artist for the BluOS player.
     *
     * @param artist the given artist as string.
     * @return vector of tuples of url and album name.
     */
    std::vector<std::tuple<QUrl,QString>> getAlbums(const QString& artist);
    /**
     * Return the tracks for a given artist and album for the BluOS player.
     *
     * @param artist the given artist as string.
     * @param album the given album as string.
     * @return vector of tuples of url, track name, length and path.
     */
    std::vector<std::tuple<QUrl,QString,qint64,QString>> getTracks(const QString& artist, const QString& album);
    /**
     * Return the info for the given track ID.
     *
     * @param path the path to the track on the BluOS player as string.
     * @return a tuple of sample rate and bits per sample.
     */
    std::tuple<int, int> getTrackInfo(const QString& path);

signals:
    void playerReIndexing(int noTracks);
    void playerStatus(const QString& path, qint64 position, const QString& quality);

private:
    /**
     * Constructor. Create network manager and connect signals.
     */
    xPlayerBluOSControls();
    /**
     * Destructor. Default.
     */
    ~xPlayerBluOSControls() override;
    /**
     * Send http requests to the BluOS Player.
     *
     * @param url the request as URL.
     * @return the request result as string.
     */
    QString sendCommand(const QUrl& url);
    /**
     * Parse the result of the initial query to determine the base path for LocalMusic.
     *
     * @param commandResult the result of the query as string.
     * @return the base path for LocalMusic as string.
     */
    QString parseBasePath(const QString& commandResult);
    /**
     * Parse the result of the player state query.
     *
     * @param commandResult the result of the query as string.
     * @return the play state as string.
     */
    QString parseState(const QString& commandResult);
    /**
     * Parse the result of the volume query.
     *
     * @param commandResult the result of the query as string.
     * @return the volume level as integer.
     */
    int parseVolume(const QString& commandResult);
    /**
     * Parse the result for mute of the status query.
     *
     * @param commandResult the result of the query as string.
     * @return true if mute is enabled, false otherwise.
     */
    bool parseMute(const QString& commandResult);
    /**
     * Parse the result for shuffle of the status query.
     *
     * @param commandResult the result of the query as string.
     * @return true if shuffle is enabled, false otherwise.
     */
    bool parseShuffle(const QString& commandResult);
    /**
     * Parse the result for the playlist query.
     *
     * @param commandResult the result of the query as string.
     * @param path the path of the track to be found.
     * @return the song id as integer.
     */
    int parsePlaylistTrackId(const QString& commandResult, const QString& path);
    /**
     * Parse the result for indexing of the status query.
     *
     * @param commandResult the result of the query as string.
     * @return number of tracks indexed.
     */
    int parseIndexing(const QString& commandResult);
    /**
     * Parse the result of a folder query.
     *
     * @param commandResult the result of the query as string.
     * @return a vector of folder names.
     */
    std::vector<QString> parseFolders(const QString& commandResult);
    /**
     * Parse the result of a track query.
     *
     * @param commandResult the result of the query as string.
     * @return a vector of tuples of track names, length and path.
     */
    std::vector<std::tuple<QString,qint64,QString>> parseTracks(const QString& commandResult);
    /**
     * Parse the result of the track info query.
     *
     * Note: the result is HTTP rather than XML.
     *
     * @param commandResult the result of the query as string.
     * @return a tuple of bitrate and bits per sample.
     */
    std::tuple<int,int> parseTrackInfo(const QString& commandResult);
    /**
     * Parse the result of a playlist query.
     *
     * @param commandResult the result of the query as string.
     * @return a vector of paths in the playlist.
     */
    std::vector<QString> parsePlaylist(const QString& commandResult);
    /**
     * Parse the result of the status query.
     *
     * Emit playerStatus signal each time the Status is queried.
     *
     * @param commandResult the result of the query as string.
     */
    void parsePlayerStatus(const QString& commandResult);

    QString bluOSUrl;
    QString bluOSBasePath;
    bool bluOSReIndexing;
    CURL* bluOSRequests;
    pugi::xml_document bluOSResponse;
    QRegularExpression* bluOSTrackInfoRegExpr;
    static xPlayerBluOSControls* bluOSControls;
    QMutex bluOSMutex;
    QTimer* bluOSStatus;
    pugi::xml_document bluOSStatusResponse;
};



#endif
