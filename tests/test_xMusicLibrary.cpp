/*
 * This file is part of xPlay.
 *
 * xPlay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * xPlay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "test_xMusicLibrary.h"
#include "xPlayerConfiguration.h"
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"

#include <QtTest/QSignalSpy>
#include <QMetaType>

#include <vector>
#include <list>
#include <tuple>


void test_xMusicLibrary::initTestCase() {
    musicLibrary = new xMusicLibrary();
    // Load extensions.
    xPlayerConfiguration::configuration()->updatedConfiguration();
}

void test_xMusicLibrary::testScanInvalidLibrary() {
    QSignalSpy spy(musicLibrary, &xMusicLibrary::scanningError);
    musicLibrary->setUrl(QUrl::fromLocalFile("../tests/input/musiclibrary.empty"));
    spy.wait();
    QVERIFY(spy.count() == 1);
}

void test_xMusicLibrary::testScannedArtists() {
    QStringList expectedArtistNames {
            "ac-dc", "black sabbath", "deep purple", "dio",
            "marillion", "opeth", "ozzy osbourne", "rainbow", "zz top"
    };
    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedArtists);
    QSignalSpy spyFinished(musicLibrary, &xMusicLibrary::scanningFinished);
    musicLibrary->setUrl(QUrl::fromLocalFile("../tests/input/musiclibrary"));
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto artists = qvariant_cast<std::vector<xMusicLibraryArtistEntry*>>(spy.at(0).at(0));
    QStringList artistNames;
    for (auto artist : artists) {
        artistNames.push_back(artist->getArtistName());
    }
    QVERIFY(artistNames == expectedArtistNames);
    spyFinished.wait();
    QVERIFY(spyFinished.count() == 1);
}

void test_xMusicLibrary::testScannedArtistsFiltered_data() {
    QTest::addColumn<QStringList>("albumMatch");
    QTest::addColumn<QStringList>("albumNotMatch");
    QTest::addColumn<QString>("searchArtist");
    QTest::addColumn<QString>("searchAlbum");
    QTest::addColumn<QString>("searchTrack");
    QTest::addColumn<QStringList>("expectedArtistNames");

    QTest::newRow("(live)") << QStringList{ "(live)" } << QStringList{ } << "" << "" << "" << QStringList {
            "ac-dc", "black sabbath", "deep purple", "opeth", "ozzy osbourne"
    };
    QTest::newRow("(live), ![hd]") << QStringList{ "(live)" } << QStringList{ "[hd]" } << "" << "" << "" << QStringList {
            "black sabbath", "deep purple", "ozzy osbourne"
    };
    QTest::newRow("paranoid") << QStringList{ } << QStringList{ } << "" << "" << "paranoid" << QStringList {
            "black sabbath", "ozzy osbourne"
    };
}
void test_xMusicLibrary::testScannedArtistsFiltered() {
    QFETCH(QStringList, albumMatch);
    QFETCH(QStringList, albumNotMatch);
    QFETCH(QString, searchArtist);
    QFETCH(QString, searchAlbum);
    QFETCH(QString, searchTrack);
    QFETCH(QStringList, expectedArtistNames);

    xMusicLibraryFilter filter;
    filter.setAlbumMatch(albumMatch, albumNotMatch);
    filter.setSearchMatch(std::make_tuple(searchArtist, searchAlbum, searchTrack));
    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedArtists);
    musicLibrary->scan(filter);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto artists = qvariant_cast<std::vector<xMusicLibraryArtistEntry*>>(spy.at(0).at(0));
    QStringList artistNames;
    for (auto artist : artists) {
        artistNames.push_back(artist->getArtistName());
    }
    QVERIFY(artistNames == expectedArtistNames);
}

void test_xMusicLibrary::testScannedAlbums_data() {
    QTest::addColumn<QString>("artist");
    QTest::addColumn<QStringList>("expectedAlbumNames");

    QTest::newRow("ac-dc") << "ac-dc" << QStringList {
            "back in black [hd]", "highway to hell [hd]",
            "live at donnington (live) [hd]", "live at river plate (live) [hd]"
    };
    QTest::newRow("black sabbath") << "black sabbath" << QStringList {
            "13 [hd]", "gathered in their masses (live) [hd]", "headless cross", "heaven and hell (2021 remastered) [hd]",
            "heaven and hell - live at the radio city music hall (live)", "paranoid", "paranoid (2012 remastered)"
    };
    QTest::newRow("deep purple") << "deep purple" << QStringList {
            "in rock", "in rock [hd]", "machine head", "machine head [hd]",
            "made in japan (live)", "made in japan (live) [hd]", "whoosh [hd]"
    };
    QTest::newRow("dio") << "dio" << QStringList {
            "holy diver", "holy diver [hd]", "killing the dragon", "killing the dragon (2019 remastered)",
            "magica", "magica (2019 remastered)"
    };
    QTest::newRow("marillion") << "marillion" << QStringList {
            "clutching at straws", "clutching at straws (demos)", "clutching at straws (remix) [hd]",
            "misplaced childhood", "misplaced childhood (steven wilson remix) [hd]", "misplaced childhood [hd]"
    };
    QTest::newRow("opeth") << "opeth" << QStringList {
            "blackwater park", "garden of the titans - live at red rocks amphitheatre (live) [hd]", "ghost reveries"
    };
    QTest::newRow("ozzy osbourne") << "ozzy osbourne" << QStringList {
            "blizzard of ozz [hd]", "blizzard of ozz tour (live)",
            "itunes festival - london 2010 (live) [mp3]", "live and loud (live)", "tribute (live) [hd]"
    };
    QTest::newRow("rainbow") << "rainbow" << QStringList {
            "long live rock 'n' roll", "rising",
    };
    QTest::newRow("zz top") << "zz top" << QStringList {
            "antenna", "eliminator [hd]", "la futura"
    };
    QTest::newRow("invalid artist") << "invalid artist" << QStringList { };
}

void test_xMusicLibrary::testScannedAlbums() {
    QFETCH(QString, artist);
    QFETCH(QStringList, expectedAlbumNames);

    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedAlbums);
    musicLibrary->scanForArtist(artist);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto albums = qvariant_cast<std::vector<xMusicLibraryAlbumEntry*>>(spy.at(0).at(0));
    QStringList albumNames;
    for (auto album : albums) {
        albumNames.push_back(album->getAlbumName());
    }
    albumNames.sort();
    QVERIFY(albumNames == expectedAlbumNames);
}

void test_xMusicLibrary::testScannedAlbumsFilter_data() {
    QTest::addColumn<QString>("artist");
    QTest::addColumn<QStringList>("albumMatch");
    QTest::addColumn<QStringList>("albumNotMatch");
    QTest::addColumn<QString>("searchAlbum");
    QTest::addColumn<QString>("searchTrack");
    QTest::addColumn<QStringList>("expectedAlbumNames");

    QTest::newRow("ac-dc, !(live)") << "ac-dc" << QStringList{ } << QStringList{ "(live)" }
                                   << "" << "" << QStringList {
            "back in black [hd]", "highway to hell [hd]"
    };
    QTest::newRow("marillion, [hd], misplaced") << "marillion" << QStringList{ "[hd]"} << QStringList{}
                               << "misplaced" << "" << QStringList {
            "misplaced childhood (steven wilson remix) [hd]", "misplaced childhood [hd]"
    };
    QTest::newRow("marillion, [hd], not misplaced") << "marillion" << QStringList{ "[hd]"} << QStringList{}
                               << "not misplaced" << "" << QStringList { };
    QTest::newRow("ozzy osbourne, (live), ![mp3]") << "ozzy osbourne" << QStringList{ "(live)" } << QStringList{ "[mp3]" }
                                                   << "" << "" << QStringList {
            "blizzard of ozz tour (live)", "live and loud (live)", "tribute (live) [hd]"
    };
    QTest::newRow("ozzy osbourne, ![hd], paranoid") << "ozzy osbourne" << QStringList{ } << QStringList{ "[hd]" }
                                                   << "" << "paranoid" << QStringList {
            "blizzard of ozz tour (live)", "live and loud (live)",
    };
}

void test_xMusicLibrary::testScannedAlbumsFilter() {
    QFETCH(QString, artist);
    QFETCH(QStringList, albumMatch);
    QFETCH(QStringList, albumNotMatch);
    QFETCH(QString, searchAlbum);
    QFETCH(QString, searchTrack);
    QFETCH(QStringList, expectedAlbumNames);

    xMusicLibraryFilter filter;
    filter.setAlbumMatch(albumMatch, albumNotMatch);
    filter.setSearchMatch(std::make_tuple("", searchAlbum, searchTrack));
    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedAlbums);
    musicLibrary->scanForArtist(artist, filter);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto albums = qvariant_cast<std::vector<xMusicLibraryAlbumEntry*>>(spy.at(0).at(0));
    QStringList albumNames;
    for (auto album : albums) {
        albumNames.push_back(album->getAlbumName());
    }
    albumNames.sort();
    QVERIFY(albumNames == expectedAlbumNames);
}

void test_xMusicLibrary::testScannedAllAlbumTracks_data() {
    QTest::addColumn<QString>("artist");
    QTest::addColumn<QStringList>("expectedAlbumNames");
    QTest::addColumn<QStringList>("expectedTrackNames");

    QTest::newRow("rainbow") << "rainbow" << QStringList {
            "long live rock 'n' roll", "rising"
    } << QStringList {
            "01 long live rock and roll.flac",
            "02 lady on the lake.flac",
            "03 l. a. connection.flac",
            "04 gates of babylon.flac",
            "05 kill the king.flac",
            "06 the shed (subtle).flac",
            "07 sensitive to light.flac",
            "08 rainbow eyes.flac",
            "09 lady of the lake (rough mix).flac",
            "10 sensitive to light (rough mix).flac",
            "11 l.a. connection (rough mix).flac",
            "12 kill the king (rough mix).flac",
            "13 the shed (subtle) (rough mix).flac",
            "14 long live rock 'n' roll (rough mix).flac",
            "15 rainbow eyes (rough mix).flac",
            "16 long live rock 'n' roll (shepperton rehearsal) (take 1).flac",
            "17 kill the king (shepperton rehearsal).flac",
            "18 long live rock 'n' roll (don kirshner's rock concert).flac",
            "19 l.a. connection (don kirshner's rock concert).flac",
            "20 gates of babylon (don kirshner's rock concert).flac",
            "21 l.a. connection (don kirshner's rock concert - outtake version).flac",
            "22 gates of babylon (don kirshner's rock concert - outtake version).flac",
            "01 tarot woman (new york mix).flac",
            "02 run with the wolf (new york mix).flac",
            "03 starstruck (new york mix).flac",
            "04 do you close your eyes (new york mix).flac",
            "05 stargazer (new york mix).flac",
            "06 a light in the black (new york mix).flac",
            "07 tarot woman (los angeles mix).flac",
            "08 run with the wolf (los angeles mix).flac",
            "09 starstruck (los angeles mix).flac",
            "10 do you close your eyes (los angeles mix).flac",
            "11 stargazer (los angeles mix).flac",
            "12 a light in the black (los angeles mix).flac",
            "13 tarot woman (rough mix).flac",
            "14 run with the wolf (rough mix).flac",
            "15 starstruck (rough mix).flac",
            "16 do you close your eyes (rough mix).flac",
            "17 stargazer (rough mix).flac",
            "18 a light in the black (rough mix).flac",
            "19 stargazer (pirate sound tour rehearsal).flac",
    };
    QTest::newRow("zz top") << "zz top" << QStringList {
            "antenna", "eliminator [hd]", "la futura"
    } << QStringList {
            "01 pincushion.flac",
            "02 breakaway.flac",
            "03 world of swirl.flac",
            "04 fuzzbox voodoo.flac",
            "05 girl in a t-shirt.flac",
            "06 antenna head.flac",
            "07 pch.flac",
            "08 cherry red.flac",
            "09 cover your rig.flac",
            "10 lizard life.flac",
            "11 deal goin' down.flac",
            "12 everything.flac",
            "01 gimme all your lovin'.flac",
            "02 got me under pressure.flac",
            "03 sharp dressed man.flac",
            "04 i need you tonight.flac",
            "05 i got the six.flac",
            "06 legs.flac",
            "07 thug.flac",
            "08 tv dinners.flac",
            "09 dirty dog.flac",
            "10 if i could only flag her down.flac",
            "11 bad girl.flac",
            "01 i gotsta get paid.flac",
            "02 chartreuse.flac",
            "03 consumption.flac",
            "04 over you.flac",
            "05 heartache in blue.flac",
            "06 i don't wanna lose, lose, you.flac",
            "07 flyin' high.flac",
            "08 it's to easy manana.flac",
            "09 big shiny nine.flac",
            "10 have a little mercy.flac",
    };

}

void test_xMusicLibrary::testScannedAllAlbumTracks() {
    QFETCH(QString, artist);
    QFETCH(QStringList, expectedAlbumNames);
    QFETCH(QStringList, expectedTrackNames);

    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedAllAlbumTracks);
    musicLibrary->scanAllAlbumsForArtist(artist);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto artistName = qvariant_cast<QString>(spy.at(0).at(0));
    auto albumTracks = qvariant_cast<QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>(spy.at(0).at(1));
    QStringList albumNames;
    QStringList trackNames;
    for (const auto& album : albumTracks) {
        albumNames.push_back(album.first);
        for (auto track : album.second) {
            trackNames.push_back(track->getTrackName());
        }
    }
    albumNames.sort();
    QVERIFY(artistName == artist);
    QVERIFY(albumNames == expectedAlbumNames);
    QVERIFY(trackNames == expectedTrackNames);
}

void test_xMusicLibrary::testScannedAllAlbumTracksFiltered_data() {
    QTest::addColumn<QString>("artist");
    QTest::addColumn<QStringList>("albumMatch");
    QTest::addColumn<QStringList>("albumNotMatch");
    QTest::addColumn<QString>("searchAlbum");
    QTest::addColumn<QString>("searchTrack");
    QTest::addColumn<QStringList>("expectedAlbumNames");

    QTest::newRow("ac-dc, hells bells") << "ac-dc" << QStringList{ } << QStringList{ } << "" << "hells bells" << QStringList {
            "back in black [hd]", "live at donnington (live) [hd]", "live at river plate (live) [hd]",
    };
    QTest::newRow("ac-dc, ![hd]") << "ac-dc" << QStringList{ } << QStringList{ "[hd]" } << "" << "" << QStringList { };
    QTest::newRow("black sabbath, !(live), hell") << "black sabbath" << QStringList{ } << QStringList{ "(live)" } << "" << "hell" << QStringList {
            "headless cross", "heaven and hell (2021 remastered) [hd]"
    };
    QTest::newRow("deep purple, [hd], in") << "deep purple" << QStringList{ "[hd]" } << QStringList{ } << "in" << "" << QStringList {
            "in rock [hd]", "machine head [hd]", "made in japan (live) [hd]",
    };
    QTest::newRow("dio, ![hd], holy diver") << "dio" << QStringList{ } << QStringList{ "[hd]" } << "" << "holy diver" << QStringList {
            "holy diver", "killing the dragon (2019 remastered)",
    };
    QTest::newRow("opeth, holy diver") << "opeth" << QStringList{ } << QStringList{ "" } << "" << "holy diver" << QStringList { };
}

void test_xMusicLibrary::testScannedAllAlbumTracksFiltered() {
    QFETCH(QString, artist);
    QFETCH(QStringList, albumMatch);
    QFETCH(QStringList, albumNotMatch);
    QFETCH(QString, searchAlbum);
    QFETCH(QString, searchTrack);
    QFETCH(QStringList, expectedAlbumNames);

    xMusicLibraryFilter filter;
    filter.setAlbumMatch(albumMatch, albumNotMatch);
    filter.setSearchMatch(std::make_tuple("", searchAlbum, searchTrack));
    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedAllAlbumTracks);
    musicLibrary->scanAllAlbumsForArtist(artist, filter);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto artistName = qvariant_cast<QString>(spy.at(0).at(0));
    auto albumTracks = qvariant_cast<QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>(spy.at(0).at(1));
    QStringList albumNames;
    for (const auto& album : albumTracks) {
        albumNames.push_back(album.first);
    }
    albumNames.sort();
    QVERIFY(artistName == artist);
    QVERIFY(albumNames == expectedAlbumNames);
}

void test_xMusicLibrary::testScannedListArtistsAllAlbumTracks_data() {
    QTest::addColumn<QStringList>("listArtists");
    QTest::addColumn<QStringList>("expectedAlbumNames");
    QTest::addColumn<QStringList>("expectedTrackNames");

    QTest::newRow("ac-dc,opeth") << QStringList { "ac-dc", "opeth" } << QStringList {
            "back in black [hd]", "highway to hell [hd]", "live at donnington (live) [hd]", "live at river plate (live) [hd]",
            "blackwater park", "garden of the titans - live at red rocks amphitheatre (live) [hd]", "ghost reveries"
    } << QStringList {
            "01 hells bells.flac",
            "02 shoot to thrill.flac",
            "03 what do you do for money honey.flac",
            "04 givin the dog a bone.flac",
            "05 let me put my love into you.flac",
            "06 back in black.flac",
            "07 you shook me all night long.flac",
            "08 have a drink on me.flac",
            "09 shake a leg.flac",
            "10 rock and roll ain't noise pollution.flac",
            "01 highway to hell.flac",
            "02 girls got rhythm.flac",
            "03 walk all over you.flac",
            "04 touch too much.flac",
            "05 beating around the bush.flac",
            "06 shot down in flames.flac",
            "07 get it hot.flac",
            "08 if you want blood (you've got it).flac",
            "09 love hungry man.flac",
            "10 night prowler.flac",
            "01 thunderstruck.flac",
            "02 shoot to thrill.flac",
            "03 back in black.flac",
            "04 hell ain't a bad place to be.flac",
            "05 heatseeker.flac",
            "06 fire your guns.flac",
            "07 jailbreak.flac",
            "08 the jack.flac",
            "09 dirty deeds done dirt cheap.flac",
            "10 moneytalks.flac",
            "11 hells bells.flac",
            "12 high voltage.flac",
            "13 whole lotta rosie.flac",
            "14 you shook me all night long.flac",
            "15 t.n.t..flac",
            "16 let there be rock.flac",
            "17 highway to hell.flac",
            "18 for those about to rock (we salute you).flac",
            "19 outro.flac",
            "01 intro.flac",
            "02 rock 'n' roll train.flac",
            "03 hell ain't a bad place to be.flac",
            "04 back in black.flac",
            "05 big jack.flac",
            "06 dirty deeds done dirt cheap.flac",
            "07 shot down in flames.flac",
            "08 thunderstruck.flac",
            "09 black ice.flac",
            "10 the jack.flac",
            "11 hells bells.flac",
            "12 shoot to thrill.flac",
            "13 war machine.flac",
            "14 dog eat dog.flac",
            "15 you shook me all night long.flac",
            "16 t.n.t..flac",
            "17 whole lotta rosie.flac",
            "18 let there be rock.flac",
            "19 highway to hell.flac",
            "20 for those about to rock (we salute you).flac",
            "01 the leper affinity.flac",
            "02 bleak.flac",
            "03 harvest.flac",
            "04 the drapery falls.flac",
            "05 dirge for november.flac",
            "06 the funeral portrait.flac",
            "07 patterns in the ivy.flac",
            "08 blackwater park.flac",
            "09 the leper affinity (live).flac",
            "10 still day beneath the sun.flac",
            "11 patterns in the ivy ii.flac",
            "01 sorceress.flac",
            "02 ghost of perdition.flac",
            "03 demon of the fall.flac",
            "04 the wilde flowers.flac",
            "05 in my time of need.flac",
            "06 the devil's orchard.flac",
            "07 cusp of eternity.flac",
            "08 heir apparent.flac",
            "09 era.flac",
            "10 deliverance.flac",
            "01 ghost of perdition.flac",
            "02 the baying of the hounds.flac",
            "03 beneath the mire.flac",
            "04 atonement.flac",
            "05 reverie - harlequin forest.flac",
            "06 hours of wealth.flac",
            "07 the grand conjuration.flac",
            "08 isolation years.flac",
            "09 soldier of fortune.flac",
    };
    QTest::newRow("opeth,invalid artist") << QStringList { "opeth", "invalid artist" } << QStringList {
            "blackwater park", "garden of the titans - live at red rocks amphitheatre (live) [hd]", "ghost reveries"
    } << QStringList {
            "01 the leper affinity.flac",
            "02 bleak.flac",
            "03 harvest.flac",
            "04 the drapery falls.flac",
            "05 dirge for november.flac",
            "06 the funeral portrait.flac",
            "07 patterns in the ivy.flac",
            "08 blackwater park.flac",
            "09 the leper affinity (live).flac",
            "10 still day beneath the sun.flac",
            "11 patterns in the ivy ii.flac",
            "01 sorceress.flac",
            "02 ghost of perdition.flac",
            "03 demon of the fall.flac",
            "04 the wilde flowers.flac",
            "05 in my time of need.flac",
            "06 the devil's orchard.flac",
            "07 cusp of eternity.flac",
            "08 heir apparent.flac",
            "09 era.flac",
            "10 deliverance.flac",
            "01 ghost of perdition.flac",
            "02 the baying of the hounds.flac",
            "03 beneath the mire.flac",
            "04 atonement.flac",
            "05 reverie - harlequin forest.flac",
            "06 hours of wealth.flac",
            "07 the grand conjuration.flac",
            "08 isolation years.flac",
            "09 soldier of fortune.flac",
    };
    QTest::newRow("rainbow,zz top") << QStringList { "rainbow", "zz top" } << QStringList {
        "long live rock 'n' roll", "rising", "antenna", "eliminator [hd]", "la futura"
    } << QStringList {
        "01 long live rock and roll.flac",
        "02 lady on the lake.flac",
        "03 l. a. connection.flac",
        "04 gates of babylon.flac",
        "05 kill the king.flac",
        "06 the shed (subtle).flac",
        "07 sensitive to light.flac",
        "08 rainbow eyes.flac",
        "09 lady of the lake (rough mix).flac",
        "10 sensitive to light (rough mix).flac",
        "11 l.a. connection (rough mix).flac",
        "12 kill the king (rough mix).flac",
        "13 the shed (subtle) (rough mix).flac",
        "14 long live rock 'n' roll (rough mix).flac",
        "15 rainbow eyes (rough mix).flac",
        "16 long live rock 'n' roll (shepperton rehearsal) (take 1).flac",
        "17 kill the king (shepperton rehearsal).flac",
        "18 long live rock 'n' roll (don kirshner's rock concert).flac",
        "19 l.a. connection (don kirshner's rock concert).flac",
        "20 gates of babylon (don kirshner's rock concert).flac",
        "21 l.a. connection (don kirshner's rock concert - outtake version).flac",
        "22 gates of babylon (don kirshner's rock concert - outtake version).flac",
        "01 tarot woman (new york mix).flac",
        "02 run with the wolf (new york mix).flac",
        "03 starstruck (new york mix).flac",
        "04 do you close your eyes (new york mix).flac",
        "05 stargazer (new york mix).flac",
        "06 a light in the black (new york mix).flac",
        "07 tarot woman (los angeles mix).flac",
        "08 run with the wolf (los angeles mix).flac",
        "09 starstruck (los angeles mix).flac",
        "10 do you close your eyes (los angeles mix).flac",
        "11 stargazer (los angeles mix).flac",
        "12 a light in the black (los angeles mix).flac",
        "13 tarot woman (rough mix).flac",
        "14 run with the wolf (rough mix).flac",
        "15 starstruck (rough mix).flac",
        "16 do you close your eyes (rough mix).flac",
        "17 stargazer (rough mix).flac",
        "18 a light in the black (rough mix).flac",
        "19 stargazer (pirate sound tour rehearsal).flac",
        "01 pincushion.flac",
        "02 breakaway.flac",
        "03 world of swirl.flac",
        "04 fuzzbox voodoo.flac",
        "05 girl in a t-shirt.flac",
        "06 antenna head.flac",
        "07 pch.flac",
        "08 cherry red.flac",
        "09 cover your rig.flac",
        "10 lizard life.flac",
        "11 deal goin' down.flac",
        "12 everything.flac",
        "01 gimme all your lovin'.flac",
        "02 got me under pressure.flac",
        "03 sharp dressed man.flac",
        "04 i need you tonight.flac",
        "05 i got the six.flac",
        "06 legs.flac",
        "07 thug.flac",
        "08 tv dinners.flac",
        "09 dirty dog.flac",
        "10 if i could only flag her down.flac",
        "11 bad girl.flac",
        "01 i gotsta get paid.flac",
        "02 chartreuse.flac",
        "03 consumption.flac",
        "04 over you.flac",
        "05 heartache in blue.flac",
        "06 i don't wanna lose, lose, you.flac",
        "07 flyin' high.flac",
        "08 it's to easy manana.flac",
        "09 big shiny nine.flac",
        "10 have a little mercy.flac",
        };
    QTest::newRow("invalid,artist") << QStringList { "invalid", "artist" } << QStringList { } << QStringList { };
}

void test_xMusicLibrary::testScannedListArtistsAllAlbumTracks() {
    QFETCH(QStringList, listArtists);
    QFETCH(QStringList, expectedAlbumNames);
    QFETCH(QStringList, expectedTrackNames);

    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedListArtistsAllAlbumTracks);
    // Convert list of artist names into list of xMusicDirectory
    musicLibrary->scanAllAlbumsForListArtists(listArtists);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto artistAlbumTracks = qvariant_cast<QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>>>(spy.at(0).at(0));
    QStringList artistNames;
    QStringList albumNames;
    QStringList trackNames;
    for (const auto& artist: artistAlbumTracks) {
        artistNames.push_back(artist.first);
        for (const auto& album : artist.second) {
            albumNames.push_back(album.first);
            for (auto track : album.second) {
                trackNames.push_back(track->getTrackName());
            }
        }
    }
    QVERIFY(artistNames == listArtists);
    QVERIFY(albumNames == expectedAlbumNames);
    QVERIFY(trackNames == expectedTrackNames);
}

void test_xMusicLibrary::testScannedListArtistsAllAlbumTracksFilter_data() {
    QTest::addColumn<QStringList>("listArtists");
    QTest::addColumn<QStringList>("albumMatch");
    QTest::addColumn<QStringList>("albumNotMatch");
    QTest::addColumn<QString>("searchAlbum");
    QTest::addColumn<QString>("searchTrack");
    QTest::addColumn<QStringList>("expectedAlbumNames");

    QTest::newRow("ac-dc, dio, hell") << QStringList{ "ac-dc", "dio" }
                                        << QStringList{ } << QStringList{ } << "" << "hell" << QStringList {
            "back in black [hd]", "highway to hell [hd]", "live at donnington (live) [hd]",
            "live at river plate (live) [hd]", "killing the dragon (2019 remastered)"
    };
    QTest::newRow("ac-dc, black sabbath, dio, hells") << QStringList{ "ac-dc", "black sabbath", "dio" }
                                      << QStringList{ } << QStringList{ } << "" << "hells" << QStringList {
            "back in black [hd]", "live at donnington (live) [hd]", "live at river plate (live) [hd]",
    };
    QTest::newRow("black sabbath, ozzy osbourne, ![hd], paranoid") << QStringList{ "black sabbath", "ozzy osbourne" }
                                                                   << QStringList{ } << QStringList{ "[hd]" } << "" << "paranoid" << QStringList {
            "paranoid", "paranoid (2012 remastered)", "blizzard of ozz tour (live)", "live and loud (live)",
    };
    QTest::newRow("black sabbath, deep purple, opeth, [hd], hea") << QStringList{ "black sabbath", "deep purple", "opeth" }
                                                                  << QStringList{ "[hd]" } << QStringList{ } << "hea" << "" << QStringList {
            "heaven and hell (2021 remastered) [hd]", "machine head [hd]",
            "garden of the titans - live at red rocks amphitheatre (live) [hd]"
    };
    QTest::newRow("marillion, opeth, no album") << QStringList{ "marillion", "opeth" } << QStringList{ } << QStringList{ } << "no album" << "" << QStringList { };
    QTest::newRow("marillion, opeth, holy diver") << QStringList{ "marillion", "opeth" } << QStringList{ } << QStringList{ } << "" << "holy diver" << QStringList { };
}

void test_xMusicLibrary::testScannedListArtistsAllAlbumTracksFilter() {
    QFETCH(QStringList, listArtists);
    QFETCH(QStringList, albumMatch);
    QFETCH(QStringList, albumNotMatch);
    QFETCH(QString, searchAlbum);
    QFETCH(QString, searchTrack);
    QFETCH(QStringList, expectedAlbumNames);

    xMusicLibraryFilter filter;
    filter.setAlbumMatch(albumMatch, albumNotMatch);
    filter.setSearchMatch(std::make_tuple("", searchAlbum, searchTrack));
    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedListArtistsAllAlbumTracks);
    // Convert list of artist names into list of xMusicDirectory
    musicLibrary->scanAllAlbumsForListArtists(listArtists, filter);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto artistAlbumTracks = qvariant_cast<QList<std::pair<QString,QList<std::pair<QString,std::vector<xMusicLibraryTrackEntry*>>>>>>(spy.at(0).at(0));
    QStringList albumNames;
    for (const auto& artist: artistAlbumTracks) {
        for (const auto& album : artist.second) {
            albumNames.push_back(album.first);
        }
    }
    QVERIFY(albumNames == expectedAlbumNames);
}

void test_xMusicLibrary::testScannedTracks_data() {
    QTest::addColumn<QString>("artist");
    QTest::addColumn<QString>("album");
    QTest::addColumn<QStringList>("expectedTrackNames");

    QTest::newRow("ac-dc/back in black [hd]") << "ac-dc" << "back in black [hd]" << QStringList {
            "01 hells bells.flac",
            "02 shoot to thrill.flac",
            "03 what do you do for money honey.flac",
            "04 givin the dog a bone.flac",
            "05 let me put my love into you.flac",
            "06 back in black.flac",
            "07 you shook me all night long.flac",
            "08 have a drink on me.flac",
            "09 shake a leg.flac",
            "10 rock and roll ain't noise pollution.flac"
    };
    QTest::newRow("black sabbath/headless cross") << "black sabbath" << "headless cross" << QStringList {
            "01 the gates of hell.flac",
            "02 headless cross.flac",
            "03 devil & daughter.flac",
            "04 when death calls.flac",
            "05 kill in the spirit world.flac",
            "06 call of the wild.flac",
            "07 black moon.flac",
            "08 nightwing.flac",
    };
    QTest::newRow("marillion/misplaced childhood (steven wilson remix) [hd]") << "marillion" << "misplaced childhood (steven wilson remix) [hd]" << QStringList {
            "01 pseudo silk kimono.flac",
            "02 kayleigh.flac",
            "03 lavender.flac",
            "04 bitter suite.flac",
            "05 heart of lothian.flac",
            "06 waterhole (expresso bongo).flac",
            "07 lords of the backstage.flac",
            "08 blind curve.flac",
            "09 childhoods end.flac",
            "10 white feather.flac",
            "11 lady nina.flac",
    };
    QTest::newRow("zz top/la futura") << "zz top" << "la futura" << QStringList {
            "01 i gotsta get paid.flac",
            "02 chartreuse.flac",
            "03 consumption.flac",
            "04 over you.flac",
            "05 heartache in blue.flac",
            "06 i don't wanna lose, lose, you.flac",
            "07 flyin' high.flac",
            "08 it's to easy manana.flac",
            "09 big shiny nine.flac",
            "10 have a little mercy.flac",
    };
    QTest::newRow("zz top/invalid album") << "zz top" << "invalid album" << QStringList{ };
    QTest::newRow("invalid artist/invalid album") << "invalid artist" << "invalid album" << QStringList{ };
}

void test_xMusicLibrary::testScannedTracks() {
    QFETCH(QString, artist);
    QFETCH(QString, album);
    QFETCH(QStringList, expectedTrackNames);

    QSignalSpy spy(musicLibrary, &xMusicLibrary::scannedTracks);
    musicLibrary->scanForArtistAndAlbum(artist, album);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto tracks = qvariant_cast<std::vector<xMusicLibraryTrackEntry*>>(spy.at(0).at(0));
    QStringList trackNames;
    for (auto track : tracks) {
        trackNames.push_back(track->getTrackName());
    }
    trackNames.sort();
    QVERIFY(trackNames == expectedTrackNames);
}
