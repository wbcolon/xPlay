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

#include "xMoviePlayerPhonon.h"
#include "xPlayerConfiguration.h"

#include <QEvent>
#include <QMouseEvent>
#include <QCheckBox>
#include <QTimer>
#include <cmath>

xMoviePlayerPhonon::xMoviePlayerPhonon(QWidget *parent):
        Phonon::VideoWidget(parent),
        moviePlayer(nullptr),
        movieController(nullptr),
        fullWindow(false) {
    // Setup the media player.
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    audioOutput->setMuted(false);
    resetMoviePlayer();
    // Connect to configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultAudioLanguage,
            this, &xMoviePlayerPhonon::updatedDefaultAudioLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultSubtitleLanguage,
            this, &xMoviePlayerPhonon::updatedDefaultSubtitleLanguage);
}

void xMoviePlayerPhonon::setMuted(bool mute) {
    audioOutput->setMuted(mute);
}

bool xMoviePlayerPhonon::isMuted() const {
    return audioOutput->isMuted();
}

void xMoviePlayerPhonon::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    audioOutput->setVolume(vol/100.0);
}

int xMoviePlayerPhonon::getVolume() const {
    return static_cast<int>(std::round(audioOutput->volume()*100.0));
}

void xMoviePlayerPhonon::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (moviePlayer->state() == Phonon::PlayingState) {
        moviePlayer->pause();
        emit currentState(State::PauseState);
    } else {
        moviePlayer->play();
        emit currentState(State::PlayingState);
    }
}

void xMoviePlayerPhonon::seek(qint64 position) {
    // Jump to position (in milliseconds) in the current track.
    moviePlayer->seek(position);
}

void xMoviePlayerPhonon::jump(qint64 delta) {
    // Jump to current position + delta (in milliseconds) in the current track.
    qint64 currentPosition = moviePlayer->currentTime() + delta;
    // Do not jump past the end (minus 100ms).
    qint64 currentLength = moviePlayer->totalTime() - 100;
    moviePlayer->seek(std::clamp(currentPosition, static_cast<qint64>(0), currentLength));
}

void xMoviePlayerPhonon::stop() {
    // Stop the media player.
    moviePlayer->stop();
    emit currentState(State::StopState);
}

void xMoviePlayerPhonon::setMovie(const QString& path, const QString& name, const QString& tag, const QString& directory) {
    resetMoviePlayer();
    moviePlayer->setCurrentSource(QUrl::fromLocalFile(path));
    moviePlayer->play();
    qDebug() << "xMoviePlayer: play: " << path;
    emit currentMovie(path, name, tag, directory);
    emit currentState(xMoviePlayerPhonon::PlayingState);
}

void xMoviePlayerPhonon::setMovieQueue(const QList<std::pair<QString,QString>>& queue, const QString& tag, const QString& directory) {
    movieQueue = queue;
    movieQueueTag = tag;
    movieQueueDirectory = directory;
}

void xMoviePlayerPhonon::clearMovieQueue() {
    movieQueue.clear();
}

void xMoviePlayerPhonon::setScaleAndCropMode(bool mode) {
    setScaleMode(mode ? Phonon::VideoWidget::ScaleAndCrop : Phonon::VideoWidget::FitInView);
}

void xMoviePlayerPhonon::toggleScaleAndCropMode() {
    auto mode = (scaleMode() == Phonon::VideoWidget::FitInView);
    setScaleAndCropMode(mode);
    emit scaleAndCropMode(mode);
}

void xMoviePlayerPhonon::availableAudioChannels() {
    currentAudioChannelDescriptions = movieController->availableAudioChannels();
    QStringList audioChannels;
    for (const auto& description : currentAudioChannelDescriptions) {
        auto audioChannel = description.name().toLower();
        updateAudioAndSubtitleDescription(audioChannel);
        if (!description.description().isEmpty()) {
            audioChannel += QString(" (%1)").arg(description.description().toLower());
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
    // Find default audio language.
    if (!movieDefaultAudioLanguage.isEmpty()) {
        // Phonon player has the movie default as index 0.
        auto defaultIndex = 0;
        for (auto i = 0; i < audioChannels.count(); ++i) {
            if (audioChannels[i].contains(movieDefaultAudioLanguage, Qt::CaseInsensitive)) {
                defaultIndex = i;
                break;
            }
        }
        emit currentAudioChannel(defaultIndex);
    }
}

void xMoviePlayerPhonon::availableSubtitles() {
    currentSubtitleDescriptions = movieController->availableSubtitles();
    QStringList subtitles;
    for (const auto& description : currentSubtitleDescriptions) {
        auto subtitle = description.name().toLower();
        updateAudioAndSubtitleDescription(subtitle);
        if (!description.description().isEmpty()) {
            subtitle += QString(" (%1)").arg(description.description().toLower());
        }
        subtitles.push_back(subtitle);
    }
    emit currentSubtitles(subtitles);
    if (currentSubtitleDescriptions.count() > 0) {
        // Find default subtitle.
        if (movieDefaultSubtitleLanguage.isEmpty()) {
            emit currentSubtitle(0);
        } else {
            // The default is disabled if we do not find the subtitle.
            auto defaultIndex = 0;
            for (auto i = 0; i < subtitles.count(); ++i) {
                if (subtitles[i].contains(movieDefaultSubtitleLanguage, Qt::CaseInsensitive)) {
                    defaultIndex = i;
                    break;
                }
            }
            emit currentSubtitle(defaultIndex);
        }
    }
    qDebug() << "xMoviePlayer: subtitle track descriptions: " << currentSubtitleDescriptions;
}

void xMoviePlayerPhonon::updateAudioAndSubtitleDescription(QString& description) {
    if (description.size() > 2) {
        // Avoid unwanted conversions.
        description.replace("de ", "german ");
        description.replace("en ", "english ");
        description.replace("fr ", "french ");
    } else {
        description.replace("de", "german");
        description.replace("en", "english");
        description.replace("fr", "french");
    }
}

void xMoviePlayerPhonon::availableChapters(int chapters) {
    qDebug() << "xMoviePlayer: number of chapters: " << chapters;
}

void xMoviePlayerPhonon::availableTitles(int titles) {
    qDebug() << "xMovie: number of titles: " << titles;
}

void xMoviePlayerPhonon::selectAudioChannel(int index) {
    if ((index >= 0) && (index < currentAudioChannelDescriptions.size())) {
        movieController->setCurrentAudioChannel(currentAudioChannelDescriptions[index]);
    }
}
void xMoviePlayerPhonon::selectSubtitle(int index) {
    if ((index >= 0) && (index < currentSubtitleDescriptions.size())) {
        movieController->setCurrentSubtitle(currentSubtitleDescriptions[index]);
    }
}
void xMoviePlayerPhonon::stateChanged(Phonon::State newState, Phonon::State oldState) {
    qDebug() << "xMoviePlayer: new: " << newState << ", old: " << oldState;
    if ((newState == Phonon::StoppedState) && (oldState == Phonon::PlayingState)) {
        emit currentState(xMoviePlayerPhonon::StopState);
    }
}

void xMoviePlayerPhonon::aboutToFinish() {
    if (movieQueue.isEmpty()) {
        qDebug() << "xMoviePlayer: aboutToFinish";
        // Go to stopping state. End full window mode.
        emit currentState(xMoviePlayerPhonon::StoppingState);
    }
}

void xMoviePlayerPhonon::closeToFinish(qint32 timeLeft) {
    if (movieQueue.isEmpty()) {
        qDebug() << "xMoviePlayer: closeToFinish: " << timeLeft;
        // Stop the media player.
        moviePlayer->stop();
        emit currentState(xMoviePlayerPhonon::StopState);
    } else {
        // Take next movie out of the queue.
        auto nextMovie = movieQueue.takeFirst();
        setMovie(nextMovie.first, nextMovie.second, movieQueueTag, movieQueueDirectory);
    }
}

void xMoviePlayerPhonon::updatedDefaultAudioLanguage() {
    movieDefaultAudioLanguage = xPlayerConfiguration::configuration()->getMovieDefaultAudioLanguage();
}

void xMoviePlayerPhonon::updatedDefaultSubtitleLanguage() {
    movieDefaultSubtitleLanguage = xPlayerConfiguration::configuration()->getMovieDefaultSubtitleLanguage();
}

void xMoviePlayerPhonon::keyPressEvent(QKeyEvent *keyEvent)
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
                emit currentState(xMoviePlayerPhonon::PauseState);
            } else {
                moviePlayer->play();
                emit currentState(xMoviePlayerPhonon::PlayingState);
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

void xMoviePlayerPhonon::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    emit toggleFullWindow();
    mouseEvent->accept();
}

void xMoviePlayerPhonon::mousePressEvent(QMouseEvent* mouseEvent)
{
    Phonon::VideoWidget::mousePressEvent(mouseEvent);
}

void xMoviePlayerPhonon::resetMoviePlayer() {
    if (movieController) {
        disconnect(movieController, &Phonon::MediaController::availableAudioChannelsChanged, this, &xMoviePlayerPhonon::availableAudioChannels);
        disconnect(movieController, &Phonon::MediaController::availableSubtitlesChanged, this, &xMoviePlayerPhonon::availableSubtitles);
        disconnect(movieController, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayerPhonon::availableTitles);
        disconnect(movieController, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayerPhonon::availableChapters);
        delete movieController;
    }
    if (moviePlayer) {
        disconnect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, this, &xMoviePlayerPhonon::currentMovieLength);
        disconnect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayerPhonon::currentMoviePlayed);
        disconnect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayerPhonon::stateChanged);
        disconnect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayerPhonon::aboutToFinish);
        disconnect(moviePlayer, &Phonon::MediaObject::prefinishMarkReached, this, &xMoviePlayerPhonon::closeToFinish);
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
    connect(movieController, &Phonon::MediaController::availableAudioChannelsChanged, this, &xMoviePlayerPhonon::availableAudioChannels);
    connect(movieController, &Phonon::MediaController::availableSubtitlesChanged, this, &xMoviePlayerPhonon::availableSubtitles);
    connect(movieController, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayerPhonon::availableTitles);
    connect(movieController, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayerPhonon::availableChapters);
    connect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, this, &xMoviePlayerPhonon::currentMovieLength);
    connect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayerPhonon::currentMoviePlayed);
    connect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayerPhonon::stateChanged);
    connect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayerPhonon::aboutToFinish);
    connect(moviePlayer, &Phonon::MediaObject::prefinishMarkReached, this, &xMoviePlayerPhonon::closeToFinish);
}