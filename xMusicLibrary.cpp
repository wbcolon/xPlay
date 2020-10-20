#include "xMusicLibrary.h"

#include <QDebug>

// Use "/tmp" as default path
const std::string defaultBaseDirectory{ "/tmp" };

xMusicLibrary::xMusicLibrary(QObject* parent):
        QObject(parent),
        baseDirctory(defaultBaseDirectory) {
}

void xMusicLibrary::setBaseDirectory(const std::filesystem::path& base) {
    // Update only if the given path is a directory
    if (std::filesystem::is_directory(base)) {
        baseDirctory = base;
        // Scan music library after updated director.
        // Rescan with same base intentional. Miniamal API.
        scan();
    }
}

const std::filesystem::path& xMusicLibrary::getBaseDirectory() const {
    return baseDirctory;
}

void xMusicLibrary::scanForArtist(const QString& artist) {
    std::list<QString> albumList;
    try {
        auto artistAlbums = musicFiles[artist];
        for (const auto& album : artistAlbums) {
            auto albumName = album.first;
            qDebug() << "xMusicLibrary::scanArtist: album: " << albumName;
            albumList.push_back(albumName);
        }
    } catch (...) {
        // Clear list on error
        albumList.clear();
    }
    // Update widget
    emit scannedAlbums(albumList);
}

void xMusicLibrary::scanForArtistAndAlbum(const QString& artist, const QString& album) {
    std::list<QString> trackList;
    try {
        auto trackPath = musicFiles[artist][album];
        // Remove album path use it to scan the directory.
        // We do not assume that there will be too many changes.
        auto albumPath = trackPath.front();
        trackPath.pop_front();
        if (trackPath.empty()) {
            qDebug() << "xMusicLibrary::scanAlbum: read files from disk";
            trackPath = scanForAlbumTracks(albumPath);
            musicFiles[artist][album] = trackPath;
            // Remove album path again.
            // We do not need it for track list.
            trackPath.pop_front();
        }
        // Generate list of tracks for artist/album
        for (const auto& track : trackPath) {
            trackList.push_back(QString::fromStdString(track.filename()));
            qDebug() << "xMusicLibrary::scanAlbum: track: " << QString::fromStdString(track.filename());
        }
    } catch (...) {
        // Clear list on error
        trackList.clear();
    }
    // Update widget
    emit scannedTracks(trackList);
}

void xMusicLibrary::scan() {
    // Cleanup before scanning path
    musicFiles.clear();
    try {
        std::list<QString> artistList;
        for (const auto& artistDir : std::filesystem::directory_iterator(baseDirctory)) {
            const auto& artistPath = artistDir.path();
            if (!std::filesystem::is_directory(artistPath)) {
                // No directory, no artist.
                continue;
            }
            auto artistName = QString::fromStdString(artistPath.filename());
            std::map<QString, std::list<std::filesystem::path>> artistAlbumMap;
            // Read all albums for the given artist.
            for (const auto& albumDir : std::filesystem::directory_iterator(artistPath)) {
                const auto& albumPath = albumDir.path();
                if (!std::filesystem::is_directory(albumPath)) {
                    // No directory, no album.
                    continue;
                }
                auto albumName = QString::fromStdString(albumPath.filename());
                qDebug() << "xMusicLibrary::scanLibrary: artist/album: " << artistName << "/" << albumName;
                // Do not scan the files in each directory as it is too costly.
                // Instead add the album path as first element element to track list.
                // It will be later used to read the tracks on demand.
                std::list<std::filesystem::path> trackList;
                trackList.push_back(albumPath);
                artistAlbumMap[albumName] = trackList;
            }

            musicFiles[artistName] = artistAlbumMap;
            artistList.push_back(artistName);
        }
        // Update widget
        emit scannedArtists(artistList);
    } catch (...) {
        // Error scanning. Structure may not be as expected.
        // Clear everything currently scanned.
        musicFiles.clear();
    }
}

std::list<std::filesystem::path> xMusicLibrary::scanForAlbumTracks(const std::filesystem::path& albumPath) {
    std::list<std::filesystem::path> trackList;
    // Add the album path itself
    trackList.push_back(albumPath);
    // Add all files in the album path
    for (const auto& trackFile : std::filesystem::directory_iterator(albumPath)) {
        if (std::filesystem::is_regular_file(trackFile)) {
            trackList.push_back(trackFile.path());
        }
    }
    return trackList;
}