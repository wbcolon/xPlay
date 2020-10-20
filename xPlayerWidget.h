#ifndef __XPLAYERWIDGET_H__
#define __XPLAYERWIDGET_H__

#include "xMusicPlayer.h"

#include <QLabel>
#include <QSlider>
#include <QPushButton>

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

private slots:
    /**
     * Update the play/pause button
     */
    void playPause();
    /**
     * Reset the play/pause button.
     */
    void stop();
    /**
     * Reset the play/pause button and the artist/album/track/played/length labels.
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
    void currentTrack(int index, const QString& artist, const QString& album, const QString& track);
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    void currentTrackLength(qint64 length);
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    void currentTrackPlayed(qint64 played);

private:
    /**
     * Labels to display information about the current track.
     */
    QLabel* artistName;
    QLabel* albumName;
    QLabel* trackName;
    QLabel* trackLength;
    QLabel* trackPlayed;
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
