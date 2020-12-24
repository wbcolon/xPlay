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
#include <QCheckBox>
#include <QTimer>
#include <cmath>

xMoviePlayer::xMoviePlayer(QWidget *parent):
        Phonon::VideoWidget(parent),
        moviePlayer(nullptr),
        movieController(nullptr),
        fullWindow(false) {
    // Setup the media player.
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    resetMoviePlayer();
}

void xMoviePlayer::setMuted(bool mute) {
    audioOutput->setMuted(mute);
}

bool xMoviePlayer::isMuted() const {
    return audioOutput->isMuted();
}

void xMoviePlayer::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    audioOutput->setVolume(vol/100.0);
}

int xMoviePlayer::getVolume() const {
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

void xMoviePlayer::setMovie(const QString& path, const QString& name, const QString& tag, const QString& directory) {
    resetMoviePlayer();
    moviePlayer->setCurrentSource(QUrl::fromLocalFile(path));
    moviePlayer->play();
    qDebug() << "xMoviePlayer: play: " << path;
    emit currentMovie(path, name, tag, directory);
    emit currentState(xMoviePlayer::PlayingState);
}

void xMoviePlayer::setMovieQueue(const QList<std::pair<QString,QString>>& queue, const QString& tag, const QString& directory) {
    movieQueue = queue;
    movieQueueTag = tag;
    movieQueueDirectory = directory;
}

void xMoviePlayer::clearMovieQueue() {
    movieQueue.clear();
}

void xMoviePlayer::setScaleAndCropMode(bool mode) {
    setScaleMode(mode ? Phonon::VideoWidget::ScaleAndCrop : Phonon::VideoWidget::FitInView);
}

void xMoviePlayer::toggleScaleAndCropMode() {
    auto mode = (scaleMode() == Phonon::VideoWidget::FitInView);
    setScaleAndCropMode(mode);
    emit scaleAndCropMode(mode);
}

void xMoviePlayer::availableAudioChannels() {
    currentAudioChannelDescriptions = movieController->availableAudioChannels();
    QStringList audioChannels;
    for (const auto& description : currentAudioChannelDescriptions) {
        auto audioChannel = description.name();
        if (!description.description().isEmpty()) {
            audioChannel += QString(" (%1)").arg(description.description());
        }
        // Shorten a few audio descriptions.
        audioChannel.replace("Free Lossless Audio Codec (FLAC)", "FLAC");
        audioChannel.replace("PCM audio", "PCM");
        audioChannel.replace("Uncompressed ", "");
        audioChannels.push_back(audioChannel);
        qDebug() << "xMoviePlayer: audio channel: " << audioChannel;
    }
    emit currentAudioChannels(audioChannels);
    qDebug() << "xMoviePlayer: audio channel descriptions: " << currentAudioChannelDescriptions;
}

void xMoviePlayer::availableSubtitles() {
    currentSubtitleDescriptions = movieController->availableSubtitles();
    QStringList subtitles;
    for (const auto& description : currentSubtitleDescriptions) {
        auto subtitle = description.name();
        if (!description.description().isEmpty()) {
            subtitle += QString(" (%1)").arg(description.description());
        }
        subtitles.push_back(subtitle);
    }
    emit currentSubtitles(subtitles);
    // Disable subtitles on default.
    movieController->setCurrentSubtitle(currentSubtitleDescriptions[0]);
    qDebug() << "xMoviePlayer: subtitle track descriptions: " << currentSubtitleDescriptions;
}

void xMoviePlayer::availableChapters(int chapters) {
    qDebug() << "xMoviePlayer: number of chapters: " << chapters;
}

void xMoviePlayer::availableTitles(int titles) {
    qDebug() << "xMovie: number of titles: " << titles;
}

void xMoviePlayer::selectAudioChannel(int index) {
    if ((index >= 0) && (index < currentAudioChannelDescriptions.size())) {
        movieController->setCurrentAudioChannel(currentAudioChannelDescriptions[index]);
    }
}
void xMoviePlayer::selectSubtitle(int index) {
    if ((index >= 0) && (index < currentSubtitleDescriptions.size())) {
        movieController->setCurrentSubtitle(currentSubtitleDescriptions[index]);
    }
}
void xMoviePlayer::stateChanged(Phonon::State newState, Phonon::State oldState) {
    qDebug() << "xMoviePlayer: new: " << newState << ", old: " << oldState;
    if ((newState == Phonon::StoppedState) && (oldState == Phonon::PlayingState)) {
        emit currentState(xMoviePlayer::StopState);
    }
}

void xMoviePlayer::aboutToFinish() {
    if (movieQueue.isEmpty()) {
        qDebug() << "xMoviePlayer: aboutToFinish";
        // Go to stopping state. End full window mode.
        emit currentState(xMoviePlayer::StoppingState);
    }
}

void xMoviePlayer::closeToFinish(qint32 timeLeft) {
    if (movieQueue.isEmpty()) {
        qDebug() << "xMoviePlayer: closeToFinish: " << timeLeft;
        // Stop the media player.
        moviePlayer->stop();
        emit currentState(xMoviePlayer::StopState);
    } else {
        // Take next movie out of the queue.
        auto nextMovie = movieQueue.takeFirst();
        setMovie(nextMovie.first, nextMovie.second, movieQueueTag, movieQueueDirectory);
    }
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
        case Qt::Key_S: {
            toggleScaleAndCropMode();
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
    if (movieController) {
        disconnect(movieController, &Phonon::MediaController::availableAudioChannelsChanged, this, &xMoviePlayer::availableAudioChannels);
        disconnect(movieController, &Phonon::MediaController::availableSubtitlesChanged, this, &xMoviePlayer::availableSubtitles);
        disconnect(movieController, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayer::availableTitles);
        disconnect(movieController, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayer::availableChapters);
        delete movieController;
    }
    if (moviePlayer) {
        disconnect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, this, &xMoviePlayer::currentMovieLength);
        disconnect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayer::currentMoviePlayed);
        disconnect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayer::stateChanged);
        disconnect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayer::aboutToFinish);
        disconnect(moviePlayer, &Phonon::MediaObject::prefinishMarkReached, this, &xMoviePlayer::closeToFinish);
        delete moviePlayer;
    }
    // Setup the media player.
    moviePlayer = new Phonon::MediaObject(this);
    // Update each second
    moviePlayer->setTickInterval(500);
    moviePlayer->setPrefinishMark(1500);
    movieController = new Phonon::MediaController(moviePlayer);
    Phonon::createPath(moviePlayer, audioOutput);
    Phonon::createPath(moviePlayer, this);
    // Connect Phonon signals to out music player signals.
    connect(movieController, &Phonon::MediaController::availableAudioChannelsChanged, this, &xMoviePlayer::availableAudioChannels);
    connect(movieController, &Phonon::MediaController::availableSubtitlesChanged, this, &xMoviePlayer::availableSubtitles);
    connect(movieController, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayer::availableTitles);
    connect(movieController, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayer::availableChapters);
    connect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, this, &xMoviePlayer::currentMovieLength);
    connect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayer::currentMoviePlayed);
    connect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayer::stateChanged);
    connect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayer::aboutToFinish);
    connect(moviePlayer, &Phonon::MediaObject::prefinishMarkReached, this, &xMoviePlayer::closeToFinish);
}