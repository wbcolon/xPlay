#ifndef __XMUSICPLAYERQT_H__
#define __XMUSICPLAYERQT_H__

#include "xMusicPlayer.h"

#include <QMediaPlayer>
#include <QMediaPlaylist>

class xMusicPlayerQt:public xMusicPlayer {
    Q_OBJECT

public:
    xMusicPlayerQt(QObject* parent = nullptr);
    ~xMusicPlayerQt() = default;
    /**
     * Return the volume for the music player
     *
     * @return integer value in between 0 and 100.
     */
    virtual int getVolume();

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
     * Remove track from the current playlist
     *
     * @param index of the track to be removed.
     */
    virtual void dequeTrack(int index);
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
    /**
     * Set the volume
     *
     * @param vol integer value between 0 (silence) and 100 (full volume)
     */
    virtual void setVolume(int vol);

private slots:
    /**
     * Current playlist track has changed.
     *
     * Called whenever a new entry out of the playlist is played. The call is
     * triggered by the playlist. The currentTrack signal is triggered.
     *
     * @param index the position of the current track in the playlist.
     */
    void currentTrackIndex(int index);

private:
    std::vector<std::tuple<QString,QString,QString>> musicPlaylistEntries;
    QMediaPlayer* musicPlayer;
    QMediaPlaylist* musicPlaylist;
};

#endif
