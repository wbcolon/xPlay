#include "xMusicPlayerPhonon.h"

xMusicPlayerPhonon::xMusicPlayerPhonon(QObject* parent):
        xMusicPlayer(parent) {
    // Setup the media player.
    musicPlayer = Phonon::createPlayer(Phonon::MusicCategory);
    musicPlayer->setTransitionTime(0);
    musicPlayer->setTickInterval(500);
    // Setup the play list.
    // Connect QMediaPlayer signals to out music player signals.
    connect(musicPlayer, &Phonon::MediaObject::tick, this, &xMusicPlayerPhonon::currentTrackPlayed);
    connect(musicPlayer, &Phonon::MediaObject::currentSourceChanged, this, &xMusicPlayerPhonon::currentTrackSource);
    // We only need this one to determine the time due to issues with Phonon
    musicPlayerForTime = new QMediaPlayer(this);
    // This player is muted. It is only used to determine the proper duration.
    musicPlayerForTime->setVolume(0);
    // Connect musicPlayerForTime.
    connect(musicPlayerForTime, &QMediaPlayer::durationChanged, this, &xMusicPlayerPhonon::currentTrackDuration);
}

void xMusicPlayerPhonon::queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) {
    // Enable auto play if playlist is currently emtpy.
    bool autoPlay = (musicPlayer->queue().size() == 0);
    // Add given tracks to the playlist and to the musicPlaylistEntries data structure.
    for (auto track : tracks) {
        auto queueEntry = std::make_tuple(artist, album, track);
        musicPlaylistEntries.push_back(queueEntry);
        musicPlaylist.push_back(QUrl::fromLocalFile(pathFromQueueEntry(queueEntry)));
    }
    musicPlayer->enqueue(musicPlaylist);
    // Play if autoplay is enabled.
    if (autoPlay) {
        musicPlayer->play();
    }
}

void xMusicPlayerPhonon::currentTrackDuration(qint64 duration) {
    qDebug() << "TOTAL_TIME_CHANGED: LENGTH: " << duration;
    emit currentTrackLength(duration);
    musicPlayerForTime->stop();
}

void xMusicPlayerPhonon::clearQueue() {
    // Stop the music player and clear its state (including queue).
    musicPlayer->stop();
    musicPlayer->clear();
    // Remove entries from the play lists.
    musicPlaylistEntries.clear();
    musicPlaylist.clear();
}

void xMusicPlayerPhonon::playPause() {
    // Pause if the media player is in playing state, resume play.
    if (musicPlayer->state() == Phonon::PlayingState) {
        musicPlayer->pause();
    } else {
        musicPlayer->play();
    }
}

void xMusicPlayerPhonon::play(int index) {
    // Check if the index is valid.
    if ((index >= 0) && (index < musicPlaylist.size())) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue the tracks starting at position index.
        for (auto i = index; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // Play.
        musicPlayer->play();
    }
}

void xMusicPlayerPhonon::seek(qint64 position) {
    // Jump to position (in milliseconds) in the current track.
    musicPlayer->seek(position);
}

void xMusicPlayerPhonon::stop() {
    // Stop the media player.
    musicPlayer->stop();
}

void xMusicPlayerPhonon::prev() {
    // Jump to the previous element in the playlist if it exists.
    auto position = musicPlaylist.indexOf(musicPlayer->currentSource());
    if (position > 0) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue all tracks starting with position - 1.
        for (auto i = position - 1; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // Play.
        musicPlayer->play();
    }
}

void xMusicPlayerPhonon::next() {
    // Jump to the next element in the playlist if it exists.
    auto position = musicPlaylist.indexOf(musicPlayer->currentSource());
    if (position < musicPlaylist.size()-1) {
        // Stop the player and clear its state.
        musicPlayer->stop();
        musicPlayer->clear();
        // Queue all tracks starting with position + 1.
        for (auto i = position + 1; i < musicPlaylist.size(); ++i) {
            musicPlayer->enqueue(musicPlaylist[i]);
        }
        // Play.
        musicPlayer->play();
    }
}

void xMusicPlayerPhonon::currentTrackSource(const Phonon::MediaSource& current) {
    // Check in the index is valid.
    auto index = musicPlaylist.indexOf(current);
    if ((index >= 0) && (index < static_cast<int>(musicPlaylistEntries.size()))) {
        // Retrieve info for the currently played track and emit the information.
        auto entry = musicPlaylistEntries[index];
        emit currentTrack(index, std::get<0>(entry), std::get<1>(entry), std::get<2>(entry));
        // Use hack to determine the proper total length.
        // We need the muted musicPlayerForTime to play until the total time has been determined
        // and the durationChanged signal was triggered.
        musicPlayerForTime->setMedia(QUrl::fromLocalFile(current.fileName()));
        musicPlayerForTime->play();
    }
}
