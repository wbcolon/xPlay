# Changelog

## Unreleased

- Switched to QT6. 
- Switched back to phonon-kde for music and movie player due to wayland issues.
- TODO: no support for visualization.
- TODO: wayland performance not optimal (use xwayland instead).


## 0.16.0 - 2024-07-21

- Replaced classic menu entries with main and view dependent settings menu.
- Add info mode (local or remote music library) for music player.
- Add support for played level indicators bronze, silver and gold.
- Add support for actual played time in music and movie player.
- Update database only if music or movie is considered played.
- Update configuration dialog to support new functionality.
- Fix unknown entries function.
- Fix BluOS player issue with certain paths.
- Code improvements and refactorings.
- Stability improvements.


## 0.15.0 - 2023-07-02

- Add full window mode for music visualization.
- Fix default audio/subtitle setting for movie files.
- Fix crashes when switching libraries without properly configured music libraries.
- Enable reindex menu entry only if BluOS player library is used.
- Improve switch times in movie view when scanning movie files.
- Improve error handling for movie player.
- Disconnect Rotel amp controls when closing.
- Various small bug fixes.


## 0.14.4 - 2023-02-20

- Add BluOS icon when playing from BluOS player library.
- Improve queue handling for BluOS player libraries.
- Fix bug if reply from Rotel amp ends with amp:freq command sequence.
  - Added tests for handling reply message.
- Fix bug in update of reindexing status dialog.
- Disable Sorting Latest for BluOS player libraries. Not file date/time for available.
- Fix audio volume bug when seeking within movie file.
- Add compressed audio plugin for movie player to configuration.


## 0.14.3 - 2023-02-04

- Adjust shuffle mode for BluOS player library.
  - Allow enabling shuffle mode if queue not empty.
  - Do not queue additional tracks if shuffle is enabled.
- Reset position after stop() for BluOS player library.
- Fix status for prev(), next() and seek() for BluOS player library.
- Fix queue handling for BluOS player library.
- Fix tests.


## 0.14.2 - 2023-01-31

- Remove Qwt dependency.
  - Use QDial with embedded QLCDNumber as a volume knob.
  - Use QSlider with self drawn scale.
- Fix movie player default language/subtitle auto-select.
- Movie player code and UI cleanups.
- Rotel widget and controls code split.


## 0.14.1 - 2023-01-21

- Restore music player volume when switching between local and BluOS player library.
- Use separate timer for BluOS player library reindexing.
- Disable repeat queue for BluOS player library.
- Extract partial track info for BluOS player library.
- Disable music visualization for BluOS player library (missing music data feed).
- Fix scanning progress when interrupting a full scan by switching to BluOS player library.


## 0.14.0 - 2023-01-04

- Add support for BluOS player libraries.
- Add mobile library IO for mobile sync view.
- Add improved sync functionality to music library.
- Add jump to artist if hovered over selector while ctrl is pressed.
- Add splitter for more flexible UI.
- Add streaming view submenu.
- Fix pulseaudio volume control by determining default pulseaudio sink.
- Disable audio visualization if configuration file not valid.
- Minor bug fixing.


## 0.13.0 - 2022-08-22

- Many UI updates.
- Add music library scanning progress.
- Add audio compression option for movie player.
- Use default pulseaudio sink instead of application stream for volume control.
- Add configuration option for website user agent.
- More libvlc audio fixes.
- Some code cleanup.


## 0.12.1 - 2022-06-05

- Add filesystem sync after the copy/remove operations in mobile sync view.
- Fix bug in compare of music libraries affecting mobile sync view.
- Only ignore . and .. directories for music library.


## 0.12.0 - 2022-05-15

- New music library implementation.
- Support renaming of music files, the corresponding database entries and update of tags.
- Fix minor UI issue if last single track is dequeued.
- Add movie filter and other movie view UI changes.
- Implement workaround for VLC audio issue after seeking within a movie.


## 0.11.1 - 2022-02-22

- Fix toggle play bug in movie player when pressing stop button twice.
- Drop "surround" description from the audio channels in movie player.
- Add codec description as tooltip for audio channels in movie player.
- Fix phonon playback issue when queueing tracks after the last track is about to finish.


## 0.11.0 - 2021-12-27

- Remove Phonon based movie player.
- Add chapter support for movie player.
- Add deinterlace support for movie player.
- Replace scale and crop with crop option.
- Remove Qt based music player.
- Make Qwt based player widgets the default. Remove corresponding Qt based player widgets.
- Fix switching to visualization if in artist info view.
- Update UI for mobile library sync.
- Some mobile library sync fixes.


## 0.10.3 - 2021-12-11

- Disable music library scanning while syncing mobile library.
- Add visualization toggle when double-clicking on music player widget.
- Add artist filter for music and mobile library.
- Fix incorrect name visualization preset output.
- Fix tree expansion mode, when updating music or mobile library tree.
- Fix segfault caused by thread based update of track time.


## 0.10.2 - 2021-11-25

- Reduce the sample rate for music visualization.
- Fix clear for artist and search selector.
- Minor code cleanups and documentation updates.
- Fix and add additional tests.
- Add support for mixed-case music libraries.
- Handle music visualization errors better.
- Select next visualization preset on ctrl+double-click.


## 0.10.1 - 2021-11-17

- Add played time to window title in full window movie playback.
- Hide mouse cursor in full window movie playback.
- Add shortcuts for Music View menu items.
- Minor UI updates for Mobile Sync view.
- Minor fixes for selectors when rescanning music library.
- Add music visualization using projectM.


## 0.10.0 - 2021-11-07

- Add initial mobile sync view.
- Add support for file sizes and total sizes in music library.
- Change vout for libVLC based movie player.
- Improve performance of list widgets used.


## 0.9.3 - 2021-10-08

- Add support for sorting artists and albums after last write access time.
- Add optimizations for xPlayerListWidget.
- Add initial tests.


## 0.9.2 - 2021-08-04

- Replace close pushbutton with close icon tool button.
- Add filter for list widgets.
- Add select option for local filter and selector widget.
- Update QWT slider widget with LCD numbers.
- Fixed bug in control of Rotel amp.
- Improve adding tracks to the queue. Show progress bar and possibly warning dialog.


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
