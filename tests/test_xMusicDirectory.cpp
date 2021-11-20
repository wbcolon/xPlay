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

#include "test_xMusicDirectory.h"

#include <filesystem>

void test_xMusicDirectory::testSimpleAccessFunctions() {
    xMusicDirectory musicDirectory("album");
    QVERIFY(musicDirectory.name() == QString("album"));
}

void test_xMusicDirectory::testAccessFunctions() {
    xMusicDirectory musicDirectory0(std::filesystem::directory_entry("../tests/input/musiclibrary/ac-dc"));
    xMusicDirectory musicDirectory1(std::filesystem::directory_entry("../tests/input/musiclibrary/ac-dc/back in black [hd]"));
    QVERIFY(musicDirectory0.name() == QString("ac-dc"));
    QVERIFY(musicDirectory0.path() == std::filesystem::path("../tests/input/musiclibrary/ac-dc"));
    QVERIFY(musicDirectory1.name() == QString("back in black [hd]"));
    QVERIFY(musicDirectory1.path() == std::filesystem::path("../tests/input/musiclibrary/ac-dc/back in black [hd]"));
    QVERIFY(musicDirectory1.lastWritten() >= musicDirectory0.lastWritten());
    QVERIFY(musicDirectory0 < musicDirectory1);
}

