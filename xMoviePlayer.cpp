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
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include "xPlayerDatabase.h"

#include <QAudioOutput>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMediaMetaData>
#include <QEvent>
#include <QMouseEvent>
#include <QCheckBox>
#include <QTimer>
#include <QDebug>
#include <cmath>

#include <vlc/vlc.h>

std::list<std::pair<QString,xMoviePlayer::AspectRatio>> xMoviePlayer::supportedAspectRatio() {
    return {
        {"Keep", xMoviePlayer::RatioKeep},
        {"Fit", xMoviePlayer::RatioIgnore},
        {"Expand", xMoviePlayer::RatioExpanding}
    };
}

xMoviePlayer::xMoviePlayer(QWidget *parent):
        QVideoWidget(parent),
        moviePlayerState(xMoviePlayer::StopState),
        movieMediaLength(0),
        movieMediaChapter(0),
        movieMediaFullWindow(false),
        movieTickConnected(false),
        movieCurrentPosition(0),
        movieCurrentPlayed(0),
        moviePlayed(-1),
        movieCurrentSkip(false),
        moviePlayedRecorded(false) {

    // Setup pulseAudio controls.
    pulseAudioControls = xPlayerPulseAudioControls::controls();
    movieFile = new xMovieFile(this);
    moviePlayer = new QMediaPlayer();
    // Configure the audio device.
    audioOutput = new QAudioOutput();
    audioOutput->setMuted(false);
    audioOutput->setVolume(1.0);
    // Connect Video and Audio to movie player.
    moviePlayer->setVideoOutput(this);
    moviePlayer->setAudioOutput(audioOutput);

    // Setup the media player.
    connect(moviePlayer, &QMediaPlayer::positionChanged, this, &xMoviePlayer::updatedPosition);
    connect(moviePlayer, &QMediaPlayer::durationChanged, [=](qint64 totalTime) {
        movieMediaLength = totalTime;
        qDebug() << "xMoviePlayer: totalTime: " << totalTime;
        emit currentMovieLength(totalTime);
    });
    connect(moviePlayer, &QMediaPlayer::metaDataChanged, this, &xMoviePlayer::availableAudioChannels);
    connect(moviePlayer, &QMediaPlayer::metaDataChanged, this, &xMoviePlayer::availableSubtitles);
    connect(moviePlayer, &QMediaPlayer::mediaStatusChanged, this, &xMoviePlayer::updatedMediaStatus);

    // Connect to configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultAudioLanguage,
        this, &xMoviePlayer::updatedDefaultAudioLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultSubtitleLanguage,
        this, &xMoviePlayer::updatedDefaultSubtitleLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieAudioDeviceId,
        this, &xMoviePlayer::updatedAudioDeviceId);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMoviePlayed, [=]() {
        moviePlayed = xPlayerConfiguration::configuration()->getDatabaseMoviePlayed();
    });
}

xMoviePlayer::~xMoviePlayer() noexcept = default;

void xMoviePlayer::setFullWindowMode(bool enabled) {
    if (movieMediaFullWindow != enabled) {
        movieMediaFullWindow = enabled;
        emit fullWindowMode(movieMediaFullWindow);
    }
}

bool xMoviePlayer::getFullWindowMode() const {
    return movieMediaFullWindow;
}

void xMoviePlayer::setMuted(bool mute) {
    // Mute/unmute the stream and the pulseaudio sink
    audioOutput->setMuted(mute);
    pulseAudioControls->setMuted(mute);
}

bool xMoviePlayer::isMuted() const {
    return audioOutput->isMuted();
}

void xMoviePlayer::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    pulseAudioControls->setVolume(vol);
}

int xMoviePlayer::getVolume() const {
    return pulseAudioControls->getVolume();
}

void xMoviePlayer::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (moviePlayer->playbackState() == QMediaPlayer::PlaybackState::PlayingState) {
        moviePlayer->pause();
        moviePlayerState = State::PauseState;
    } else {
        moviePlayer->play();
        moviePlayerState = State::PlayingState;
    }
    emit currentState(moviePlayerState);
}

void xMoviePlayer::playChapter(int chapter) {
    if ((chapter >= 0) && (chapter < movieMediaChapterBegin.count())) {
        qDebug() << "xMoviePlayer: playChapter:: " << chapter;
        seek(movieMediaChapterBegin[chapter]);
        moviePlayer->play();
        movieCurrentSkip = true;
        emit currentState(moviePlayerState = State::PlayingState);
    }
    updateCurrentChapter();
}

void xMoviePlayer::previousChapter() {
    // Indicate skip in order to correctly current position.
    if (movieMediaChapter > 0) {
        --movieMediaChapter;
        qDebug() << "xMoviePlayer: previousChapter:: " << movieMediaChapter;
        moviePlayer->setPosition(movieMediaChapterBegin[movieMediaChapter]);
        moviePlayer->play();
        movieCurrentSkip = true;
        emit currentState(moviePlayerState = State::PlayingState);
    }
    updateCurrentChapter();
}

void xMoviePlayer::nextChapter() {
    // Indicate skip to correctly current position.
    if (movieMediaChapter < movieMediaChapterBegin.size() - 1 ) {
        ++movieMediaChapter;
        qDebug() << "xMoviePlayer: nextChapter:: " << movieMediaChapter;
        moviePlayer->setPosition(movieMediaChapterBegin[movieMediaChapter]);
        moviePlayer->play();
        movieCurrentSkip = true;
        emit currentState(moviePlayerState = State::PlayingState);
    }
    updateCurrentChapter();
}

void xMoviePlayer::seek(qint64 position) {
    movieCurrentSkip = true;
    moviePlayer->setPosition(position);
    updateCurrentChapter();
}

void xMoviePlayer::jump(qint64 delta) {
    // Jump to current position + delta (in milliseconds) in the current track.
    qint64 currentPosition = moviePlayer->position() + delta;
    // Do not jump past the end (minus 100ms).
    qint64 currentLength = moviePlayer->duration() - 100;
    movieCurrentSkip = true;
    moviePlayer->play();
    moviePlayer->setPosition(std::clamp(currentPosition, static_cast<qint64>(0), currentLength));
    emit currentState(moviePlayerState = State::PlayingState);
    updateCurrentChapter();
}

void xMoviePlayer::stop() {
    qDebug() << "xMoviePlayer: stop";
    // Simulate the stop by pausing and seeking to the zero position.
    moviePlayer->stop();
    // Reset the current position.
    movieCurrentPosition = 0;
    // Update states.
    emit currentState(moviePlayerState = State::StopState);
    emit currentMoviePlayed(0);
    emit currentChapter(0);
}

void xMoviePlayer::setMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory) {
    QString filePath = QString::fromStdString( path.string() );
    // Reset movie played.
    movieCurrentPosition = 0;
    movieCurrentPlayed = 0;
    movieCurrentSkip = false;
    moviePlayedRecorded = false;
    // Update current.
    movieCurrent = std::make_pair(path, name);
    movieCurrentTag = tag;
    movieCurrentDirectory = directory;
    // Analyze movie file.
    movieFile->analyze(filePath);
    movieMediaChapterBegin = movieFile->getChapterBegin();
    // Set current movie.
    moviePlayer->setSource(QUrl::fromLocalFile(filePath));
    moviePlayer->play();
    qDebug() << "xMoviePlayer: play: " << filePath;
    emit currentMovie(path, name, tag, directory);
    emit currentState(moviePlayerState = xMoviePlayer::PlayingState);
    availableChapters(movieMediaChapterBegin.size());
}

void xMoviePlayer::setMovieQueue(const QList<std::pair<std::filesystem::path,QString>>& queue) {
    movieQueue = queue;
}

void xMoviePlayer::clearMovieQueue() {
    movieQueue.clear();
}

void xMoviePlayer::setAspectRatio(xMoviePlayer::AspectRatio aspectRatio) {
    setAspectRatioMode(static_cast<Qt::AspectRatioMode>(aspectRatio));
}

void xMoviePlayer::availableAudioChannels() {
    QStringList audioChannels;
    // Process audio channels
    audioChannelsMetaData = moviePlayer->audioTracks();
    for (auto& audioChannelMetaData : audioChannelsMetaData) {
        auto audioChannelDescription = audioChannelMetaData.stringValue(QMediaMetaData::Language);
        if (audioChannelDescription.isEmpty()) {
            audioChannelDescription = "Unknown";
        }
        auto audioCodec = audioChannelMetaData.stringValue(QMediaMetaData::AudioCodec);
        if (!audioCodec.isEmpty()) {
            audioChannelDescription.append(QString(" (%1)").arg(audioCodec));
        }
        audioChannels.push_back(audioChannelDescription);
    }
    // Determine default audio track.
    int defaultAudioIndex = 0; // Choose the first audio track if the default does not match.
    for (const auto& audioChannel : audioChannels ) {
        if (audioChannel.contains(movieDefaultAudioLanguage, Qt::CaseInsensitive)) {
            break;
        }
        ++defaultAudioIndex;
    }
    // Do we have a default audio channel?
    if (defaultAudioIndex >= audioChannels.count()) {
        defaultAudioIndex = 0;
    }

    emit currentAudioChannels(audioChannels, QStringList());
    emit currentAudioChannel(defaultAudioIndex);
    selectAudioChannel(defaultAudioIndex);
}

void xMoviePlayer::availableSubtitles() {
    QStringList subtitles;
    // Process subtitles.
    subtitlesMetaData = moviePlayer->subtitleTracks();
    for (auto& subtitleMetaData : subtitlesMetaData) {
        if (subtitleMetaData.keys().contains(QMediaMetaData::Language)) {
            subtitles.push_back(subtitleMetaData.stringValue(QMediaMetaData::Language));
        } else {
            subtitles.push_back("Unknown");
        }
    }
    if (!subtitles.isEmpty()) {
        subtitles.push_back("Disable");
    }
    // Subtitles are currently disabled. Segmentation fault for a number of movies.
    subtitles.clear(); // Remove once subtitles work properly.
    emit currentSubtitles(subtitles);
}

void xMoviePlayer::availableChapters(int chapters) {
    qDebug() << "xMoviePlayer: number of chapters: " << chapters;
    currentChapterDescriptions.clear();
    for (int i = 1; i <= chapters; ++i) {
        currentChapterDescriptions.push_back(QString("Chapter %1").arg(i));
    }
    emit currentChapters(currentChapterDescriptions);
}

void xMoviePlayer::selectAudioChannel(int index) {
    if ((index >= 0) && (index < audioChannelsMetaData.size())) {
        qDebug() << "xMovie: selectAudioChannel:: " << index;
        moviePlayer->setActiveAudioTrack(index);
    }
}

void xMoviePlayer::selectSubtitle(int index) {
    qDebug() << "xMovie: selectSubtitle:: " << index;
    if (subtitlesMetaData.count() > 0) {
        if ((index > 0) && (index < subtitlesMetaData.count())) {
            moviePlayer->setActiveSubtitleTrack(index);
        }
        else {
            /* Disable subtitles */
            moviePlayer->setActiveSubtitleTrack(-1);
        }
    }
}

void xMoviePlayer::updatedPosition(qint64 movieMediaPos) {
    // The phonon-kde and QMediaPlayer may produce way to many tick/position updates. Ignore most of them.
    if (std::abs(movieMediaPos - movieCurrentPosition) < xPlayer::MovieTickDeltaIgnore) {
        return;
    }
    qDebug() << "xMovie: updatedPosition:: " << movieMediaPos << ", currentPlayed: " << movieCurrentPlayed << ", currentPos: " << movieCurrentPosition;

    // Update played time and current position.
    if (movieCurrentSkip) {
        // Update the current position if we skipped.
        movieCurrentPosition = movieMediaPos;
        movieCurrentSkip = false;
    }
    // Do we need to check for database update.
    if (!moviePlayedRecorded) {
        if (movieCurrentPosition <= movieMediaPos) {
            movieCurrentPlayed += (movieMediaPos - movieCurrentPosition);
            // Determine if we need to update the database.
            auto update = false;
            if ((movieMediaLength - 10000) <= moviePlayed) {
                // Update if we are close to the end (within 10 seconds towards the end)
                update = (movieMediaPos >= movieMediaLength - 10000) && (movieMediaPos <= movieCurrentPlayed);
            } else {
                // Update if we played enough of the song.
                update = (movieCurrentPlayed >= moviePlayed);
            }
            if (update) {
                // Update database.
                auto name = movieCurrent.second;
                auto result = xPlayerDatabase::database()->updateMovieFile(movieCurrentTag, movieCurrentDirectory, name);

                qDebug() << "xMovie: updatedPosition: db: " << movieCurrentTag << "," << movieCurrentDirectory << "," << name;
                qDebug() << "xMovie: updatedPosition: db: " << result;
                if (result.second > 0) {
                    // Update database overlay.
                    emit updatePlayedMovie(movieCurrentTag, movieCurrentDirectory,
                                           name, result.first, result.second);
                    moviePlayedRecorded = true;
                }
            }
        } else {
            qCritical() << "xMoviePlayer::updatedPosition: illegal movie positions: "
                << movieCurrentPosition << "," << movieMediaPos;
        }
    }
    emit currentMoviePlayed(movieCurrentPosition = movieMediaPos);
    updateCurrentChapter();
}

void xMoviePlayer::stateChanged(QMediaPlayer::PlaybackState newState) {
    qDebug() << "xMoviePlayer: new: " << newState;
}

void xMoviePlayer::aboutToFinish() {
    if (movieQueue.isEmpty()) {
        qDebug() << "xMoviePlayer: aboutToFinish";
        // Go to the stopping state. End full window mode.
        emit currentState(moviePlayerState = xMoviePlayer::StoppingState);
    }
}

void xMoviePlayer::updatedMediaStatus(QMediaPlayer::MediaStatus status) {
    qDebug() << "xMoviePlayer: media status: " << status;
    if (status == QMediaPlayer::EndOfMedia) {
        if (movieQueue.isEmpty()) {
            emit currentState(moviePlayerState = xMoviePlayer::StoppingState);
            stop();
        }
        else {
            // Take the next movie out of the queue and directly play it.
            auto nextMovie = movieQueue.takeFirst();
            setMovie(nextMovie.first, nextMovie.second, movieCurrentTag, movieCurrentDirectory);
        }
    }
}

void xMoviePlayer::updatedDefaultAudioLanguage() {
    movieDefaultAudioLanguage = xPlayerConfiguration::configuration()->getMovieDefaultAudioLanguage();
}

void xMoviePlayer::updatedDefaultSubtitleLanguage() {
    movieDefaultSubtitleLanguage = xPlayerConfiguration::configuration()->getMovieDefaultSubtitleLanguage();
}

void xMoviePlayer::updatedAudioDeviceId() {
    auto audioDeviceId = xPlayerConfiguration::configuration()->getMovieAudioDeviceId();
    for (const auto& audioDevice : QMediaDevices::audioOutputs()) {
        if (audioDevice.id() == audioDeviceId) {
            qDebug() << "Selecting Audio Device: " << audioDevice.description();
            audioOutput->setDevice(audioDevice);
            break;
        }
    }
}

void xMoviePlayer::updateCurrentChapter() {
    if (movieMediaChapterBegin.count() > 0) {
        qint64 currentPosition = moviePlayer->position();
        int chapter = static_cast<int>(movieMediaChapterBegin.count())-1;
        while (chapter >= 0) {
            if (movieMediaChapterBegin[chapter] < currentPosition) {
                break;
            }
            --chapter;
        }
        if (movieMediaChapter != chapter) {
            // Update current chapter.
            movieMediaChapter = chapter;
            // Emit new chapter index.
            emit currentChapter(chapter);
        }
    }
}

void xMoviePlayer::keyPressEvent(QKeyEvent *keyEvent) {
    switch (keyEvent->key()) {
        case Qt::Key_Escape: {
            setFullWindowMode(false);
        } break;
        case Qt::Key_Right: {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                nextChapter();
            } else {
                // Seek +1 min
                jump(xPlayer::MovieForwardRewindDelta);
            }
        } break;
        case Qt::Key_Left: {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                previousChapter();
            } else {
                // Seek -1 min
                jump(-xPlayer::MovieForwardRewindDelta);
            }
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
            if (moviePlayer->playbackState() == QMediaPlayer::PlaybackState::PlayingState) {
                moviePlayer->pause();
                emit currentState(moviePlayerState = xMoviePlayer::PauseState);
            } else {
                moviePlayer->play();
                emit currentState(moviePlayerState = xMoviePlayer::PlayingState);
            }
        } break;
        default: {
            QVideoWidget::keyPressEvent(keyEvent);
            // Do not accept key event.
            return;
        }
    }
    keyEvent->accept();
}

void xMoviePlayer::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
    setFullWindowMode(!movieMediaFullWindow);
    mouseEvent->accept();
}

void xMoviePlayer::mousePressEvent(QMouseEvent* mouseEvent) {
    QVideoWidget::mousePressEvent(mouseEvent);
}
