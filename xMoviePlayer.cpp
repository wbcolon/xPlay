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
#include "xPlayerPulseAudioControls.h"

#include <QEvent>
#include <QMouseEvent>
#include <QCheckBox>
#include <QTimer>
#include <QDebug>
#include <cmath>

#include <vlc/vlc.h>

std::list<std::pair<QString,QString>> xMoviePlayer::supportedAspectRatio() {
    return {
        {"2.39 : 1", "239:100"},
        {"1.85 : 1", "185:100"},
        {"16 : 9", "16:9"},
        {"16 : 10", "16:10"},
        {"4 : 3", "4:3"}
    };
}

xMoviePlayer::xMoviePlayer(QWidget *parent):
        QFrame(parent),
        movieMediaPlayer(nullptr),
        movieMediaEventManager(nullptr),
        movieMedia(nullptr),
        movieMediaInitialPlay(false),
        movieMediaParsed(false),
        movieMediaPlaying(false),
        movieMediaLength(0),
        movieMediaChapter(0),
        movieMediaDeinterlaceMode(false),
        movieMediaCropAspectRatio(),
        movieDefaultAudioLanguageIndex(-1),
        movieDefaultSubtitleLanguageIndex(-1),
        fullWindow(false) {
    // Setup the media player.
    // Create a new libvlc instance. User "--verbose=2" for additional libvlc output.
    // const char* const movieVLCArgs[] = { "--vout=gl",  "--verbose=2" };
    const char* const movieVLCArgs[] = { "--vout=xcb_xv", "--quiet" };
    movieInstance = libvlc_new(sizeof(movieVLCArgs)/sizeof(movieVLCArgs[0]), movieVLCArgs);
    movieMediaPlayer = libvlc_media_player_new(movieInstance);
    movieMediaPlayerEventManager = libvlc_media_player_event_manager(movieMediaPlayer);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerPlaying, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerEndReached, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerAudioVolume, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerPositionChanged, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerLengthChanged, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerBuffering, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerChapterChanged, handleVLCEvents, this);
    libvlc_event_attach(movieMediaPlayerEventManager,libvlc_MediaPlayerEncounteredError, handleVLCEvents, this);
    // Connect signals used to call out of VLC handler.
    connect(this, &xMoviePlayer::eventHandler_stop, this, &xMoviePlayer::stop);
    connect(this, &xMoviePlayer::eventHandler_setMovie, this, &xMoviePlayer::setMovie);
    connect(this, &xMoviePlayer::eventHandler_selectAudioChannel, this, &xMoviePlayer::selectAudioChannel);
    connect(this, &xMoviePlayer::eventHandler_selectSubtitle, this, &xMoviePlayer::selectSubtitle);
    // Connect to configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultAudioLanguage,
            this, &xMoviePlayer::updatedDefaultAudioLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultSubtitleLanguage,
            this, &xMoviePlayer::updatedDefaultSubtitleLanguage);
}

xMoviePlayer::~xMoviePlayer() noexcept {
    // Detach events
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerPlaying, handleVLCEvents, this);
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerEndReached, handleVLCEvents, this);
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerAudioVolume, handleVLCEvents, this);
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerPositionChanged, handleVLCEvents, this);
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerLengthChanged, handleVLCEvents, this);
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerBuffering, handleVLCEvents, this);
    libvlc_event_detach(movieMediaPlayerEventManager,libvlc_MediaPlayerEncounteredError, handleVLCEvents, this);
    // Stop playing
    libvlc_media_player_stop(movieMediaPlayer);
    // Free the media_player
    libvlc_media_player_release(movieMediaPlayer);
    // Release instance
    libvlc_release(movieInstance);
}

void xMoviePlayer::setMuted(bool mute) {
    // Mute the stream and the pulse audio sink
    libvlc_audio_set_mute(movieMediaPlayer, mute);
    xPlayerPulseAudioControls::controls()->setMuted(mute);
}

bool xMoviePlayer::isMuted() const {
    return static_cast<bool>(libvlc_audio_get_mute(movieMediaPlayer));
}

void xMoviePlayer::setVolume(int vol) { // NOLINT
    vol = std::clamp(vol, 0, 100);
    xPlayerPulseAudioControls::controls()->setVolume(vol);
}

int xMoviePlayer::getVolume() const { // NOLINT
    return xPlayerPulseAudioControls::controls()->getVolume();
}

void xMoviePlayer::playPause() {
    // Pause if the media player is in playing state, resume play.
    auto state = libvlc_media_player_get_state(movieMediaPlayer);
    if (state == libvlc_Playing) {
        libvlc_media_player_pause(movieMediaPlayer);
        emit currentState(State::PauseState);
    }
    if ((state == libvlc_Paused) || (state == libvlc_Stopped)) {
        libvlc_media_player_play(movieMediaPlayer);
        vlcFixAudio();
        emit currentState(State::PlayingState);
    }
}

void xMoviePlayer::playChapter(int chapter) {
    if ((chapter >= 0) && (chapter < currentChapterDescriptions.count())) {
        libvlc_media_player_set_chapter(movieMediaPlayer, chapter);
        vlcFixAudio();
        updateCurrentChapter();
        emit currentState(State::PlayingState);
    }
}

void xMoviePlayer::previousChapter() {
    libvlc_media_player_previous_chapter(movieMediaPlayer);
    vlcFixAudio();
    updateCurrentChapter();
}

void xMoviePlayer::nextChapter() {
    libvlc_media_player_next_chapter(movieMediaPlayer);
    vlcFixAudio();
    updateCurrentChapter();
}

void xMoviePlayer::seek(qint64 position) {
    // Check if VLC is still playing.
    if (libvlc_media_player_get_media(movieMediaPlayer) == nullptr) {
        return;
    }
    // Jump to position (in milliseconds) in the current track.
    auto newPosition = static_cast<float>(static_cast<double>(position)/static_cast<double>(movieMediaLength));
    libvlc_media_player_set_position(movieMediaPlayer, newPosition);
    vlcFixAudio();
    updateCurrentChapter();
}

void xMoviePlayer::jump(qint64 delta) {
    // Check if VLC is still playing.
    if (libvlc_media_player_get_media(movieMediaPlayer) == nullptr) {
        return;
    }
    // Jump to current position + delta (in milliseconds) in the current track.
    auto currentPosition = static_cast<qint64>(libvlc_media_player_get_position(movieMediaPlayer)*movieMediaLength); // NOLINT
    seek(currentPosition+delta);
}

void xMoviePlayer::stop() {
    // Stop (pause and reset to position 0) the media player.
    libvlc_media_player_set_pause(movieMediaPlayer, 1);
    libvlc_media_player_set_position(movieMediaPlayer, 0);
    vlcFixAudio();
    updateCurrentChapter();
    emit currentState(State::StopState);
    emit currentMoviePlayed(0);
}

void xMoviePlayer::setMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory) {
    // Stop parsing if current movie file is still being parsed.
    if (movieMedia) {
        libvlc_event_detach(movieMediaEventManager,libvlc_MediaParsedChanged, handleVLCEvents, this);
        libvlc_event_detach(movieMediaEventManager,libvlc_MediaDurationChanged, handleVLCEvents, this);
        libvlc_event_detach(movieMediaEventManager,libvlc_MediaStateChanged, handleVLCEvents, this);
        libvlc_media_parse_stop(movieMedia);
        libvlc_media_release(movieMedia);
        movieMedia = nullptr;
    }
    movieMedia = libvlc_media_new_path(movieInstance, path.generic_string().c_str());
    movieMediaEventManager = libvlc_media_event_manager(movieMedia);
    libvlc_event_attach(movieMediaEventManager,libvlc_MediaParsedChanged, handleVLCEvents, this);
    libvlc_event_attach(movieMediaEventManager,libvlc_MediaDurationChanged, handleVLCEvents, this);
    libvlc_event_attach(movieMediaEventManager,libvlc_MediaStateChanged, handleVLCEvents, this);
    // Create a media player playing environment.
    libvlc_media_player_set_media(movieMediaPlayer, movieMedia);
    // Start parsing the file. Allow up to 1 second for preparsing.
    libvlc_media_parse_with_options(movieMedia, libvlc_media_parse_network, 1000);
    libvlc_media_player_set_xwindow(movieMediaPlayer, winId());
    libvlc_media_player_play(movieMediaPlayer);
    movieMediaInitialPlay = true;
    // Handle events coming in any order.
    movieMediaParsed = false;
    movieMediaPlaying = false;
    emit currentMovie(path, name, tag, directory);
    emit currentState(xMoviePlayer::PlayingState);
}

void xMoviePlayer::setMovieQueue(const QList<std::pair<std::filesystem::path,QString>>& queue, const QString& tag, const QString& directory) {
    movieQueue = queue;
    movieQueueTag = tag;
    movieQueueDirectory = directory;
}

void xMoviePlayer::clearMovieQueue() {
    movieQueue.clear();
}

void xMoviePlayer::setDeinterlaceMode(bool mode) {
    movieMediaDeinterlaceMode = mode;
    if (movieMediaDeinterlaceMode) {
        libvlc_video_set_deinterlace(movieMediaPlayer, "Linear");
    } else {
        libvlc_video_set_deinterlace(movieMediaPlayer, nullptr);
    }
}

bool xMoviePlayer::deinterlaceMode() const {
    return movieMediaDeinterlaceMode;
}

void xMoviePlayer::setCropAspectRatio(const QString& aspectRatio) {
    if (aspectRatio.isEmpty()) {
        libvlc_video_set_crop_geometry(movieMediaPlayer, nullptr);
        movieMediaCropAspectRatio.clear();
    } else {
        for (auto&& supported : supportedAspectRatio()) {
            if (supported.second == aspectRatio) {
                libvlc_video_set_crop_geometry(movieMediaPlayer, aspectRatio.toStdString().c_str());
                movieMediaCropAspectRatio = aspectRatio;
            }
        }
    }
}

QString xMoviePlayer::cropAspectRatio() const {
    return movieMediaCropAspectRatio;
}

void xMoviePlayer::selectAudioChannel(int index) {
    if ((index >= 0) && (index < currentAudioChannelDescriptions.size())) {
        libvlc_audio_set_track(movieMediaPlayer, currentAudioChannelDescriptions[index].first);
    }
}
void xMoviePlayer::selectSubtitle(int index) {
    if ((index >= 0) && (index < currentSubtitleDescriptions.size())) {
        libvlc_video_set_spu(movieMediaPlayer, currentSubtitleDescriptions[index].first);
    }
}

void xMoviePlayer::handleVLCEvents(const libvlc_event_t *event, void *data) {
    // Reconvert data to this (as self).
    auto self = reinterpret_cast<xMoviePlayer*>(data);
    switch (event->type) {
        case libvlc_MediaParsedChanged: {
            if (libvlc_media_get_parsed_status(self->movieMedia) == libvlc_media_parsed_status_done) {
                // Determine length.
                self->movieMediaLength = libvlc_media_get_duration(self->movieMedia);
                emit self->currentMovieLength(self->movieMediaLength);
                // Scan for audio channels and subtitles.
                self->scanForAudioAndSubtitles();
                // Scan for chapters.
                self->scanForChapters();
                // Scanning successful. No need for movieMedia pointer any more.
                libvlc_event_detach(self->movieMediaEventManager,libvlc_MediaParsedChanged, handleVLCEvents, self);
                libvlc_media_release(self->movieMedia);
                self->movieMedia = nullptr;
                self->movieMediaParsed = true;
            }
        } break;
        case libvlc_MediaDurationChanged: {
            if (event->u.media_duration_changed.new_duration != self->movieMediaLength) {
                qDebug() << "[libvlc_MediaDurationChanged] duration changed from " << self->movieMediaLength << " to "
                         << event->u.media_duration_changed.new_duration;
                self->movieMediaLength = event->u.media_duration_changed.new_duration;
                emit self->currentMovieLength(self->movieMediaLength);
            }
        } break;
        case libvlc_MediaStateChanged: {
            qDebug() << "[libvlc_MediaStateChanged] new state: " << event->u.media_state_changed.new_state;
        } break;
        case libvlc_MediaPlayerAudioVolume: {
            auto volume =  static_cast<int>(event->u.media_player_audio_volume.volume*100);
            emit self->currentVolume(volume);
        } break;
        case libvlc_MediaPlayerPositionChanged: {
            auto movieMediaPos = static_cast<qint64>(event->u.media_player_position_changed.new_position*self->movieMediaLength); // NOLINT
            emit self->currentMoviePlayed(movieMediaPos);
            self->updateCurrentChapter();
        } break;
        case libvlc_MediaPlayerPlaying: {
            self->movieMediaPlaying = true;
        } break;
        case libvlc_MediaPlayerLengthChanged: {
            if (event->u.media_player_length_changed.new_length != self->movieMediaLength) {
                qDebug() << "[libvlc_MediaPlayerLengthChanged] duration changed from " << self->movieMediaLength << " to "
                         << event->u.media_player_length_changed.new_length;
                self->movieMediaLength = event->u.media_player_length_changed.new_length;
                emit self->currentMovieLength(self->movieMediaLength);
            }
        } break;
        case libvlc_MediaPlayerChapterChanged: {
            // Scan for chapters if we did not find any so far.
            if (!self->currentChapterDescriptions.count()) {
                self->scanForChapters();
            }
        } break;
        case libvlc_MediaPlayerEncounteredError: {
            qWarning() << "[libvlc_MediaPlayerEncounteredError]";
        } break;
        case libvlc_MediaPlayerEndReached: {
            // Supposed to be playing, but state does not match.
            // Play next movie in the queue or stop.
            if (self->movieQueue.isEmpty()) {
                emit self->eventHandler_stop();
            } else {
                auto nextMovie = self->movieQueue.takeFirst();
                emit self->eventHandler_setMovie(nextMovie.first, nextMovie.second, self->movieQueueTag,
                                                 self->movieQueueDirectory);
            }
        } break;
    }
    // Movie is parsed and playing.
    if ((self->movieMediaPlaying) && (self->movieMediaParsed)) {
        if (self->movieMediaInitialPlay) {
            self->movieMediaInitialPlay = false;
            // Select default audio channel in the UI and the player.
            if (self->movieDefaultAudioLanguageIndex >= 0) {
                emit self->currentAudioChannel(self->movieDefaultAudioLanguageIndex);
                emit self->eventHandler_selectAudioChannel(self->movieDefaultAudioLanguageIndex);
            }
            // Select default subtitle in the UI and the player.
            emit self->currentSubtitle(self->movieDefaultSubtitleLanguageIndex);
            emit self->eventHandler_selectSubtitle(self->movieDefaultSubtitleLanguageIndex);
        }
    }
}

void xMoviePlayer::scanForAudioAndSubtitles() {
    libvlc_media_track_t** movieMediaTracks = nullptr;
    unsigned noTracks = libvlc_media_tracks_get(movieMedia, &movieMediaTracks);
    // Reset channel descriptions.
    currentAudioChannelDescriptions.clear();
    currentSubtitleDescriptions.clear();
    currentSubtitleDescriptions.push_back(std::make_pair(-1, "disable"));
    QStringList audioChannels;
    QStringList audioCodecs;
    QStringList subtitles { "disable" };
    for (unsigned i = 0; i < noTracks; ++i) {
         if (movieMediaTracks[i]->i_type == libvlc_track_audio) {
             auto audioChannelDescription = QString(movieMediaTracks[i]->psz_language).toLower();
             updateAudioAndSubtitleDescription(audioChannelDescription);
             auto description = QString(movieMediaTracks[i]->psz_description).toLower();
             // The descriptions stereo and surround do not add any value. Skip them.
             if ((!description.isEmpty()) && (description.compare("stereo") != 0) && (description.compare("surround") != 0)) {
                 audioChannelDescription += QString(" (%1)").arg(description);
             }
             if (movieMediaTracks[i]->audio->i_channels > 2) {
                 audioChannelDescription += QString(" [%1.1]").arg(movieMediaTracks[i]->audio->i_channels - 1);
             } else {
                 audioChannelDescription += QString(" [stereo]");
             }
             auto codec = QString(libvlc_media_get_codec_description(libvlc_track_audio, movieMediaTracks[i]->i_codec)).toLower();
             audioCodecs.push_back(codec);
             audioChannels.push_back(audioChannelDescription);
             currentAudioChannelDescriptions.push_back(std::make_pair(i, audioChannelDescription));
         }
        if (movieMediaTracks[i]->i_type == libvlc_track_text) {
            auto subtitleDescription = QString(movieMediaTracks[i]->psz_language);
            updateAudioAndSubtitleDescription(subtitleDescription);
            auto description = QString(movieMediaTracks[i]->psz_description).toLower();
            if (!description.isEmpty()) {
                subtitleDescription += QString(" (%1)").arg(movieMediaTracks[i]->psz_description).toLower();
            }
            auto encoding = QString(movieMediaTracks[i]->subtitle->psz_encoding).toLower();
            if (!encoding.isEmpty()) {
                subtitleDescription += QString(" [%1]").arg(movieMediaTracks[i]->subtitle->psz_encoding).toLower();
            }
            subtitles.push_back(subtitleDescription);
            currentSubtitleDescriptions.push_back(std::make_pair(i, subtitleDescription));
        }
    }
    libvlc_media_tracks_release(movieMediaTracks, noTracks);
    // Check if we have a default audio language.
    movieDefaultAudioLanguageIndex = -1;
    if (!movieDefaultAudioLanguage.isEmpty()) {
        for (auto i = 0; i < audioChannels.count(); ++i) {
            // Find the first match.
            if (audioChannels[i].contains(movieDefaultAudioLanguage, Qt::CaseInsensitive)) {
                movieDefaultAudioLanguageIndex = i;
                break;
            }
        }
    }
    // Check if we have a default audio language.
    movieDefaultSubtitleLanguageIndex = 0;
    if (!movieDefaultSubtitleLanguage.isEmpty()) {
        for (auto i = 0; i < subtitles.count(); ++i) {
            // Find the first match.
            if (subtitles[i].contains(movieDefaultSubtitleLanguage, Qt::CaseInsensitive)) {
                movieDefaultSubtitleLanguageIndex = i;
                break;
            }
        }
    }
    // Emit the updates.
    emit currentAudioChannels(audioChannels, audioCodecs);
    emit currentSubtitles(subtitles);
}

void xMoviePlayer::scanForChapters() {
    // Reset chapter descriptions.
    currentChapterDescriptions.clear();
    // Check for chapters.
    QStringList chapters;
    libvlc_chapter_description_t** chapterDescription = nullptr;
    int noChapters = libvlc_media_player_get_full_chapter_descriptions(movieMediaPlayer, -1, &chapterDescription);
    if (chapterDescription != nullptr) {
        for (auto i = 0; i < noChapters; ++i) {
            auto chapterName = QString(chapterDescription[i]->psz_name).trimmed();
            currentChapterDescriptions.push_back(std::make_pair(chapterDescription[i]->i_time_offset, chapterName));
            chapters.push_back(chapterName);
        }
        libvlc_chapter_descriptions_release(chapterDescription, noChapters);
    }
    // Emit the updates.
    emit currentChapters(chapters);
}

void xMoviePlayer::updateCurrentChapter() {
    if (currentChapterDescriptions.count() > 0) {
        auto chapter = libvlc_media_player_get_chapter(movieMediaPlayer);
        if (movieMediaChapter != chapter) {
            // Update current chapter.
            movieMediaChapter = chapter;
            // Emit new chapter index.
            emit currentChapter(chapter);
        }
    }
}

void xMoviePlayer::vlcFixAudio() {
    // Workaround for audio issues after seeking, otherwise we see some kind of audio stutter.
    auto oldAudioChannel = libvlc_audio_get_channel(movieMediaPlayer);
    if (oldAudioChannel != libvlc_AudioChannel_RStereo) {
        libvlc_audio_set_channel(movieMediaPlayer, libvlc_AudioChannel_RStereo);
    } else {
        libvlc_audio_set_channel(movieMediaPlayer, libvlc_AudioChannel_Stereo);
    }
    libvlc_audio_set_channel(movieMediaPlayer, oldAudioChannel);
}

void xMoviePlayer::updateAudioAndSubtitleDescription(QString& description) {
    description.replace("ger", "german");
    description.replace("deu", "german");
    description.replace("eng", "english");
    description.replace("fre", "french");
}

void xMoviePlayer::updatedDefaultAudioLanguage() {
    movieDefaultAudioLanguage = xPlayerConfiguration::configuration()->getMovieDefaultAudioLanguage();
}

void xMoviePlayer::updatedDefaultSubtitleLanguage() {
    movieDefaultSubtitleLanguage = xPlayerConfiguration::configuration()->getMovieDefaultSubtitleLanguage();
}

void xMoviePlayer::keyPressEvent(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
        case Qt::Key_Escape: {
            toggleFullWindow();
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
            auto state = libvlc_media_player_get_state(movieMediaPlayer);
            if (state == libvlc_Playing) {
                libvlc_media_player_pause(movieMediaPlayer);
                emit currentState(State::PauseState);
            }
            if ((state == libvlc_Paused) || (state == libvlc_Stopped)) {
                libvlc_media_player_play(movieMediaPlayer);
                vlcFixAudio();
                emit currentState(State::PlayingState);
            }
        } break;
        default: {
            QFrame::keyPressEvent(keyEvent);
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
    QFrame::mousePressEvent(mouseEvent);
}
