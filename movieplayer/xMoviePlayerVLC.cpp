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

#include "xMoviePlayerVLC.h"

#include <QEvent>
#include <QMouseEvent>
#include <QCheckBox>
#include <QTimer>
#include <QDebug>
#include <cmath>
#include <vlc/vlc.h>

// Borrowed code from https://wiki.videolan.org/LibVLC_SampleCode_Qt/

xMoviePlayerVLC::xMoviePlayerVLC(QWidget *parent):
        QFrame(parent),
        movieMedia(nullptr),
        movieMediaPlaying(false),
        movieMediaInitialParse(false),
        fullWindow(false) {
    // Setup the media player.
    moviePoller = new QTimer(this);
    // Create a new libvlc instance
    //const char* const movieVLCArgs[] = { "--verbose=2" };
    const char* const movieVLCArgs[] = { "--verbose=0" };
    movieInstance = libvlc_new(sizeof(movieVLCArgs)/sizeof(movieVLCArgs[0]), movieVLCArgs);
    // Create a media player playing environement
    movieMediaPlayer = libvlc_media_player_new(movieInstance);
    // Connect the two sliders to the corresponding slots (uses Qt's signal / slots technology)
    connect(moviePoller, &QTimer::timeout, this, &xMoviePlayerVLC::updateState);
    moviePoller->start(250); //start timer to trigger every 250 ms the updateInterface slot
}

xMoviePlayerVLC::~xMoviePlayerVLC() noexcept {
    // Stop playing
    libvlc_media_player_stop(movieMediaPlayer);
    // Free the media_player
    libvlc_media_player_release(movieMediaPlayer);
    // Release instance
    libvlc_release(movieInstance);
}

void xMoviePlayerVLC::setMuted(bool mute) {
    libvlc_audio_set_mute(movieMediaPlayer, mute);
}

bool xMoviePlayerVLC::isMuted() const {
    return static_cast<bool>(libvlc_audio_get_mute(movieMediaPlayer));
}

void xMoviePlayerVLC::setVolume(int vol) {
    vol = std::clamp(vol, 0, 100);
    libvlc_audio_set_volume(movieMediaPlayer, vol);
}

int xMoviePlayerVLC::getVolume() const {
    return libvlc_audio_get_volume(movieMediaPlayer);
}

void xMoviePlayerVLC::playPause() {
    // Pause if the media player is in playing state, resume play.
    auto state = libvlc_media_player_get_state(movieMediaPlayer);
    if (state == libvlc_Playing) {
        libvlc_media_player_pause(movieMediaPlayer);
        movieMediaPlaying = true;
        emit currentState(State::PauseState);
    }
    if ((state == libvlc_Paused) || (state == libvlc_Stopped)) {
        libvlc_media_player_play(movieMediaPlayer);
        movieMediaPlaying = false;
        emit currentState(State::PlayingState);
    }
}

void xMoviePlayerVLC::seek(qint64 position) {
    // Check if VLC is still playing.
    if (libvlc_media_player_get_media(movieMediaPlayer) == nullptr) {
        return;
    }
    // Jump to position (in milliseconds) in the current track.
    auto newPosition = static_cast<float>(static_cast<double>(position)/static_cast<double>(movieMediaLength));
    libvlc_media_player_set_position(movieMediaPlayer, newPosition);
}

void xMoviePlayerVLC::jump(qint64 delta) {
    // Check if VLC is still playing.
    if (libvlc_media_player_get_media(movieMediaPlayer) == nullptr) {
        return;
    }
    // Jump to current position + delta (in milliseconds) in the current track.
    auto currentPosition = static_cast<qint64>(libvlc_media_player_get_position(movieMediaPlayer)*movieMediaLength);
    seek(currentPosition+delta);
}

void xMoviePlayerVLC::stop() {
    // Stop the media player.
    libvlc_media_player_stop(movieMediaPlayer);
    movieMediaPlaying = false;
    movieMediaInitialParse = true;
    emit currentState(State::StopState);
    emit currentMoviePlayed(0);
}

void xMoviePlayerVLC::setMovie(const QString& path, const QString& name, const QString& tag, const QString& directory) {
    movieMedia = libvlc_media_new_path(movieInstance, path.toStdString().c_str());
    libvlc_media_player_set_media(movieMediaPlayer, movieMedia);
    scanMediaFile();
    libvlc_media_player_set_xwindow(movieMediaPlayer, winId());
    libvlc_media_player_play(movieMediaPlayer);
    movieMediaPlaying = true;
    movieMediaInitialParse = true;
    emit currentMovie(path, name, tag, directory);
    emit currentState(xMoviePlayerVLC::PlayingState);
}

void xMoviePlayerVLC::setMovieQueue(const QList<std::pair<QString,QString>>& queue, const QString& tag, const QString& directory) {
    movieQueue = queue;
    movieQueueTag = tag;
    movieQueueDirectory = directory;
}

void xMoviePlayerVLC::clearMovieQueue() {
    movieQueue.clear();
}

void xMoviePlayerVLC::setScaleAndCropMode(bool mode) {
    libvlc_video_set_scale(movieMediaPlayer, mode ? 0.0 : 1.0);
}

void xMoviePlayerVLC::toggleScaleAndCropMode() {
    auto mode = (libvlc_video_get_scale(movieMediaPlayer) == 0.0);
    setScaleAndCropMode(mode);
    emit scaleAndCropMode(mode);
}

void xMoviePlayerVLC::selectAudioChannel(int index) {
    if ((index >= 0) && (index < currentAudioChannelDescriptions.size())) {
        libvlc_audio_set_track(movieMediaPlayer, currentAudioChannelDescriptions[index].first);
    }
}
void xMoviePlayerVLC::selectSubtitle(int index) {
    if ((index >= 0) && (index < currentSubtitleDescriptions.size())) {
        libvlc_video_set_spu(movieMediaPlayer, currentSubtitleDescriptions[index].first);
    }
}

void xMoviePlayerVLC::updateState() {
    if (libvlc_media_player_get_media(movieMediaPlayer) == nullptr) {
        return;
    }
    auto state = libvlc_media_player_get_state(movieMediaPlayer);
    if (state == libvlc_Playing) {
        auto movieMediaPos = static_cast<qint64>(libvlc_media_player_get_position(movieMediaPlayer) * movieMediaLength);
        emit currentMoviePlayed(movieMediaPos);

        if (movieMediaInitialParse) {
            movieMediaInitialParse = false;
            // Determine current volume.
            auto volume = libvlc_audio_get_volume(movieMediaPlayer);
            emit currentVolume(volume);
            // Disable subtitles by default
            libvlc_video_set_spu(movieMediaPlayer, -1);
            // Use first audio channel by default
            if (currentAudioChannelDescriptions.count() > 0) {
                libvlc_audio_set_track(movieMediaPlayer, currentAudioChannelDescriptions[0].first);
            }
            // Update scale and crop mode.
            emit scaleAndCropMode(true);
        }
    } else {
        if ((movieMediaPlaying) && (state >= libvlc_Stopped)) {
            // Supposed to be playing, but state does not match.
            // Play next movie in the queue or stop.
            if (movieQueue.isEmpty()) {
                stop();
            } else {
                auto nextMovie = movieQueue.takeFirst();
                setMovie(nextMovie.first, nextMovie.second, movieQueueTag, movieQueueDirectory);
            }
        }
    }
}

void xMoviePlayerVLC::scanMediaFile() {
    libvlc_media_parse(movieMedia);
    movieMediaLength = static_cast<qint64>(libvlc_media_get_duration(movieMedia));
    emit currentMovieLength(movieMediaLength);

    libvlc_media_track_t** movieMediaTracks = nullptr;
    unsigned noTracks = libvlc_media_tracks_get(movieMedia, &movieMediaTracks);
    // Reset channel descriptions.
    currentAudioChannelDescriptions.clear();
    currentSubtitleDescriptions.clear();
    currentSubtitleDescriptions.push_back(std::make_pair(-1, "disable"));
    QStringList audioChannels;
    QStringList subtitles { "disable" };
    for (unsigned i = 0; i < noTracks; ++i) {
         if (movieMediaTracks[i]->i_type == libvlc_track_audio) {
             auto audioChannelDescription = QString(movieMediaTracks[i]->psz_language);
             if ((movieMediaTracks[i]->psz_description) &&
                 (strlen(movieMediaTracks[i]->psz_description) > 0)) {
                 audioChannelDescription += QString(" (%1)").arg(movieMediaTracks[i]->psz_description).toLower();
             }
             if (movieMediaTracks[i]->audio->i_channels > 2) {
                 audioChannelDescription += QString(" [%1.1]").arg(movieMediaTracks[i]->audio->i_channels - 1);
             } else {
                 audioChannelDescription += QString(" [stereo]");
             }
             audioChannels.push_back(audioChannelDescription);
             currentAudioChannelDescriptions.push_back(std::make_pair(i, audioChannelDescription));
         }
        if (movieMediaTracks[i]->i_type == libvlc_track_text) {
            auto subtitleDescription = QString(movieMediaTracks[i]->psz_language);
            if ((movieMediaTracks[i]->psz_description) &&
                (strlen(movieMediaTracks[i]->psz_description) > 0)) {
                subtitleDescription += QString(" (%1)").arg(movieMediaTracks[i]->psz_description).toLower();
            }
            if ((movieMediaTracks[i]->subtitle->psz_encoding) &&
                (strlen(movieMediaTracks[i]->subtitle->psz_encoding) > 0)) {
                    subtitleDescription += QString(" [%1]").arg(movieMediaTracks[i]->subtitle->psz_encoding).toLower();
            }
            subtitles.push_back(subtitleDescription);
            currentSubtitleDescriptions.push_back(std::make_pair(i, subtitleDescription));
        }
    }
    libvlc_media_tracks_release(movieMediaTracks, noTracks);
    emit currentAudioChannels(audioChannels);
    emit currentSubtitles(subtitles);
}

void xMoviePlayerVLC::keyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
        case Qt::Key_Escape: {
            toggleFullWindow();
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
            auto state = libvlc_media_player_get_state(movieMediaPlayer);
            if (state == libvlc_Playing) {
                libvlc_media_player_pause(movieMediaPlayer);
                emit currentState(State::PauseState);
            }
            if ((state == libvlc_Paused) || (state == libvlc_Stopped)) {
                libvlc_media_player_play(movieMediaPlayer);
                emit currentState(State::PlayingState);
            }
        } break;
        case Qt::Key_S: {
            toggleScaleAndCropMode();
        } break;
        default: {
            QFrame::keyPressEvent(keyEvent);
            // Do not accept key event.
            return;
        }
    }
    keyEvent->accept();
}

void xMoviePlayerVLC::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    emit toggleFullWindow();
    mouseEvent->accept();
}

void xMoviePlayerVLC::mousePressEvent(QMouseEvent* mouseEvent)
{
    QFrame::mousePressEvent(mouseEvent);
}
