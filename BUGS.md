# Known Issues

- mute volume directly after startup (possibly before playback) does not work.
- update Problems when switching views while playing (e.g. movie view, heavy load).
- changing font sizes while running causes UI layout issues.


# Fixed
- if queue is played and then songs are added then the last song is autoplayed. It should start with the newly added (phonon,qt)
- if playlist is at the end then the player does not go into the stop state (phonon,qt)
- player at last queue song (still playing). Songs added to the queue are not played after last song finished.
- movie files stopping after the end of playback sometimes problematic.
- rescan of music library while first scan active does not work properly.
- mixed case naming schemes may cause issues.


# Error handling
- player sometimes seems to halt "source ignored..." -> maybe internal issue. -> try to recover.

