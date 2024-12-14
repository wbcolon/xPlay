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
#include "xPlayerConfiguration.h"
#include "xPlayerDatabase.h"

#include <QEvent>
#include <QMouseEvent>
#include <QCheckBox>
#include <QTimer>
#include <QDebug>
#include <cmath>

#include <vlc/vlc.h>

std::list<std::pair<QString,xMoviePlayer::AspectRatio>> xMoviePlayer::supportedAspectRatio() {
    return {
        {"auto", xMoviePlayer::RatioAuto},
        {"fit", xMoviePlayer::RatioFitWidget},
        {"16 : 9", xMoviePlayer::Ratio16x9},
        {"4 : 3", xMoviePlayer::Ratio4x3}
    };
}

xMoviePlayer::xMoviePlayer(QWidget *parent):
        Phonon::VideoWidget(parent),
        moviePlayerState(xMoviePlayer::StopState),
        movieMediaLength(0),
        movieMediaChapter(0),
        movieMediaDeinterlaceMode(false),
        movieMediaCropAspectRatio(),
        movieMediaFullWindow(false),
        movieCurrentPosition(0),
        movieCurrentPlayed(0),
        moviePlayed(-1),
        movieCurrentSkip(false),
        moviePlayedRecorded(false) {

    // Setup pulseAudio controls.
    pulseAudioControls = xPlayerPulseAudioControls::controls();
    movieFile = new xMovieFile(this);
    // Setup the media player.
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    audioOutput->setMuted(false);
    audioOutput->setVolume(1.0);
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
    connect(movieController, &Phonon::MediaController::availableChaptersChanged, this, &xMoviePlayer::availableChapters);
    // Chapters and Titles are not properly updated by Phonon. Therefor we are using ffprobe to scan for chapters.
    // connect(movieController, &Phonon::MediaController::availableTitlesChanged, this, &xMoviePlayer::availableTitles);
    // connect(movieController, &Phonon::MediaController::chapterChanged, this, &xMoviePlayer::updatedChapter);
    connect(moviePlayer, &Phonon::MediaObject::totalTimeChanged, [=](qint64 totalTime) {
        movieMediaLength = totalTime;
        qDebug() << "xMoviePlayer: totalTime: " << totalTime;
        emit currentMovieLength(totalTime);
    });
    connect(moviePlayer, &Phonon::MediaObject::tick, this, &xMoviePlayer::updatedTick);
    connect(moviePlayer, &Phonon::MediaObject::stateChanged, this, &xMoviePlayer::stateChanged);
    connect(moviePlayer, &Phonon::MediaObject::aboutToFinish, this, &xMoviePlayer::aboutToFinish);
    connect(moviePlayer, &Phonon::MediaObject::prefinishMarkReached, this, &xMoviePlayer::closeToFinish);
    // Connect to configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultAudioLanguage,
        this, &xMoviePlayer::updatedDefaultAudioLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultSubtitleLanguage,
        this, &xMoviePlayer::updatedDefaultSubtitleLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMoviePlayed, [=]() {
        moviePlayed = xPlayerConfiguration::configuration()->getDatabaseMoviePlayed();
    });
}

xMoviePlayer::~xMoviePlayer() noexcept {
}

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
    if (moviePlayer->state() == Phonon::PlayingState) {
        moviePlayer->pause();
        moviePlayerState = State::PauseState;
    } else {
        moviePlayer->play();
        moviePlayerState = State::PlayingState;
    }
    emit currentState(moviePlayerState);
}

void xMoviePlayer::playChapter(int chapter) {
    if ((chapter >= 0) && (chapter < currentChapterDescriptions.count())) {
        qDebug() << "xMoviePlayer: playChapter:: " << chapter;
        movieController->setCurrentChapter( chapter );
        emit currentState(moviePlayerState = State::PlayingState);
    }
    updateCurrentChapter();
}

void xMoviePlayer::previousChapter() {
    // Indicate skip in order to correctly current position.
    int chapter = movieController->currentChapter();
    movieCurrentSkip = true;
    if (chapter > 0) {
        qDebug() << "xMoviePlayer: previousChapter:: " << chapter;
        movieController->setCurrentChapter(chapter - 1);
        emit currentState(moviePlayerState = State::PlayingState);
    }
    updateCurrentChapter();
}

void xMoviePlayer::nextChapter() {
    // Indicate skip in order to correctly current position.
    movieCurrentSkip = true;
    int chapter = movieController->currentChapter();
    if (chapter < movieController->availableChapters() - 1 ) {
        qDebug() << "xMoviePlayer: nextChapter:: " << chapter;
        movieController->setCurrentChapter( chapter + 1 );
        emit currentState(moviePlayerState = State::PlayingState);
    }
    updateCurrentChapter();
}

void xMoviePlayer::seek(qint64 position) {
    // Check if VLC is still playing.
    moviePlayer->seek(position);
    movieCurrentSkip = true;
    updateCurrentChapter();
}

void xMoviePlayer::jump(qint64 delta) {
    // Jump to current position + delta (in milliseconds) in the current track.
    qint64 currentPosition = moviePlayer->currentTime() + delta;
    // Do not jump past the end (minus 100ms).
    qint64 currentLength = moviePlayer->totalTime() - 100;
    moviePlayer->seek(std::clamp(currentPosition, static_cast<qint64>(0), currentLength));
    movieCurrentSkip = true;
    updateCurrentChapter();
}

void xMoviePlayer::stop() {
    qDebug() << "xMoviePlayer: stop";
    // Stop the media player.
    moviePlayer->stop();
    seek(0);
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
    currentChapterBegin = movieFile->getChapterBegin();
    // Set current movie.
    moviePlayer->setCurrentSource(QUrl::fromLocalFile(filePath));
    moviePlayer->play();
    qDebug() << "xMoviePlayer: play: " << filePath;
    emit currentMovie(path, name, tag, directory);
    emit currentState(moviePlayerState = xMoviePlayer::PlayingState);
}

void xMoviePlayer::setMovieQueue(const QList<std::pair<std::filesystem::path,QString>>& queue) {
    movieQueue = queue;
}

void xMoviePlayer::clearMovieQueue() {
    movieQueue.clear();
}

void xMoviePlayer::setAudioCompressionMode(bool mode) {
    /* VLC */
    if (mode != movieMediaAudioCompressionMode) {
        // Reset the vlc player.
    }
}

bool xMoviePlayer::audioCompressionMode() const {
    return movieMediaAudioCompressionMode;
}

void xMoviePlayer::setDeinterlaceMode(bool mode) {
    movieMediaDeinterlaceMode = mode;
    /* VLC */
}

bool xMoviePlayer::deinterlaceMode() const {
    return movieMediaDeinterlaceMode;
}

void xMoviePlayer::setCropAspectRatio(xMoviePlayer::AspectRatio aspectRatio) {
    setAspectRatio(static_cast<Phonon::VideoWidget::AspectRatio>(aspectRatio));
}

QString xMoviePlayer::cropAspectRatio() const {
    return movieMediaCropAspectRatio;
}

void xMoviePlayer::availableAudioChannels() {
    QStringList audioChannels;
    currentAudioChannelDescriptions = movieController->availableAudioChannels();
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
    // Determine default audio track.
    int defaultAudioIndex = 0; // First phonon audio track is disabled.
    for (const auto& audioChannel : audioChannels ) {
        if (audioChannel.contains(movieDefaultAudioLanguage, Qt::CaseInsensitive)) {
            break;
        }
        ++defaultAudioIndex;
    }
    // Do we have a default audio channel?
    if (defaultAudioIndex >= audioChannels.count()) {
        // Phonon uses index 0 for disabled.
        defaultAudioIndex = 1;
    }
    emit currentAudioChannels(audioChannels, QStringList() );
    emit currentAudioChannel(defaultAudioIndex);
    selectAudioChannel(defaultAudioIndex);
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
    if (currentSubtitleDescriptions.count() > 0) {
        // Disable subtitles on default.
        movieController->setCurrentSubtitle(currentSubtitleDescriptions[0]);
    }
    qDebug() << "xMoviePlayer: subtitle track descriptions: " << currentSubtitleDescriptions;
}

void xMoviePlayer::availableChapters(int chapters) {
    qDebug() << "xMoviePlayer: number of chapters: " << chapters;
    currentChapterDescriptions.clear();
    for (int i = 1; i <= chapters; ++i) {
        currentChapterDescriptions.push_back( QString("Chapter %1").arg(i) );
    }
    emit currentChapters( currentChapterDescriptions );
}

void xMoviePlayer::availableTitles(int titles) {
    qDebug() << "xMovie: number of titles: " << titles;
}

void xMoviePlayer::selectAudioChannel(int index) {
    if ((index >= 0) && (index < currentAudioChannelDescriptions.size())) {
        qDebug() << "xMovie: selectAudioChannel:: " << index;
        movieController->setCurrentAudioChannel(currentAudioChannelDescriptions[index]);
        fixAudioIssue();
    }
}

void xMoviePlayer::selectSubtitle(int index) {
    if ((index >= 0) && (index < currentSubtitleDescriptions.size())) {
        qDebug() << "xMovie: selectSubtitle:: " << index;
        movieController->setCurrentSubtitle(currentSubtitleDescriptions[index]);
        fixAudioIssue();
    }
}

void xMoviePlayer::updatedChapter(int chapter) {
    qDebug() << "xMovie: updatedChapter:: " << chapter;
}

void xMoviePlayer::updatedTick(qint64 movieMediaPos) {
    qDebug() << "xMovie: updatedTick:: " << movieMediaPos << ", currentPlayed: " << movieCurrentPlayed << ", currentPos: " << movieCurrentPosition;

    if (moviePlayerState != xMoviePlayer::PlayingState) {
        qWarning() << "xMovie: already stopping; not tick update";
        return;
    }
    // Skip left-over ticks from previous movie
    if ((movieMediaPos > 500) && (movieCurrentPlayed == 0) && (movieCurrentPosition == 0)) {
        qWarning() << "xMovie: skip left-over tick";
        return;
    }

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
            movieCurrentPosition = movieMediaPos;
            // Determine if we need to update the database.
            auto update = false;
            if ((movieMediaLength - 10000) <= moviePlayed) {
                // Update if we are close to the end (within 10 seconds towards the end)
                update = (movieCurrentPosition >= movieMediaLength - 10000) &&
                    (movieCurrentPosition <= movieCurrentPlayed);
            } else {
                // Update if we played enough of the song.
                update = (movieCurrentPlayed >= moviePlayed);
            }
            if (update) {
                // Update database.
                auto name = movieCurrent.second;
                auto result = xPlayerDatabase::database()->updateMovieFile(movieCurrentTag,
                                                                           movieCurrentDirectory, name);

                qDebug() << "xMovie: updatedTick: db: " << movieCurrentTag << "," << movieCurrentDirectory << "," << name;
                qDebug() << "xMovie: updatedTick: db: " << result;
                if (result.second > 0) {
                    // Update database overlay.
                    emit updatePlayedMovie(movieCurrentTag, movieCurrentDirectory,
                                           name, result.first, result.second);
                    moviePlayedRecorded = true;
                }
            }
        } else {
            qCritical() << "xMoviePlayer::updatedTick: illegal movie positions: "
                << movieCurrentPosition << "," << movieMediaPos;
        }
    }

    emit currentMoviePlayed(movieMediaPos);
    updateCurrentChapter();
}

void xMoviePlayer::stateChanged(Phonon::State newState, Phonon::State oldState) {
    qDebug() << "xMoviePlayer: new: " << newState << ", old: " << oldState;
    if ((newState == Phonon::StoppedState) && (oldState == Phonon::PlayingState)) {
        emit currentState(moviePlayerState = xMoviePlayer::StopState);
    }
}

void xMoviePlayer::aboutToFinish() {
    if (movieQueue.isEmpty()) {
        qDebug() << "xMoviePlayer: aboutToFinish";
        // Go to stopping state. End full window mode.
        emit currentState(moviePlayerState = xMoviePlayer::StoppingState);
    }
}

void xMoviePlayer::closeToFinish(qint32 timeLeft) {
    qDebug() << "xMoviePlayer: closeToFinish: " << timeLeft;
    if (movieQueue.isEmpty()) {
        stop();
    } else {
        // Take next movie out of the queue and directly play it.
        auto nextMovie = movieQueue.takeFirst();
        setMovie(nextMovie.first, nextMovie.second, movieCurrentTag, movieCurrentDirectory);
    }
}

void xMoviePlayer::updatedDefaultAudioLanguage() {
    movieDefaultAudioLanguage = xPlayerConfiguration::configuration()->getMovieDefaultAudioLanguage();
}

void xMoviePlayer::updatedDefaultSubtitleLanguage() {
    movieDefaultSubtitleLanguage = xPlayerConfiguration::configuration()->getMovieDefaultSubtitleLanguage();
}

void xMoviePlayer::updateCurrentChapter() {
    if (currentChapterBegin.count() > 0) {
        qint64 currentPosition = moviePlayer->currentTime();
        int chapter = static_cast<int>(currentChapterBegin.count())-1;
        while (chapter >= 0) {
            if (currentChapterBegin[chapter] < currentPosition) {
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

void xMoviePlayer::fixAudioIssue() {
    jump(-1);
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
                jump(xMoviePlayer::ForwardRewindDelta);
            }
        } break;
        case Qt::Key_Left: {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                previousChapter();
            } else {
                // Seek -1 min
                jump(-xMoviePlayer::ForwardRewindDelta);
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
            if (moviePlayer->state() == Phonon::PlayingState) {
                moviePlayer->pause();
                emit currentState(moviePlayerState = xMoviePlayer::PauseState);
            } else {
                moviePlayer->play();
                emit currentState(moviePlayerState = xMoviePlayer::PlayingState);
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

void xMoviePlayer::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
    setFullWindowMode(!movieMediaFullWindow);
    mouseEvent->accept();
}

void xMoviePlayer::mousePressEvent(QMouseEvent* mouseEvent) {
    Phonon::VideoWidget::mousePressEvent(mouseEvent);
}
