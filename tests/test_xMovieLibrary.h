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

#include <QtTest>
#include <QtTestWidgets>
#include <QMetaType>

#include "xMovieLibrary.h"

class test_xMovieLibrary:public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testScannedTags();
    void testScannedDirectories_data();
    void testScannedDirectories();
    void testScannedMovies_data();
    void testScannedMovies();

private:
    xMovieLibrary* movieLibrary;
    std::list<std::pair<QString, std::filesystem::path>> baseDirectories;
};
