#ifndef __XMUSICPLAYERPHONON_H__
#define __XMUSICPLAYERPHONON_H__

#include "xMusicPlayer.h"

#include <phonon/MediaObject>
#include <phonon/MediaSource>
#include <QMediaPlayer>

class xMusicPlayerPhonon:public xMusicPlayer {
    Q_OBJECT

public:
    xMusicPlayerPhonon(QObject* parent = nullptr);
    ~xMusicPlayerPhonon() = default;

public slots:
    /**
     * Append the given tracks to the current playlist.
     *
     * Each of the tracks will be added to the playlist as file (@see pathFromQueueEntry).
     * In addition the information is cached in the musicPlaylistEntries data structure
     * to allow for GUI updates of the player widget.
     *
     * @param artist the artist name for all queued tracks.
     * @param album the album name for all queued tracks.
     * @param tracks vector of track names.
     */
    virtual void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks);
    /**
     * Clear the playlist and stop the player.
     */
    virtual void clearQueue();

    /**
     * Play or pause depending on the current media player state.
     */
    virtual void playPause();
    /**
     * Play a given entry of the current playlist.
     *
     * @param index the position of the track in the playlist.
     */
    virtual void play(int index);
    /**
     * Move to the given position in the current track.
     *
     * @param position the position given in millisecond.
     */
    virtual void seek(qint64 position);
    /**
     * Stop the media player.
     */
    virtual void stop();
    /**
     * Jump to the previous track in the playlist.
     */
    virtual void prev();
    /**
     * Jump to the next track in the playlist.
     */
    virtual void next();

private slots:
    /**
     * Current playlist track has changed.
     *
     * Called whenever a new entry out of the playlist is played. The call is
     * triggered by the playlist. The currentTrack signal is triggered.
     *
     * @param index the position of the current track in the playlist.
     */
    void currentTrackSource(const Phonon::MediaSource& current);
    void currentTrackDuration(qint64 duration);

private:
    std::vector<std::tuple<QString,QString,QString>> musicPlaylistEntries;
    QList<Phonon::MediaSource> musicPlaylist;
    Phonon::MediaObject* musicPlayer;
    // Only required due to track length issues with phonon.
    QMediaPlayer* musicPlayerForTime;
};

#endif
