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

const std::list<int> VLC_MediaPlayer_Events {  // NOLINT
        libvlc_MediaPlayerPlaying,
        libvlc_MediaPlayerEndReached,
        libvlc_MediaPlayerPositionChanged,
        libvlc_MediaPlayerLengthChanged,
        libvlc_MediaPlayerBuffering,
        libvlc_MediaPlayerChapterChanged,
        libvlc_MediaPlayerEncounteredError
};

const std::list<int> VLC_Media_Events {  // NOLINT
        libvlc_MediaParsedChanged, libvlc_MediaDurationChanged, libvlc_MediaStateChanged,
};

void xMoviePlayer::vlcStartMediaPlayer(bool compressAudio) {
    // Create a new libvlc instance. Use "--verbose=2" for additional libvlc output.
    const char* const movieVLCArgs[] = {
            "--vout=xcb_xv",
            "--file-caching=60000",
            "--live-caching=60000",
            "--disc-caching=60000",
            "--network-caching=60000",
            "--quiet",
            // "--verbose=2"
    };
    const char* const movieVLCArgsCompressor[] = {
            "--vout=xcb_xv",
            "--file-caching=60000",
            "--live-caching=60000",
            "--disc-caching=60000",
            "--network-caching=60000",
            "--audio-filter=compressor",
            "--compressor-rms-peak=0.2",
            "--compressor-attack=25",
            "--compressor-release=100",
            "--compressor-threshold=-11",
            "--compressor-ratio=4",
            "--compressor-knee=5",
            "--compressor-makeup-gain=7",
            "--quiet",
            // "--verbose=2"
    };
    movieMediaAudioCompressionMode = compressAudio;
    if (movieMediaAudioCompressionMode) {
        movieInstance = libvlc_new(sizeof(movieVLCArgsCompressor)/sizeof(movieVLCArgsCompressor[0]), movieVLCArgsCompressor);
        if (!movieInstance) {
            movieMediaAudioCompressionMode = false;
            qCritical() << "Unable to enable the VLC compressor plugin. Disabling audio compression.";
            emit moviePlayerError(tr("Unable to enable the VLC compressor plugin. Disabling audio compression."));
        }
    }
    if (!movieMediaAudioCompressionMode) {
        movieInstance = libvlc_new(sizeof(movieVLCArgs)/sizeof(movieVLCArgs[0]), movieVLCArgs);
    }
    if (!movieInstance) {
        qCritical() << "Unable to start vlc. Critical error. Abort.";
        abort();
    }
    movieMediaPlayer = libvlc_media_player_new(movieInstance);
    libvlc_media_player_set_role(movieMediaPlayer, libvlc_role_Video);
    movieMediaPlayerEventManager = libvlc_media_player_event_manager(movieMediaPlayer);
    for (auto event : VLC_MediaPlayer_Events) {
        libvlc_event_attach(movieMediaPlayerEventManager, event, handleVLCMediaEvents, this);
    }
}

void xMoviePlayer::vlcStopMediaPlayer() {
    // Detach events
    for (auto event : VLC_MediaPlayer_Events) {
        libvlc_event_detach(movieMediaPlayerEventManager, event, handleVLCMediaEvents, this);
    }
    // Stop playing
    libvlc_media_player_stop(movieMediaPlayer);
    // Free the media_player
    libvlc_media_player_release(movieMediaPlayer);
    // Release instance
    libvlc_release(movieInstance);
}

xMoviePlayer::xMoviePlayer(QWidget *parent):
        QFrame(parent),
        movieInstance(nullptr),
        movieMediaPlayer(nullptr),
        movieMediaPlayerEventManager(nullptr),
        movieMediaEventManager(nullptr),
        movieMedia(nullptr),
        movieMediaInitialPlay(false),
        movieMediaParsed(false),
        movieMediaPlaying(false),
        movieMediaLength(0),
        movieMediaChapter(0),
        movieMediaDeinterlaceMode(false),
        movieMediaCropAspectRatio(),
        movieMediaFullWindow(false),
        movieDefaultAudioLanguageIndex(-1),
        movieDefaultSubtitleLanguageIndex(-1) {
    // Do we use audio compression.
    movieMediaAudioCompressionMode = xPlayerConfiguration::configuration()->getMovieAudioCompression();
    // Set up the media player.
    vlcStartMediaPlayer(movieMediaAudioCompressionMode);
    pulseAudioControls = xPlayerPulseAudioControls::controls();
    // Connect signals used to call out of VLC handler.
    connect(this, &xMoviePlayer::eventHandler_stop, this, &xMoviePlayer::stop);
    connect(this, &xMoviePlayer::eventHandler_setMovie, this, &xMoviePlayer::setMovie);
    connect(this, &xMoviePlayer::eventHandler_selectAudioChannel, this, &xMoviePlayer::selectAudioChannel);
    connect(this, &xMoviePlayer::eventHandler_selectSubtitle, this, &xMoviePlayer::selectSubtitle);
    connect(this, &xMoviePlayer::eventHandler_parseFinished, this, &xMoviePlayer::parseFinished);
    // Connect to configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultAudioLanguage,
            this, &xMoviePlayer::updatedDefaultAudioLanguage);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieDefaultSubtitleLanguage,
            this, &xMoviePlayer::updatedDefaultSubtitleLanguage);
}

xMoviePlayer::~xMoviePlayer() noexcept {
    vlcStopMediaPlayer();
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
    libvlc_audio_set_mute(movieMediaPlayer, mute);
    pulseAudioControls->setMuted(mute);
}

bool xMoviePlayer::isMuted() const {
    return static_cast<bool>(libvlc_audio_get_mute(movieMediaPlayer));
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
        for (auto event: VLC_Media_Events) {
            libvlc_event_detach(movieMediaEventManager, event, handleVLCMediaEvents, this);
        }
        libvlc_media_parse_stop(movieMedia);
        libvlc_media_release(movieMedia);
        movieMedia = nullptr;
    }
    movieMedia = libvlc_media_new_path(movieInstance, path.generic_string().c_str());
    movieMediaEventManager = libvlc_media_event_manager(movieMedia);
    for (auto event: VLC_Media_Events) {
        libvlc_event_attach(movieMediaEventManager, event, handleVLCMediaEvents, this);
    }
    // Handle events coming in any order.
    movieMediaParsed = false;
    movieMediaPlaying = false;
    // Create a media player playing environment.
    libvlc_media_player_set_media(movieMediaPlayer, movieMedia);
    // Start parsing the file. Allow up to 10 second for preparsing.
    libvlc_media_parse_with_options(movieMedia, libvlc_media_parse_network, 10000);
    // Update display.
    emit currentMovie(path, name, tag, directory);
}

void xMoviePlayer::parseFinished() {
    movieMediaInitialPlay = true;
    libvlc_media_player_set_xwindow(movieMediaPlayer, winId());
    libvlc_media_player_play(movieMediaPlayer);
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

void xMoviePlayer::setAudioCompressionMode(bool mode) {
    if (mode != movieMediaAudioCompressionMode) {
        // Reset the vlc player.
        vlcStopMediaPlayer();
        vlcStartMediaPlayer(mode);
        emit currentState(State::ResetState);
        // Do not update the configuration. Configuration is default behavior.
    }
}

bool xMoviePlayer::audioCompressionMode() const {
    return movieMediaAudioCompressionMode;
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

void xMoviePlayer::handleVLCMediaEvents(const libvlc_event_t *event, void *data) {
    // Reconvert data to this (as self).
    auto self = reinterpret_cast<xMoviePlayer*>(data);
    switch (event->type) {
        case libvlc_MediaParsedChanged: {
            auto parsedStatus = libvlc_media_get_parsed_status(self->movieMedia);
            switch (parsedStatus) {
                case libvlc_media_parsed_status_done: {
                    // Determine length.
                    self->movieMediaLength = libvlc_media_get_duration(self->movieMedia);
                    emit self->currentMovieLength(self->movieMediaLength);
                    // Scan for audio channels and subtitles.
                    self->scanForAudioAndSubtitles();
                    // Scan for chapters.
                    self->scanForChapters();
                    // Scanning successful. No need for movieMedia pointer any more.
                    libvlc_event_detach(self->movieMediaEventManager,libvlc_MediaParsedChanged, handleVLCMediaEvents, self);
                    libvlc_media_release(self->movieMedia);
                    if (!self->movieMediaPlaying) {
                        self->movieMedia = nullptr;
                        self->movieMediaParsed = true;
                        emit self->eventHandler_parseFinished();
                    }
                    qDebug() << "PARSING FINISHED...";
                } break;
                case libvlc_media_parsed_status_timeout: {
                    // Start playing.
                    emit self->eventHandler_parseFinished();
                } break;
                default: {
                    qCritical() << "[libvlc] parsing failed with: " << parsedStatus << ". Stopping movie";
                    emit self->eventHandler_stop();
                }
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
        case libvlc_MediaPlayerPositionChanged: {
            auto movieMediaPos = static_cast<qint64>(event->u.media_player_position_changed.new_position*self->movieMediaLength); // NOLINT
            emit self->currentMoviePlayed(movieMediaPos);
            self->updateCurrentChapter();
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
        case libvlc_MediaStateChanged: {
            qDebug() << "[libvlc_MediaStateChanged] new state: " << event->u.media_state_changed.new_state;
            if (event->u.media_state_changed.new_state == libvlc_Playing) {
                self->movieMediaPlaying = true;
            }
        } break;
    }
    // Movie is parsed and playing.
    if ((self->movieMediaPlaying) && (self->movieMediaParsed)) {
        if (self->movieMediaInitialPlay) {
            self->movieMediaInitialPlay = false;
            // Select default audio channel in the UI and the player.
            if (self->movieDefaultAudioLanguageIndex >= 0) {
                emit self->eventHandler_selectAudioChannel(self->movieDefaultAudioLanguageIndex);
                emit self->currentAudioChannel(self->movieDefaultAudioLanguageIndex);
            }
            // Select default subtitle in the UI and the player.
            emit self->eventHandler_selectSubtitle(self->movieDefaultSubtitleLanguageIndex);
            emit self->currentSubtitle(self->movieDefaultSubtitleLanguageIndex);
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

void xMoviePlayer::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
    setFullWindowMode(!movieMediaFullWindow);
    mouseEvent->accept();
}

void xMoviePlayer::mousePressEvent(QMouseEvent* mouseEvent) {
    QFrame::mousePressEvent(mouseEvent);
}
