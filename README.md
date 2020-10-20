# xPlay

## Overview

xPlay is a music player designed for large libraries that are on a Samba or NFS share. Scanning
every file is very time consuming. The player makes a number of assumptions about the structure 
of your music library. The layout is as follows

```
artist/
        album/
                track
                track
                ...
        album/
                track
                ...
        ...
...
```

I was so tired about these modern media player that try to scan and analyze each and every of my
over 60000 music files. This takes way to long and does not have any benefit.

## Beware

Certain things may be hardcoded until I get around completing the UI.

## Note

xPlay started out as a little project over the weekend in order to evaluate the clion C++ IDE.
