#ifndef __XMUSICPLAYER_H__
#define __XMUSICPLAYER_H__

#include <QMediaPlayer>
#include <QMediaPlaylist>

class xMusicPlayer:public QObject {
    Q_OBJECT

public:
    xMusicPlayer(QObject* parent = nullptr);
    ~xMusicPlayer() = default;

    /**
     * Set the base directory for the music library
     *
     * @param base the absolute path of the base of the music library.
     */
    void setBaseDirectory(const QString& base);

signals:
    /**
     * Signals information of the currently played track to the receiver.
     *
     * @param index the position of the current track in the playlist
     * @param artist the artist name for the current track.
     * @param album the album name for the current track.
     * @param track the name of the current track.
     */
    void currentTrack(int index, const QString& artist, const QString& album, const QString& track);
    /**
     * Signal the amount played for the current track.
     *
     * @param played the amount played in milliseconds.
     */
    void currentTrackPlayed(qint64 played);
    /**
     * Signal the length of the current track
     *
     * @param length the length in milliseconds.
     */
    void currentTrackLength(qint64 length);

public slots:
    virtual void queueTracks(const QString& artist, const QString& album, const std::vector<QString>& tracks) = 0;
    virtual void clearQueue() = 0;
    virtual void playPause() = 0;
    virtual void play(int index) = 0;
    virtual void seek(qint64 position) = 0;
    virtual void stop() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;

protected:
    /**
     * Generate a path from the given information.
     *
     * @param entry tuple of artist,album and track.
     * @return absolute path to the specified track.
     */
    QString pathFromQueueEntry(const std::tuple<QString, QString, QString>& entry);

    QString baseDirectory;
};

#endif
