#include "xMusicPlayer.h"

xMusicPlayer::xMusicPlayer(QObject* parent):
        QObject(parent) {
}

void xMusicPlayer::setBaseDirectory(const QString& base) {
    // Set base directory for the music library. Necessary for creating absolute file paths.
    baseDirectory = base;
}

QString xMusicPlayer::pathFromQueueEntry(const std::tuple<QString, QString, QString>& elem) {
    // Create an absolute path from the music library base and the tuple of album,artist and track.
    return baseDirectory+"/"+std::get<0>(elem)+"/"+std::get<1>(elem)+"/"+std::get<2>(elem);
}

