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
    void currentAudioChannels(const QStringList& audioChannels);
    void currentSubtitles(const QStringList& subtitles);
    void currentChapters(int chapters);
    void currentTitles(int titles);
    void currentVolume(int vol);

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
    void playPause();
    void seek(qint64 position);
    void jump(qint64 delta);
    void stop();

    void setVolume(int vol);
    void setMovie(const QString& moviePath);

    void selectAudioChannel(int index);
    void selectSubtitle(int index);

private slots:
    void availableChapters(int chapters);
    void availableTitles(int titles);
    void availableAudioChannels();
    void availableSubtitles();
    void stateChanged(Phonon::State newState, Phonon::State oldState);
    void aboutToFinish();

protected:
    void keyPressEvent(QKeyEvent* keyEvent) override;
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) override;
    void mousePressEvent(QMouseEvent* mouseEvent) override;

private:
    void resetMoviePlayer();

    Phonon::MediaObject* moviePlayer;
    Phonon::MediaController* movieControler;
    Phonon::AudioOutput* audioOutput;
    QList<Phonon::SubtitleDescription> currentSubtiteDescriptions;
    QList<Phonon::AudioChannelDescription> currentAudioChannelDescriptions;
    bool fullWindow;
};

#endif
