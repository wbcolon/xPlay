# Changelog

## Unreleased

- Replace close pushbutton with close icon toolbutton.


## 0.9.1 - 2021-06-13

- Replace libsoci with libsqlite3.
- Fix subtitle and audio channel defaults for VLC movie player.


## 0.9.0 - 2021-05-30

- Add tag functionality.
- Add drag-and-drop move support for queue.
- Add artist transitions to artist popup menu.
- Minor UI fixes.


## 0.8.1 - 2021-05-23

- Fix UI resizing issue affecting queue list.
- Improve performance for queueing tracks.
- Fix segfault if audio properties are not available.


## 0.8.0 - 2021-05-16

- Add artist info for music view.
- Add zoom factor to streaming view.
- Add default zoom factor in configuration.
- Fix album initializer for music file.
- Added feature that selects artist and album for queued track (ctrl+click).
- Improve album and search selectors.


## 0.7.2 - 2021-05-13

- Added busy cursor for artist and selector queue operations.
- Improved database overlay update for played tracks.
- Fix audio and subtitle default selection.
- Exit if dbus command cannot connect to dbus interface of xPlay (not running).
- Minor fixes.


## 0.7.1 - 2021-04-24

- Added limited error handling for music library scanning.
- Added option for default audio and subtitle language for movies.
- Minor configuration dialog UI layout fixes.
- Clang-tidy fixes.


## 0.7.0 - 2021-04-06

- Added movie player implementation using libVLC.
- Added music library filter class.
- Added music file wrapper class.
- Added custom list widget with track time display.
- Added database based selection to the album selector.
- Added search based selector.
- Added tabwidget for artist, album and search selectors.
- Added database album transition table (transitions currently recorded, but not used).
- Fix thread cleanup if rescan or quit before music library scanning finished.


## 0.6.4 - 2021-03-16

- Minor UI adjustments to fit HD screen.
- Fix for play/pause control buttons.


## 0.6.3 - 2021-03-15

- Add sync between database and library for music and movie.
- Add balance slider for rotel widget.
- Add "random" artist selector.
- Some UI layout adjustments.


## 0.6.2 - 2021-01-31

- Update error recovery code for phonon music player.
- Fix queue issue if single track is added.
- Handle database update errors.
- Update configuration dialog. Add options for path to database and to ignore database update errors.
- UI fixes for selectors and streaming view.


## 0.6.1 - 2021-01-06

- Fix autoplay and queue.


## 0.6.0 - 2021-01-06

- Add playlist support for music view.
- Improve scanning for artist and selector queueing.
- Improve shuffle mode.
- Minor Layout adjustments.
- Autoplay bugfix for Phonon bases music player.
- Update known issues.


## 0.5.4 - 2020-12-31

- Add argument for starting xPlay maximized.
- Fix source string for PC-USB input.
- Fix bug of selecting subtitle if no description exists (VLC backend).
- Use fixed width for the volume knob.


## 0.5.3 - 2020-12-30

- Queue album by double-click on album entry.
- Add album selector in order to filter the albums for a given artist.
- The album filtering also applies when queueing an artist or artist selector.


## 0.5.2 - 2020-12-29

- Add shuffle-mode for the queue. Deque and double-click on queue track are disabled.
- Improved database usage.


## 0.5.1 - 2020-12-27

- Adjusted music view layout.
- Queue all artists matching selector on double-click.


## 0.5.0 - 2020-12-26

- Add database for played music and movie files.
- Add configurable database layer for music and movie view.


## 0.4.4 - 2020-12-21

- Display currently played movie in the window title if in full window mode.
- Fix minor rounding issue in times displayed in slider.


## 0.4.3 - 2020-12-12

- Add autoplay next movie feature. Updated movie player UI.
- Allow enable/disable of Rotel widget in configuration dialog.


## 0.4.2 - 2020-12-09

- Do a proper UI cleanup on rescanning the movie or music library.
- Enable or disable the Rotel widget based on the network connection.


## 0.4.1 - 2020-11-22

- Add queue artist feature. Double-click on artist to queue all albums.
- Hide menu bar in full window mode.
- Change mute behavior of volume widget. Single-click to "Volume".
- Some cleanup (clang-tidy).


## 0.4.0 - 2020-11-14

- Add streaming view using QWebEngine based browser.
- Add simple DBus interface for remote control.
- Updated configuration and corresponding dialog.
- Updated volume and Rotel widget.
- Some bugfixes.


## 0.3.2 - 2020-11-02

- Add scale and crop option for movies.
- Bugfix for phonon music player.
- Documentation update.
- Code cleanup.


## 0.3.1 - 2020-11-01

- Movie library scanning now threaded. Faster startup.
- Bugfixes
- Code cleanup and rework


## 0.3.0 - 2020-10-31

- Added a movie player view, movie library scanning.
- Bugfixes


## 0.2.0 - 2020-10-27

- Added control for a Rotel A14 amp.
- Updated library scanning (background scanning of album tracks).
- Bugfixes


## 0.1.1 - 2020-10-24

- Bugfixes


## 0.1.0 - 2020-10-24

- Initial Release
- Functional music player including gapless playback (requires Phonon).
- Simple UI interface for large libraries.
