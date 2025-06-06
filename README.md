# xPlay

**Note:** The current Qt6 port is a work-in-progress.

## Overview

xPlay is a music/movie player designed for large music/movie file libraries that may be accessed through a 
Samba or NFS share. Scanning every file is very time-consuming which may cause significant delays. 
xPlay therefore makes a number of assumptions about the structure of your music or movie library. 
The directory structure of the music library needs to be as follows.

* artist/album/track

*Examples:*

* abstrakt algebra/i/01 stigmata.flac
* abstrakt algebra/ii/09 enigma.flac
* iommi/fused/01 dopamine.flac

The movie library is set up differently. It is possible to define tags to which base directories can be
attached. The movie library scanner will not do a full recursive scan. It will scan only one subdirectory
further.

*Examples:*

* [movies] /extern/movies-dvd/filme
* [movies] /extern/movies-bd/filme
* [shows] /extern/movies-dvd/serien
* [dokus] /extern/movies-bd/dokumentationen

See the [changelog](CHANGELOG.md) for the latest development progress.

### Remarks

I was unhappy with the current media players that try to scan and analyze each one of my
personal music library that contains over 65000 files. The scanning took way to long and did not have
any benefits for me.

## Library scanning

The music and movie library are scanned on every startup. The scanning functionality itself is threaded in
order to avoid any UI startup delays.

### Music library

xPlay takes advantage of the music library structure. The initial scan only reads the artists and album
directories. The scanning is so fast that there is no need for a database of scanned library entries. It
therefore can be performed each time on startup. The music view is updated after this initial scan
is complete. The files (tracks) within "artist/album" will only be scanned on demand and cached, if an
artist and corresponding album have been selected. The music library scan continues in the background in
order to fill the "artist/album" file (tracks) cache. The tracks itself will initially not be scanned or analyzed
in order to retrieve any media tags. They will only be analyzed if necessary. Until then only the file names are
of interest. xPlay does support remote access to BluOS player libraries. The Player itself has to be fully configured
and the folder structure for its library has to match the artist/album/track structure.

*Note:* The initial scan takes only a few seconds for my setup (Raspberry Pi 4 as Samba server).

### Movie library

The scanning of the movie library is significantly different due to the fact that there is less structure.
xPlay supports only one subdirectory in the movie library base. All subdirectories and movie files are
grouped by a tag. Each tag itself can have multiple directories associated. The movie scanning assumes that the
movie files in each of the subdirectory are distinct. No duplicate names are allowed. Earlier entries will be
overwritten and previous movie file is not accessible.

## Player Backends

### Phonon

The Phonon/KDE backend supports gapless playback, but it is a bit more complicated to use. On top, there seems
to be an issue calculating the length of the currently played track. I worked around this issue by utilizing
a muted QMediaPlayer object. The Phonon backend is used for the music player and for the movie player due to VLC 
issues with Qt6 and Wayland.

### PulseAudio

The volume changes are applied to the default PulseAudio sink, not to the xPlay application stream. If the volume
is muted then the PulseAudio sink and the application stream are both muted. The volume for the application stream
is set to a 100 percent.

### BluOS player

BluOS does not support moving files within the playlist. Drag and drop within the queue is therefor disabled if a
BluOS player library is used. The volume changes are applied to the BluOS player volume, not the PulseAudio sink.

## Usage

xPlay has a simple and easy to use UI interface that provides the set on functionalities that I expect from a
music and movie player. The main purpose is to navigate a huge library and play the songs or movies and display
the relevant information.

xPlay has a music view, a movie view, a streaming view and a mobile sync view.

### Music View

![Screenshot Music View (empty queue)](screenshots/xplay_screenshot_music_view_00.png)
![Screenshot Music View (full queue)](screenshots/xplay_screenshot_music_view_001.png)
![Screenshot Music View (shuffle)](screenshots/xplay_screenshot_music_view_01.png)
![Screenshot Music View (database)](screenshots/xplay_screenshot_music_view_02.png)
![Screenshot Music View (search)](screenshots/xplay_screenshot_music_view_03.png)
![Screenshot Music View (playlists)](screenshots/xplay_screenshot_music_view_04.png)
![Screenshot Music View (tags)](screenshots/xplay_screenshot_music_view_05.png)
![Screenshot Music View (tag popup)](screenshots/xplay_screenshot_music_view_06.png)
![Screenshot Music View (artistinfo popup)](screenshots/xplay_screenshot_music_view_07.png)
![Screenshot Music View (artistinfo)](screenshots/xplay_screenshot_music_view_08.png)
![Screenshot Music View (rename)](screenshots/xplay_screenshot_music_view_10.png)
![Screenshot Music View (bluos - empty queue)](screenshots/xplay_screenshot_music_view_11.png)
![Screenshot Music View (bluos - full queue)](screenshots/xplay_screenshot_music_view_111.png)

The main screen of the music view has four vertical list for the artists, album, tracks and the queue. The
album list is updated if you click (or select) on an artist and the track list is updated if you click (or select)
on an album. If you double-click on a track in the track list, then the track and the following tracks of the list
are added to the queue. If you right-click on a track in the track list, then only this track will be added to the
queue. Then entire album can be queued by a double-click on the corresponding album entry. A double-click on an
artist entry will queue all albums for the clicked artist.

The queue does support a shuffle mode in which the queued tracks will be played at random. The jump to an individual
track in the queue or the deque of a single track is not supported in shuffle mode. Each track of the queue has a
tooltip showing the artist and album the track belongs to. The playlist dialog box will open upon pressing the
*PlayList* button at the bottom of the queue. It allows to save the current queue as playlist into the database for
later use. The dialog allows for loading playlists that are stored in the database or remove any of the stored
playlists (including their entries).

The database overlay (if activated) marks every artist, album and track with a star (*) if it has been played within the
configured time period (see configuration dialog). For each track a tooltip is added that shows how many times this
track has been played and the last time it was played. The overlay makes it easier to identify music that you have not
listened too in some time or at all.

The player itself displays the artist, album and track currently played. Additional information such as sample rate, 
bits per sample and bitrate are only displayed for local music libraries. BluOS does not provide access to this
information. It includes a slider to seek within the currently played file. In addition, there is player control 
section with a *play/pause*, *stop*, *prev*, *next* and *clear queue* buttons and a volume dialer. If you 
double-click on an entry of the queue then the player jumps to this track. Right-clicking on a queue entry will 
remove this track from the queue.

The artist popup menu can be accessed through a context menu by a right-click on an entry in the artist list. The 
menu entry *Link To Website* leads to the artist info which is a small simple web browser with minimal navigational 
control buttons such as home, back and forward as well as selecting a zoom factor. Pressing the *close* button will end
the artist info view. A white filled star icon indicates that no URL is currently stored within our database. A grey
filled star indicates that the URL was taken out of our database. Clicking on a white filled star will store the current
URL to the database. The database entry is removed if we click on a grey filled star.
The artist popup will also show other artists in the *Other Artists* section that have been played before or after 
the selected artist in the past. You can jump to this artist by selecting the corresponding artist popup menu entry.

Tags can be added and removed by the tag popup menu which can be accessed through a context menu by a right-click on an
entry in the track list or the queue. The tags are stored in the database. The tag dialog can be accessed by pressing 
the *Tags* button at the bottom of the queue list. The tag dialog allows to load or extend the queue with tracks that
have a specific tag.

The artist, album and track name can be modified. If the queue is empty and no music file is playing then a
shift-right click on an artist, album or track entry will open a popup menu that allows us to rename the
entry. The corresponding directory (for artist and album) or the file name (for track) will be renamed. The tags of
all affected music files will be updated as well as any matching database entries.

The Rotel widget allows to control a Rotel A12 or A14 amp via a network connection. The volume can be adjusted
(maximum of 60) and the input can be selected. The values for bass and treble can be adjusted (from -10 to +10)
as well as the balance (from -5 to +5).

#### Visualization

**TODO:** The music visualization is currently not supported for Qt6.

![Screenshot Music View (visualization)(1)](screenshots/xplay_screenshot_music_view_09.png)
![Screenshot Music View (visualization)(2)](screenshots/xplay_screenshot_music_view_091.png)
![Screenshot Music View (visualization)(3)](screenshots/xplay_screenshot_music_view_092.png)

A music visualization based on projectM is available and can be activated via the *Visualization* entry of the 
settings menu on the right side. The *Visualization Mode* allows you to select the central window or a small window mode.
The visualization (when activated) is displayed whenever the music player is playing and the artist info view is
currently not activated. The visualization (when displayed) reverts to the artist, album and track view whenever the 
music player pauses or stops. A right-click on the visualization opens a popup menu that allows selecting a projectM 
preset. A double-click within the visualization will toggle a full window mode. This feature is only available for if 
the central window visualization mode is selected. A ctrl-left-click will display the name of the currently selected 
preset. A double-click on the player widget itself will toggle the music visualization. 

**Note:** The music visualization is not supported for the BluOS player music library.

#### Selectors and Filters

The selectors are located at the bottom of the music view and include an artist selector, an album selector and
an a search bar. A filter is available that can be applied to the artist, album or track list. 
The selectors as well as the filters can be made accessible through the settings menu.


##### Artist Selector

The artist selector list can be used to filter the artist list by the first character. The filtering is removed if
you click on *none*. If a selector is double-clicked then all selected artists with their albums are queue. A
double-click on *none* will queue the entire music library. The artist selector *random* will randomize the list of
artists. A double-click on *random* will randomize the artist list again and not add any tracks to the queue.
The *Sorting Latest* checkbox will sort the artists and albums according to the timestamp they were last modified.
The tracks ordering is not affected by this option. The checkbox is cleared whenever the *random* selector is chosen.

##### Album Selector

The album selector can be used to filter the displayed albums for an artist. The individual selectors can be modified
by double-click. A selector in white is currently not used. Selectors in green (at least one of them) must be found as
part of the album name. Red represents not matched selectors. An album cannot contain any of the selectors in red as part 
of its album name. The *database* selector uses a mapping of artist and albums are stored in the databases and that have
been played after the given timestamp. The timestamp can easily be adjusted directly or by using the *Now* button
(current time) or the *Begin* button (beginning of the database). The *database* selector allows to easily filter for
albums that have never been played or that have recently been played. The *none* selector clears any filtering.

The filtering done by the album selector is applied whenever a selector state changes. The filtering is performed
on the music library level meaning that artists that do not have any albums that pass the album selector will be
removed from the artist list. The artist selector list is updated based on this filtered list of artists.

*Examples*

* Marking the *database* in red and marking *[hd]* in green will result in all albums being displayed that contain
  the *[hd]* tag and that have not been played since the given timestamp in the album selector.
* Marking the *(live)* in red and then double-click on an artist will queue all albums except the ones
  containing *(live)*.
 
##### Search

The search bar can be used to find an artist, album or track that contains a given substring. Put in the search text
and decide on artist, album or track to search for and then press the *Search* button to activate the filter. The
*Clear* button will remove any search based filtering. When searching for a specific track in your library only the
artists and albums are displayed that contain a track matching the search text. All tracks of the album will be
displayed, not only the tracks matching. The search based filtering will be added on top of the album selector based
filtering. It is therefore possible to search for a specific track that has not yet been played and that contains the
*(live)* tag.

##### Filters

The filter is applied on the list widget itself and therefore can be combined with the selectors. A list widget item
is shown if it matches the given filter string.


### Movie View

![Screenshot Movie View](screenshots/xplay_screenshot_movie_view_00.png)
![Screenshot Movie View](screenshots/xplay_screenshot_movie_view_001.png)
![Screenshot Movie View (fullwindow)](screenshots/xplay_screenshot_movie_view_01.png)
![Screenshot Movie View (database overlay)](screenshots/xplay_screenshot_movie_view_02.png)

The main screen of the movie view has three vertical lists for tags, directories and movies. A tag is a
representation of one or more base directories. The directory list contains all subdirectories (only one level)
of these. An additional entry "." is added for all movies that are not located within a subdirectory. The
movie list displays all movie files, but this list does not act as a queue. Double-click on an entry in the
movie list will start the playback. The database overlay (if activated) will mark every tag, directory and movie
with a star (*) if it has been played within the configured time period. For each movie a tooltip is added that
displays the number of times this movie has been played and the last time it was played. The overlay helps keeping
track which episode of a show you have already seen.

The player section itself allows for seeking within the movie and selecting the audio channel or subtitle. The
control section has a *play/pause* and *stop* buttons. The *rew* and *fwd* buttons jump 60 seconds backward or
forward in the movie. The *full window* button maximizes the video output window. The currently played movie will
be displayed in the window title. The *scale and crop* checkbox may be usable for some movies that have black borders.
It is activated by default for the VLC based movie player. The video output window can also be toggled by a
double-click. In addition, you can rewind and forward by 60 seconds using the left and right arrow keys. The up and down
arrow keys will increase or decrease the volume by one. The *S* key will toggle the scale and crop mode. The *ESC* key
can be used to end the full window mode. The full window mode will automatically end if the current movie is about to
end. If the *Autoplay Next* in the *Options* menu is enabled then the next movie in the movie list will be played as 
soon as the current movie has ended. The player will stay in fill window mode until the last movie of the list is 
finished.

##### Filters

The movie filter is similar to the filters for the artist, album or track filter in the music view.
The filter is applied on the movie list widget itself. A list widget item is shown if it matches the given 
filter string. All entries will appear if the movie filter is disabled in the settings menu.


### Streaming View

![Screenshot Streaming View (qobuz)](screenshots/xplay_screenshot_streaming_view_00.png)
![Screenshot Streaming View (youtube)](screenshots/xplay_screenshot_streaming_view_01.png)

The main screen of the streaming view is basically a simple web browser using the QWebEngine. On the right side
xPlay has a number of controls. The controls include a combo box which allows us to select in between a set of
preset websites (see configuration dialog). Data for the sites, including history, cache and cookies can easily
be removed. The navigation section allows for a very basic website navigation. A control for the Rotel amp is also
available. The browser is limited by the capabilities of QWebEngine, e.g., Netflix did not work for my setup. This
may change for coming versions of QWebEngine.

### Mobile Sync View

![Screenshot Mobile Sync View](screenshots/xplay_screenshot_mobile_sync_view_00.png)
![Screenshot Mobile Sync View (Mark Missing)](screenshots/xplay_screenshot_mobile_sync_view_01.png)
![Screenshot Mobile Sync View (Sync Operations)](screenshots/xplay_screenshot_mobile_sync_view_02.png)
![Screenshot Mobile Sync View (Mark Existing)](screenshots/xplay_screenshot_mobile_sync_view_03.png)
![Screenshot Mobile Sync View (Progress)](screenshots/xplay_screenshot_mobile_sync_view_04.png)

The main screen of the mobile sync view is designed to sync a subset of the music library to the so-called mobile
library which is usually located on some external storage. The directory for the mobile library can input directory or 
conveniently selected via a file dialog when pressing the *Open* button. Press the *Scan* button afterwards to 
initiate a scan of the mobile library. The widget is updated as soon as the scanning process is finished.
The music library can be sorted alphabetical by artist name or by total size for each artist if the *Sort by Size*
checkbox is selected. Pressing the *Compare* button will initiate a compare of the music and the mobile library in order
to mark missing artists, albums or tracks in red. Partially missing artists or albums are marked using a lighter 
background pattern. The content of a mobile library can be saved into a so-called existing elements set. This is 
especially useful if the mobile library is stored on multiple external storage devices. Using the *Mark* popup menu the
current subset of the mobile library can be saved and all existing elements can be marked in green. These markings help
to avoid duplicates among multiple external storage devices.

A ctrl+right-click to an element of the music library will add the corresponding artist, album or track to the
*Add to Mobile Library* list. A ctrl+right-click to an element of the mobile library will add the corresponding
element to the *Remove from Mobile Library* list. The impact of the listed changes to the mobile library will be
displayed in a progress bar. The progress bar changes color from green to red in case there is no sufficient storage
space for the mobile library. Pressing the *Apply* button will perform the removal operations before the add operations.
A progess bar will show display the progress of the sync operation. A blocking filesystem sync will be performed after
the removal/copy operations. The re-scanning of the music library is disabled during the sync operation.


### Menu

The menu has two entries, a main menu on the left side and a view dependent settings menu on the right side. 
The main menu hase seven entries. A *Configure* entry will open configuration dialog (see below).
The entries *Select Music View*, *Select Movie View*, *Select Streaming View* or *Select Mobile Sync View* allow 
to switch between the different views. The entry *About Qt* displays copyright information about the used 
Qt library and *Exit* ends xPlay.
The settings menus contains view dependent entries that include entries such as *Rescan Library* and 
*Check Database*. Checking the database will verify the database entries against the music or movie library and display
any unknown entries in a dialog box. These entries can be removed.


#### Configuration Dialog

The configuration dialog is using QSettings to load and store the xPlay configuration. The directory and
extensions for the music and the movie library can be configured as well as the sites for the streaming view. For
movies, the default audio and subtitle language can be selected. The Rotel widget can be enabled or disabled and its
network connection can be configured. The database overlay for the music and movie view can be configured using
individual check boxes. A cut-off date can be set for each database query. If it is specified then entries with a time
stamp before the cut-off date are ignored. This features enables the user to e.g. display which movies he has seen 
the last month. A play level indicator allows to customize the color of the indicator star dependent on the play count.
The *save* button will be disabled if the following contraint *bronze <= silver <= gold* is not satisfied by the
configured played levels.

![Screenshot Configuration Dialog (Music)](screenshots/xplay_screenshot_configuration_dialog_00.png)
![Screenshot Configuration Dialog (Movie)](screenshots/xplay_screenshot_configuration_dialog_01.png)
![Screenshot Configuration Dialog (Streaming)](screenshots/xplay_screenshot_configuration_dialog_02.png)
![Screenshot Configuration Dialog (Additional)](screenshots/xplay_screenshot_configuration_dialog_03.png)


## Artwork

The media control artwork was borrowed from Wikipedia (https://en.wikipedia.org/wiki/Media_control_symbols)
and the KDE 5.x breeze icons.

## Requirements

* Qt 6.x (https://www.qt.io/)
* Phonon (https://github.com/KDE/phonon)
* SQLite3 - The Database Access Library (https://sqlite.org/)
* TagLib Audio Meta-Data Library (https://taglib.org/)
* libcurl Library (https://curl.se/libcurl/)
* PugiXML Library (https://pugixml.org/)
* libVLC Library (https://wiki.videolan.org/LibVLC/)
* libpulse Library (https://freedesktop.org/software/pulseaudio/doxygen)
* projectM Library (https://github.com/projectM-visualizer/projectm)
* C++17 (clang or gcc)

## Known Issues

* Ending the Application before the library scanning threads are finished will result in an abort return code.
* Phonon issues
    * The GStreamer backend is unfortunately non-functional for Qt6.
    * The VLC backend does not support visualization.
    * The VLC backend performance is poor for wayland output.
* PulseAudio
    * Changing the default audio sink may not be recognized.
    * Changing the volume outside xPlay is not recognized by xPlay.

## Notes

xPlay started out as a little project over the weekend in order to evaluate the Clion C++ IDE.

**Please support artists by buying their products.**
