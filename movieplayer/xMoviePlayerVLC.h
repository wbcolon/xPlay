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

#ifndef __XMOVIEPLAYERVLC_H__
#define __XMOVIEPLAYERVLC_H__

#include <QFrame>
#include <filesystem>
#include <vlc/vlc.h>

class xMoviePlayerVLC:public QFrame {
    Q_OBJECT

public:
    /**
     * State of the movie player.
     */
    enum State {
        PlayingState,
        PauseState,
        StoppingState,
        StopState
    };

    explicit xMoviePlayerVLC(QWidget* parent=nullptr);
    ~xMoviePlayerVLC() override;
    /**
     * Return the volume for the movie player
     *
     * @return integer value in between 0 and 100.
     */
    [[nodiscard]] int getVolume() const;
    /**
     * Return the mute state for the movie player
     *
     * @return true if music player is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted() const;

signals:
    /**
     * Signal the audio channel for the current movie.
     *
     * @param audioChannel the index of audio channel.
     */
    void currentAudioChannel(int audioChannel);
    /**
     * Signal the audio channels available in the current movie.
     *
     * @param audioChannels the audio channels as list of strings.
     */
    void currentAudioChannels(const QStringList& audioChannels);
    /**
     * Signal the subtitle for the current movie.
     *
     * @param subtitle the index of the subtitle.
     */
    void currentSubtitle(int subtitle);
    /**
     * Signal the subtitles available in the current movie.
     *
     * @param subtitles the subtitles as list of strings.
     */
    void currentSubtitles(const QStringList& subtitles);
    /**
     * Signal the number chapters for the current movie.
     *
     * @param chapters number of chapters as integer.
     */
    void currentChapters(int chapters);
    /**
     * Signal the number titles for the current movie.
     *
     * @param titles number of titles as integer.
     */
    void currentTitles(int titles);
    /**
     * Signal the current volume of the movie player.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    void currentVolume(int vol);
    /**
     * Signal the state of the scale and crop mode.
     *
     * @param mode true if scale and crop should be enabled, false if it is disabled.
     */
    void scaleAndCropMode(bool mode);
    /**
     * Signal a toggle in between enable/disable of the full screen mode.
     */
    void toggleFullWindow();
    /**
     * Signal the name of the currently played movie.
     *
     * @param path the absolute path of the currently played movie.
     * @param name the name of the currently played movie.
     * @param tag the tag for the currently played movie.
     * @param directory the directory for the currently played movie.
     */
    void currentMovie(const QString& path, const QString& name, const QString& tag, const QString& directory);
    /**
     * Signal the amount played for the current movie.
     *
     * @param played the amount played in milliseconds.
     */
    void currentMoviePlayed(qint64 played);
    /**
     * Signal the length of the current movie
     *
     * @param length the length in milliseconds.
     */
    void currentMovieLength(qint64 length);
    /**
     * Signal the current state of the movie player
     *
     * @param state the current state
     */
    void currentState(xMoviePlayerVLC::State state);
    /**
     * Helper signal to call stop from event handler callback.
     */
    void eventHandler_stop();
    /**
     * Helper signal to call setMovie from event handler callback.
     */
    void eventHandler_setMovie(const QString& path, const QString& name, const QString& tag, const QString& directory);

public slots:
    /**
     * Play or pause the current movie depending on the current state.
     */
    void playPause();
    /**
     * Seek to a given position within the current movie.
     *
     * @param position the position in the movie in ms.
     */
    void seek(qint64 position);
    /**
     * Jump relative from the current position within the movie.
     *
     * @param delta the offset to the current position in ms (can be negative).
     */
    void jump(qint64 delta);
    /**
     * Stop the playback of the current movie.
     */
    void stop();
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute);
    /**
     * Set the volume for the movie player.
     *
     * @param vol the new volume as integer (in between 0 and 100).
     */
    void setVolume(int vol);
    /**
     * Set the movie to be played.
     *
     * @param path absolute path to the movie to be played.
     * @param name the name of the movie displayed.
     * @param tag the tag for the movie played.
     * @param directory the directory for the movie played.
     */
    void setMovie(const QString& path, const QString& name, const QString& tag, const QString& directory);
    /**
     * Setup the queue of movies to be played after the current one.
     *
     * @param queue list of pairs of path and name to be displayed.
     * @param tag the tag for the movies in the queue.
     * @param directory the directory for the movies in the queue.
     */
    void setMovieQueue(const QList<std::pair<QString,QString>>& queue, const QString& tag, const QString& directory);
    /**
     * Clear the queue of movies.
     */
    void clearMovieQueue();
    /**
     * Enable or disable the scale and crop mode.
     *
     * @param mode enable scale and crop if true, disable otherwise.
     */
    void setScaleAndCropMode(bool mode);
    /**
     * Toggle enable/disable of the scale and crop mode.
     */
    void toggleScaleAndCropMode();
    /**
     * Select an audio channel for the current movie.
     *
     * @param index the index of the audio channel.
     */
    void selectAudioChannel(int index);
    /**
     * Select a subtitle for the current movie.
     *
     * @param index the index of the subtitle.
     */
    void selectSubtitle(int index);

private slots:
    /**
     * Called if default audio language has been changed.
     */
    void updatedDefaultAudioLanguage();
    /**
     * Called if default subtitle language has been changed.
     */
    void updatedDefaultSubtitleLanguage();

protected:
    /**
     * Receive the pressed keys within the video output.
     *
     * @param keyEvent the key event observed.
     */
    void keyPressEvent(QKeyEvent* keyEvent) override;
    /**
     * Receive any double-click within the video output.
     *
     * @param mouseEvent the mouse event observed.
     */
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) override;
    /**
     * Receive any mouse press within the video output.
     *
     * @param mouseEvent the mouse event observed.
     */
    void mousePressEvent(QMouseEvent* mouseEvent) override;

private:
    static void handleVLCEvents(const libvlc_event_t*  event, void* data);
    /**
     * Scan the media file for tracks and length.
     */
    void scanForAudioAndSubtitles();
    /**
     * Update the audio channel/subtitle description. Expand language strings.
     *
     * @param decription the current description as string.
     */
    static void updateAudioAndSubtitleDescription(QString& description);

    libvlc_instance_t* movieInstance;
    libvlc_media_player_t* movieMediaPlayer;
    libvlc_event_manager_t* movieMediaPlayerEventManager;
    libvlc_event_manager_t* movieMediaEventManager;
    libvlc_media_t* movieMedia;
    bool movieMediaInitialPlay;
    qint64 movieMediaLength;

    QList<std::pair<int,QString>> currentSubtitleDescriptions;
    QList<std::pair<int,QString>> currentAudioChannelDescriptions;
    QList<std::pair<QString,QString>> movieQueue;
    QString movieQueueTag;
    QString movieQueueDirectory;
    QString movieDefaultAudioLanguage;
    QString movieDefaultSubtitleLanguage;
    int movieDefaultAudioLanguageIndex;
    int movieDefaultSubtitleLanguageIndex;
    bool fullWindow;
};

#endif
