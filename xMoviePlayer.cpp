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

#include "xMoviePlayer.h"

#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <cmath>

xMoviePlayer::xMoviePlayer(QWidget *parent):
        Phonon::VideoWidget(parent),
        moviePlayer(nullptr),
        movieControler(nullptr),
        fullWindow(false) {
    // Configure Video Output
    //setScaleMode(Phonon::VideoWidget::ScaleAndCrop);
    // Setup the media player.
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    resetMoviePlayer();
}

void xMoviePlayer::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    audioOutput->setVolume(vol/100.0);
}

int xMoviePlayer::getVolume() {
    return static_cast<int>(std::round(audioOutput->volume()*100.0));
}

void xMoviePlayer::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (moviePlayer->state() == Phonon::PlayingState) {
        moviePlayer->pause();
        emit currentState(State::PauseState);
    } else {
        moviePlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMoviePlayer::seek(qint64 position) {
    // Jump to position (in milliseconds) in the current track.
    moviePlayer->seek(position);
}

void xMoviePlayer::jump(qint64 delta) {
    // Jump to current position + delta (in milliseconds) in the current track.
    qint64 currentPosition = moviePlayer->currentTime() + delta;
    // Do not jump past the end (minus 100ms).
    qint64 currentLength = moviePlayer->totalTime() - 100;
    moviePlayer->seek(std::clamp(currentPosition, static_cast<qint64>(0), currentLength));
}

void xMoviePlayer::stop() {
    // Stop the media player.
    moviePlayer->stop();
    emit currentState(State::StopState);
}

void xMoviePlayer::setMovie(const QString& moviePath) {
    qDebug() << "xMoviePlayer: play: " << moviePath;
    resetMoviePlayer();
    moviePlayer->setCurrentSource(QUrl::fromLocalFile(moviePath));
    moviePlayer->play();
    emit currentState(xMoviePlayer::PlayingState);
}

void xMoviePlayer::availableAudioChannels() {
    currentAudioChannelDescriptions = movieControler->availableAudioChannels();
    QStringList audioChannels;
    for (const auto& description : currentAudioChannelDescriptions) {
        auto audioChannel = description.name();
        if (!description.description().isEmpty()) {
            audioChannel += QString(" (%1)").arg(description.description());
        }
        audioChannels.push_back(audioChannel);
        qDebug() << "xMoviePlayer: audio channel: " << audioChannel;
    }
    emit currentAudioChannels(audioChannels);
    qDebug() << "xMoviePlayer: audio channel descriptions: " << currentAudioChannelDescriptions;
}

void xMoviePlayer::availableSubtitles() {
    currentSubtiteDescriptions = movieControler->availableSubtitles();
    QStringList subtitles;
    for (const auto& description : currentSubtiteDescriptions) {
        auto subtitle = description.name();
        if (!description.description().isEmpty()) {
            subtitle += QString(" (%1)").arg(description.description());
        }
        subtitles.push_back(subtitle);
    }
    emit currentSubtitles(subtitles);
    // Disable subtitles on default.
    movieControler->setCurrentSubtitle(currentSubtiteDescriptions[0]);
    qDebug() << "xMoviePlayer: subtitle track descriptions: " << currentSubtiteDescriptions;
}

void xMoviePlayer::availableChapters(int chapters) {
    qDebug() << "xMoviePlayer: number of chapters: " << chapters;
}

void xMoviePlayer::availableTitles(int titles) {
    qDebug() << "xMovie: number of titles: " << titles;
}

void xMoviePlayer::selectAudioChannel(int index) {
    if ((index >= 0) && (index < currentAudioChannelDescriptions.size())) {
        movieControler->setCurrentAudioChannel(currentAudioChannelDescriptions[index]);
    }
}
void xMoviePlayer::selectSubtitle(int index) {
    if ((index >= 0) && (index < currentSubtiteDescriptions.size())) {
        movieControler->setCurrentSubtitle(currentSubtiteDescriptions[index]);
    }
}
void xMoviePlayer::stateChanged(Phonon::State newState, Phonon::State oldState) {
    qDebug() << "xMoviePlayer: new: " << newState << ", old: " << oldState;
    if ((newState == Phonon::StoppedState) && (oldState == Phonon::PlayingState)) {
        emit currentState(xMoviePlayer::StopState);
    }
}

void xMoviePlayer::aboutToFinish() {
    qDebug() << "xMoviePlayer: aboutToFinish";
    // Go to stopping state. End full window mode.
    emit currentState(xMoviePlayer::StoppingState);
    // Stop the player after 1sec.
    QTimer::singleShot(1000, [=] { moviePlayer->stop(); emit currentState(xMoviePlayer::StopState); });
}

void xMoviePlayer::keyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
        case Qt::Key_Escape: {
            toggleFullWindow();
        } break;
        case Qt::Key_F10: {
            setFullScreen(!isFullScreen());
        } break;
        case Qt::Key_Right: {
            // Seek +1 min
            jump(60000);
        } break;
        case Qt::Key_Left: {
            // Seek -1 min
            jump(-60000);
        } break;
        case Qt::Key_Up: {
            // Increase Volume by 1.
            setVolume(getVolume()+1);
            emit currentVolume(getVolume());
        } break;
        case Qt::Key_Down: {
            // Decrease Volume by 1.
            setVolume(getVolume()-1);
            emit currentVolume(getVolume());
        } break;
        case Qt::Key_Space: {
            if (moviePlayer->state() == Phonon::PlayingState) {
                moviePlayer->pause();
                emit currentState(xMoviePlayer::PauseState);
            } else {
                moviePlayer->play();
                emit currentState(xMoviePlayer::PlayingState);
            }
        } break;
        default: {
            Phonon::VideoWidget::keyPressEvent(keyEvent);
            // Do not accept key event.
            return;
        }
    }
    keyEvent->accept();
}

void xMoviePlayer::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    emit toggleFullWindow();
    mouseEvent->accept();
}

void xMoviePlayer::mousePressEvent(QMouseEvent* mouseEvent)
{
    Phonon::VideoWidget::mousePressEvent(mouseEvent);
}

void xMoviePlayer::resetMoviePlayer() {
    if (movieControler) {
        disconnect(movieControler, &Phonon::MediaController::availableAudioChannelsChanged, this, &xMoviePlayer::availableAudioChannels);
        disconnect(movieControler, &Phonon::MediaController::availableSubtitlesChanged, this, &xMoviePlayer::availableSubtitles);
        disconnect(movieControler, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayer::availableTitles);
        disconnect(movieControler, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayer::availableChapters);
        delete movieControler;
    }
    if (moviePlayer) {
        disconnect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, this, &xMoviePlayer::currentMovieLength);
        disconnect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayer::currentMoviePlayed);
        disconnect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayer::stateChanged);
        disconnect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayer::aboutToFinish);
        delete moviePlayer;
    }
    // Setup the media player.
    moviePlayer = new Phonon::MediaObject(this);
    // Update each second
    moviePlayer->setTickInterval(500);
    movieControler = new Phonon::MediaController(moviePlayer);
    Phonon::createPath(moviePlayer, audioOutput);
    Phonon::createPath(moviePlayer, this);
    // Connect Phono signals to out music player signals.
    connect(movieControler, &Phonon::MediaController::availableAudioChannelsChanged, this, &xMoviePlayer::availableAudioChannels);
    connect(movieControler, &Phonon::MediaController::availableSubtitlesChanged, this, &xMoviePlayer::availableSubtitles);
    connect(movieControler, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayer::availableTitles);
    connect(movieControler, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayer::availableChapters);
    connect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, this, &xMoviePlayer::currentMovieLength);
    connect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayer::currentMoviePlayed);
    connect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayer::stateChanged);
    connect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayer::aboutToFinish);
}