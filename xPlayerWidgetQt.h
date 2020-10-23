#ifndef __XPLAYERWIDGETQT_H__
#define __XPLAYERWIDGETQT_H__

#include "xPlayerWidget.h"

#include <QLabel>
#include <QSlider>
#include <QPushButton>

class xPlayerWidgetQt:public xPlayerWidget {
    Q_OBJECT

public:
    xPlayerWidgetQt(xMusicPlayer* player, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerWidgetQt() = default;

protected slots:
    /**
     * Reset the the artist/album/track/played/length labels.
     */
    void clear();

    /**
     * Update the player widget with the provided information.
     *
     * @param index the position of the current track in the playlist.
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track name of the current track.
     */
    virtual void currentTrack(int index, const QString& artist, const QString& album, const QString& track);
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    virtual void currentTrackLength(qint64 length);
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    virtual void currentTrackPlayed(qint64 played);
    /**
     * Update the player UI based on the player state.
     *
     * @param state the current state of the player.
     */
    virtual void currentState(xMusicPlayer::State state);
    /**
     * Update the volume label.
     *
     * @param vol the current vol in between 0 and 100.
     */
    virtual void setVolume(int vol);

private:
    /**
     * Labels to display information about the current track.
     */
    QLabel* artistName;
    QLabel* albumName;
    QLabel* trackName;
    QLabel* trackLength;
    QLabel* trackPlayed;
    QLabel* volume;
    /**
     * The track slider can be used to seek within the current track.
     */
    QSlider* trackSlider;
    /**
     * Play/pause button alternates between "Play" and "Pause" depending on the music players state.
     */
    QPushButton* playPauseButton;
};

#endif
