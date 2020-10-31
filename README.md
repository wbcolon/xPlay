# xPlay

## Overview

xPlay is a music/movie player designed for large libraries that may be accessed through a Samba or 
NFS share. Scanning every file can therefore be very time consuming. xPlay therefore makes a number 
of assumptions about the structure of your music library. The layout needs to be as follows.

* artist/album/track

Examples:

* abstrakt algebra/i/01 stigmata.flac
* abstrakt algebra/ii/09 enigma.flac
* iommi/fused/01 dopamine.flac

The movie library is setup differently. You can define tags to which you can attach base directories.
The movie library scanner will not do a recursive scan. It will only go down one level further.

Example:

* [movies] /extern/movies-dvd/filme
* [movies] /extern/movies-bd/filme
* [shows] /extern/movies-dvd/serien
* [dokus] /extenn/movies-bd/dokumentationen

### Remarks

I did not like the current media player that try to scan and analyze each and every one of my
over 60000 music files. The scanning takes way to long and does not have any benefit for me.

## Music Library scanning

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
the a muted QMediaPlayer object. The Phonon backend in required for movie player as it supports audio channels
and subtitles.

## Usage

xPlay has a simple UI layout with a minimal set on functionalities that I want from my music and movie player. 
The purpose is to navigate a huge libraries and play the songs or videos. The UI has a music player and a movie
player view.  

The main screen of the music player view has four vertical list for the artists, album, tracks and the queue. The 
album list is updated if you click (or select) on an artist and the track list is updated if you click (or select) 
on an album. If you double-click on a track in the track list, then the track and the following tracks of the list 
are added to the queue. If you right-click on a track in the track list, then only this track will be added to the 
queue. The horizontal artist selector list can be used to filter the artist list by their first character. The 
filtering is removed if you click on *none*.

The player itself displays the artist, album and track currently played. It includes a slider to seek within the
currently played file. In addition there is player control section with a *play/pause*, *stop*, *prev*, *next*
and *clear queue* buttons and a volume dialer. If you double-click on an entry of the queue then the player jumps
to this track. Right-clicking on a queue entry will remove this track from the queue.

The main screen of the movie player view has three vertical lists for tags, directories and movies. A tag is a 
representation of one or more base directories. The directories list contains all sub directories (only level one)
of each these. An additional entry "." is added for all movies that are not located within a sub directory. The 
movies list displays all movie files, but does not act as a queue. Double-click on an entry in the movies list
will start the playback. 

The player section itself allows for seeking within the movie and selecting the audio channel or subtitle. The
control section has a *play/pause* and *stop*" buttons. The *rew* and *fwd* buttons jump 60 seconds backward or 
forward in the movie. The *full window* button maximizes the video output window. The video output window can
also be toggled by a double-click. In addition you can rewind and forward by 60 seconds using the left and right
arrow keys. The up and down arrow keys will increase or decrease the volume by one. The ESC key can be used to
end the full window mode. The full window mode will automatically end if the current movie is about to end.

The file menu has currently two entries. An *Exit* and *Configure* entry. The *Configure* will
display a file dialog that allows you to select a base directory for your music library. This directory is stored
using QSettings and is retrieved on startup of xPlay.

## Artwork

The media control artwork has been borrowed from Wikipedia.
(https://en.wikipedia.org/wiki/Media_control_symbols)

## Requirements

* Qt 5.x
* Qwt 6.x for Qt5 (optional, deactivate with USE_QWT=OFF)
* Phonon (optional, deactivate with USE_PHONON=OFF)
* C++17

With Qwt the UI has a slightly improved volume knop and track slider.

## Notes

xPlay started out as a little project over the weekend in order to evaluate the Clion C++ IDE.
