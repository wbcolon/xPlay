# xPlay

## Overview

xPlay is a music player designed for large libraries that may be accesed through a Samba or NFS share.
Scanning every file is therefore very time consuming. xPlay therefore makes a number of assumptions
about the structure of your music library. The layout needs to be as follows.

* artist/album/track

Examples:

* abstrakt algebra/i/01 stigmata.flac
* abstrakt algebra/ii/09 enigma.flac
* iommi/fused/01 dopamine.flac

I did not like the current media player that try to scan and analyze each and every one of my
over 60000 music files. The scanning takes way to long and does not have any benefit for me.

## Library scanning

xPlay takes advantage of a structured music library structure. The inital scan only reads the artists and album
directories in order to avoid unnecessary delays. The files (tracks) within "artist/album" will only be scanned
on demand, if an artist and corresponding album have been selected. The tracks will not be scanned or analyzed in
order to retrieve any media tags. Only the file names are of interest.
The scanning is so fast that there is not database of scanned entries. The scanning is perfomed each time on
startup. It takes only a few seconds for my setup.

## Player Backends

### Qt

The Qt backend is easy to implement using the classes QMediaPlayer and QMediaPlaylist. The main disadvantage
is the missing gapless playback.

### Phonon

The Phonon/KDE backend supports gapless playback, but it is a bit more complicated to use. On top, there seems
to be an issue calculating the length of the currently played track. I worked around this issue by utilizing
the a muted QMediaPlayer object.

## Usage

xPlay has a simple UI layout with a minimun set on functionalities that I want from my music player. The purpose
is to navigate a huge music library and play the songs.

The main screen has four vertical list for the artists, album, tracks and the queue. The album list is updated
if you click (or select) on an artist and the track list is updated if you click (or select) on an album. If you
double-click on a track in the track list, then the track and the following tracks of the list are added to the
queue. If you right-click on a track in the track list, then only this track will be added to the queue.
The horizontal artist selector list can be used to filter the artist list by their first character. The filtering
is removed if you click on *none*.

The player itself displays the artist, album and track currently played. It includes a slider to seek within the
currently played file. In addition there is player control section with a *play/pause*, *stop*, *prev*, *next*
and *clear queue* buttons and a volume dialer. If you double-click on an entry of the queue then the player jumps
to this track. Right-clicking on a queue entry will remove this track from the queue.

The file menu has currently two entries. An *Exit* and *Open Music Library* entry. The *Open Music Library* will
display a file dialog that allows you to select a base directory for your music library. This directory is stored
using QSettings and is retrieved on startup of xPlay.

## Requirements

* Qt 5.x
* Qwt 6.x for Qt5 (optional, deactivate with USE_QWT=OFF)
* Phonon (optional, deactivate with USE_PHONON=OFF)
* C++17

With Qwt the UI has a slightly improved volume knop and track slider.

## Notes

xPlay started out as a little project over the weekend in order to evaluate the Clion C++ IDE.
