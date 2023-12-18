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

#ifndef __XPLAYERTYPES_H__
#define __XPLAYERTYPES_H__

#include <QString>
#include <QUrl>

/**
 * Mode for displaying time.
 */
enum xPlayerTimeMode {
    NoTime,
    MinuteTimeShortMode,
    MinuteTimeMode,
    HourTimeShortMode,
    HourTimeMode,
};

/**
 * Contains url, path, name and length of the directory entry.
 */
typedef std::tuple<QUrl,QString,QString,qint64> xDirectoryEntry;

#endif
