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

#ifndef __XMOVIEPLAYER_H__
#define __XMOVIEPLAYER_H__

#include "xPlayerPulseAudioControls.h"
#include "xMovieFile.h"

#include <QMediaPlayer>
#include <QVideoWidget>
#include <filesystem>

class xMoviePlayer:public QVideoWidget {
    Q_OBJECT

public:
    /**
     * State of the movie player.
     */
    enum State {
        PlayingState,
        PauseState,
        StoppingState,
        StopState,
        ResetState,
    };

    enum AspectRatio {
        RatioAuto = Qt::KeepAspectRatio,
        RatioKeep = Qt::KeepAspectRatio,
        RatioIgnore = Qt::IgnoreAspectRatio,
        RatioExpanding = Qt::KeepAspectRatioByExpanding,
    };

    explicit xMoviePlayer(QWidget* parent=nullptr);
    ~xMoviePlayer() noexcept override;
    /**
     * Set the full window mode.
     *
     * @param enabled enable full window mode if true, disable otherwise.
     */
    void setFullWindowMode(bool enabled);
    /**
     * Return the state of the full window mode.
     *
     * @return true if full window mode is enabled, false otherwise.
     */
    [[nodiscard]] bool getFullWindowMode() const;
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
    /**
     * Return the supported aspect ratio.
     *
     * @return a list of pairs (label, mode) of aspect ratio.
     */
    [[nodiscard]] static std::list<std::pair<QString,xMoviePlayer::AspectRatio>> supportedAspectRatio();

signals:
    /**
     * Signal any error in the movie player.
     *
     * @param msg the error message to be shown.
     */
    void moviePlayerError(QString msg);
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
     * @param audioCodecs the corresponding audio codecs as list of strings.
     */
    void currentAudioChannels(const QStringList& audioChannels, const QStringList& audioCodecs);
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
     * Signal the chapter the current movie.
     *
     * @param chapter the chapter number as integer.
     */
    void currentChapter(int chapter);
    /**
     * Signal the number chapters for the current movie.
     *
     * @param chapters number of chapters as integer.
     */
    void currentChapters(const QStringList& chapters);
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
     * Signal a toggle in between enable/disable of the full screen mode.
     */
    void fullWindowMode(bool enabled);
    /**
     * Signal the name of the currently played movie.
     *
     * @param path the absolute path of the currently played movie.
     * @param name the name of the currently played movie.
     * @param tag the tag for the currently played movie.
     * @param directory the directory for the currently played movie.
     */
    void currentMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory);
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
    void currentState(xMoviePlayer::State state);
    /**
     * Update the database overlay for currently played movie and add tooltips.
     *
     * @param tag the tag for the currently played movie.
     * @param directory the directory for the currently played movie.
     * @param movie the name of the currently played movie.
     * @param playCount the play count for the currently played movie.
     * @param timeStamp the last played time stamp in milli seconds for the currently played movie.
     */
    void updatePlayedMovie(const QString& tag, const QString& directory, const QString& movie, int playCount, qint64 timeStamp);
    /**
     * Helper signal to call stop from event handler callback.
     */
    void eventHandler_stop();
    /**
     * Helper signal to call setMovie from event handler callback.
     */
    void eventHandler_setMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory);
    /*
     * Helper signal to call selectAudioChannel from the event handler callback.
     */
    void eventHandler_selectAudioChannel(int audioChannel);
    /*
     * Helper signal to call selectSubtitle from the event handler callback.
     */
    void eventHandler_selectSubtitle(int subtitle);
    /*
     * Helper signal to call parseFinished from the event handler callback.
     */
    void eventHandler_parseFinished();

public slots:
    /**
     * Play or pause the current movie depending on the current state.
     */
    void playPause();
    /**
     * Play starting with the given chapter index (starting from 0).
     *
     * @param chapter the chapter number as integer.
     */
    void playChapter(int chapter);
    /**
     * Play the previous chapter.
     */
    void previousChapter();
    /**
     * Play the next chapter.
     */
    void nextChapter();
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
    void setMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory);
    /**
     * Setup the queue of movies to be played after the current one.
     *
     * @param queue list of pairs of path and name to be displayed.
     */
    void setMovieQueue(const QList<std::pair<std::filesystem::path,QString>>& queue);
    /**
     * Clear the queue of movies.
     */
    void clearMovieQueue();
    /**
     * Set the aspect ratio used for the video output crop.
     *
     * @param aspectRatio the aspect ratio.
     */
    void setAspectRatio(xMoviePlayer::AspectRatio aspectRatio);
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
     * Get the number of chapters for the current movie.
     *
     * @param chapters number of chapters as integer.
     */
    void availableChapters(int chapters);
    /**
     * Get the number of titles for the current movie.
     *
     * @param titles number of titles as integer.
     */
    void availableTitles(int titles);
    /**
     * The audio channels for the current movie have been determined.
     */
    void availableAudioChannels();
    /**
     * The subtitles for the current movie have been determined.
     */
    void availableSubtitles();
    /**
     * Follow the state changes of the movie player.
     *
     * @param newState the current state of the movie player.
     */
    void stateChanged(QMediaPlayer::PlaybackState newState);
    /**
     * The playback of the current movie player is about to end.
     */
    void aboutToFinish();
    /**
     * Called if tick has been changed.
     */
    void updatedPosition(qint64 position);
    /**
     * Called if the media status is updated. Handles end of media.
     *
     * @param status the current media status for movie player.
     */
    void updatedMediaStatus(QMediaPlayer::MediaStatus status);

    void updatedChapter(int chapter);
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
    /**
     * Update the current chapter index.
     */
    void updateCurrentChapter();
    /**
     * Update the current position for play time recording.
     */
    void updateCurrentPosition();
    /**
     * VLC handler for Media events.
     */
    // static void handleVLCMediaEvents(const libvlc_event_t*  event, void* data);
    /**
     * Scan the media file for chapters.
     */
    void scanForChapters();

    xPlayerPulseAudioControls* pulseAudioControls;
    xMovieFile* movieFile;
    QMediaPlayer* moviePlayer;
    QAudioOutput* audioOutput;
    xMoviePlayer::State moviePlayerState;
    QList<QMediaMetaData> subtitlesMetaData;
    QList<QMediaMetaData> audioChannelsMetaData;
    QList<std::pair<std::filesystem::path,QString>> movieQueue;
    qint64 movieMediaLength;
    QVector<qint64> movieMediaChapterBegin;
    int movieMediaChapter;
    QString movieMediaCropAspectRatio;
    bool movieMediaFullWindow;
    bool movieTickConnected;
    qint64 movieCurrentPosition;
    qint64 movieCurrentPlayed;
    qint64 moviePlayed;
    bool movieCurrentSkip;
    bool moviePlayedRecorded;
    QStringList currentChapterDescriptions;
    std::pair<std::filesystem::path,QString> movieCurrent;
    QString movieCurrentTag;
    QString movieCurrentDirectory;
    QString movieDefaultAudioLanguage;
    QString movieDefaultSubtitleLanguage;
};

Q_DECLARE_METATYPE(std::filesystem::path)

#endif
