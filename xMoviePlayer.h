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

#include <phonon/MediaObject>
#include <phonon/MediaSource>
#include <phonon/MediaController>
#include <phonon/AudioOutput>
#include <phonon/VideoWidget>
#include <filesystem>

class xMoviePlayer:public Phonon::VideoWidget {
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

    xMoviePlayer(QWidget* parent=nullptr);
    ~xMoviePlayer() = default;
    /**
     * Return the volume for the movie player
     *
     * @return integer value in between 0 and 100.
     */
    int getVolume();

signals:
    /**
     * Signal the audio channels available in the current movie.
     *
     * @param audioChannels the audio channels as list of strings.
     */
    void currentAudioChannels(const QStringList& audioChannels);
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
     * Set the volume for the movie player.
     *
     * @param vol the new volume as integer (in between 0 and 100).
     */
    void setVolume(int vol);
    /**
     * Set the movie to be played.
     *
     * @param moviePath absolute path to the movie to be played.
     */
    void setMovie(const QString& moviePath);
    /**
     * Enable or disable the scale and crop mode.
     *
     * @param mode enable scale and crop if true, disable otherwise.
     */
    void setScaleAndCropMode(bool mode);
    /**
     * Select an audio channel for the current movie.
     *
     * @param index the index of the audio channel.
     */
    void selectAudioChannel(int index);
    /**
     * Select a subtile for the current movie.
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
     * @param oldState the previous state of the movie player.
     */
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    /**
     * The playback of the current movie player is about to end.
     */
    void aboutToFinish();

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
     * Reset the movie player.
     *
     * All signals are disconnected. The movie player and controller are
     * deleted and newly created. The signals are connected again. This is
     * a workaround due to a number of playback issues.
     */
    void resetMoviePlayer();

    Phonon::MediaObject* moviePlayer;
    Phonon::MediaController* movieControler;
    Phonon::AudioOutput* audioOutput;
    QList<Phonon::SubtitleDescription> currentSubtiteDescriptions;
    QList<Phonon::AudioChannelDescription> currentAudioChannelDescriptions;
    bool fullWindow;
};

#endif
