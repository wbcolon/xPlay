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

#include "test_xMovieLibrary.h"
#include "xPlayerConfiguration.h"

#include <QtTest/QSignalSpy>
#include <QMetaType>

#include <vector>
#include <list>
#include <tuple>


void test_xMovieLibrary::initTestCase() {
    movieLibrary = new xMovieLibrary();
    // Load extensions.
    xPlayerConfiguration::configuration()->updatedConfiguration();
    // Set base directories.
    baseDirectories = {
            {"documentation", "../tests/input/movielibrary/documentation/" },
            {"movies", "../tests/input/movielibrary/movies/" },
            {"movies", "../tests/input/movielibrary/movies (bd)/" },
            {"shows", "../tests/input/movielibrary/shows/" },
            {"shows", "../tests/input/movielibrary/shows (bd)/" },
    };
}

void test_xMovieLibrary::testScannedTags() {
    QStringList expectedTags {
            "documentation", "movies", "shows"
    };
    QSignalSpy spy(movieLibrary, &xMovieLibrary::scannedTags);
    movieLibrary->setBaseDirectories(baseDirectories);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto tags = qvariant_cast<QStringList>(spy.at(0).at(0));
    QVERIFY(tags == expectedTags);
}

void test_xMovieLibrary::testScannedDirectories_data() {
    QTest::addColumn<QString>("tag");
    QTest::addColumn<QStringList>("expectedDirectories");

    QTest::newRow("documentation") << "documentation" << QStringList { };
    QTest::newRow("movies") << "movies" << QStringList {
            "asterix und obelix",
            "bud spencer",
            "bud spencer and terence hill",
            "dc-movies",
            "jackie chan",
            "marvel-movies",
            "schwarzenegger",
            "stallone",
    };
    QTest::newRow("shows") << "shows" << QStringList {
            "boston legal",
            "columbo",
            "der tatortreiniger",
            "die purpurnen fluesse",
            "einstein",
            "knight rider",
            "lie to me",
            "lilyhammer",
            "psych",
            "rome",
            "tatort",
            "the big bang theory",
            "the sopranos",
    };
}

void test_xMovieLibrary::testScannedDirectories() {
    QFETCH(QString, tag);
    QFETCH(QStringList, expectedDirectories);

    QSignalSpy spy(movieLibrary, &xMovieLibrary::scannedDirectories);
    movieLibrary->scanForTag(tag);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto directories = qvariant_cast<QStringList>(spy.at(0).at(0));
    directories.sort();
    QVERIFY(directories == expectedDirectories);
}

void test_xMovieLibrary::testScannedMovies_data() {
    QTest::addColumn<QString>("tag");
    QTest::addColumn<QString>("directory");
    QTest::addColumn<QStringList>("expectedMovies");

    QTest::newRow("documentation/.") << "documentation" << "." << QStringList {
            "bowling for columbine.mkv",
            "fahrenheit 9-11.mkv",
            "god bless ozzy osbourne.mkv",
            "sicko.mkv",
    };
    QTest::newRow("movies/stallone") << "movies" << "stallone" << QStringList {
            "over the top.mkv",
            "rocky.mkv",
    };
    QTest::newRow("movies/schwarzenegger") << "movies" << "schwarzenegger" << QStringList {
            "running man.mkv",
            "terminator.mkv",
            "true lies.mkv",
    };
    QTest::newRow("movies/.") << "movies" << "." << QStringList {
            "2012.mkv",
            "300.mkv",
            "a few good men.mkv",
            "a fish called wanda.mkv",
            "batman (part 1 - batman begins).mkv",
            "batman (part 2 - the dark knight).mkv",
            "batman (part 3 - the dark knight rises).mkv",
            "die feuerzangenbowle.mkv",
            "elizabethtown.mkv",
            "hot fuzz.mkv",
            "independence day.mkv",
            "jack reacher.mkv",
            "john wick.mkv",
            "knight and day.mkv",
            "matrix.mkv",
            "men in black.mkv",
            "spider-man.mkv",
            "starship troopers.mkv",
            "unbreakable.mkv",
            "wasabi.mkv",
            "xxx.mkv",
    };
    QTest::newRow("shows/columbo") << "shows" << "columbo" << QStringList {
            "s01 e01 - prescription - murder.mkv",
            "s01 e02 - ransom for a dead man.mkv"
    };
    QTest::newRow("shows/die purpurnen fluesse") << "shows" << "die purpurnen fluesse" << QStringList {
            "s01 e01 - melodie des todes.mp4",
            "s01 e02 - tag der asche.mp4",
    };
}

void test_xMovieLibrary::testScannedMovies() {
    QFETCH(QString, tag);
    QFETCH(QString, directory);
    QFETCH(QStringList, expectedMovies);

    QSignalSpy spy(movieLibrary, &xMovieLibrary::scannedMovies);
    movieLibrary->scanForTagAndDirectory(tag, directory);
    spy.wait();
    QVERIFY(spy.count() == 1);
    // Convert result to string list.
    auto moviesPaths = qvariant_cast<std::vector<std::pair<QString,QString>>>(spy.at(0).at(0));
    QStringList movies;
    for (const auto& moviePath : moviesPaths) {
        movies.push_back(moviePath.first);
    }
    movies.sort();
    QVERIFY(movies == expectedMovies);
}
