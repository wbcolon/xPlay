#ifndef __XPLAYERWIDGET_H__
#define __XPLAYERWIDGET_H__

#include "xMusicPlayer.h"

#include <QWidget>

class xPlayerWidget:public QWidget {
    Q_OBJECT

public:
    xPlayerWidget(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerWidget() = default;

signals:
    /**
     * Signal the position of the current track.
     *
     * @param index the position of the track in the playlist.
     */
    void currentQueueTrack(int index);
    /**
     * Signal that the playlist needs to be cleared.
     */
    void clearQueue();

protected slots:
    /**
     * Reset the the artist/album/track/played/length labels.
     */
    virtual void clear() = 0;

    /**
     * Update the player widget with the provided information.
     *
     * @param index the position of the current track in the playlist.
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track name of the current track.
     */
    virtual void currentTrack(int index, const QString& artist, const QString& album, const QString& track) = 0;
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    virtual void currentTrackLength(qint64 length) = 0;
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    virtual void currentTrackPlayed(qint64 played) = 0;
    /**
     * Update the player UI based on the player state.
     *
     * @param state the current state of the player.
     */
    virtual void currentState(xMusicPlayer::State state) = 0;
    /**
     * Update the volume label.
     *
     * @param vol the current vol in between 0 and 100.
     */
    virtual void setVolume(int vol) = 0;
};

#endif
