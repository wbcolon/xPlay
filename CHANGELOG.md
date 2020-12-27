# Changelog

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
