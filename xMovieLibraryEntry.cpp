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

#include "xMovieLibraryEntry.h"
#include "xPlayerDatabase.h"

#include <QThread>
#include <QProcess>
#include <QDebug>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>

xMovieLibraryEntry::xMovieLibraryEntry() :
    QObject(),
    entryTag(),
    entryDirectory(),
    entryMovie(),
    entryPath(),
    entryAnalyzed(false),
    entryLength(0),
    entrySize(0),
    entryChapterBegin(),
    entryChapterLength() {}

xMovieLibraryEntry::xMovieLibraryEntry(const QString& tag, const QString& directory,
                                       const QString& movie, const std::filesystem::path& moviePath, QObject* parent) :
    QObject(parent),
    entryTag(tag),
    entryDirectory(directory),
    entryMovie(movie),
    entryPath(moviePath),
    entryAnalyzed(false),
    entryLength(-1),
    entrySize(-1),
    entryChapterBegin(),
    entryChapterLength() {}

xMovieLibraryEntry::xMovieLibraryEntry(const xMovieLibraryEntry& entry) :
    QObject(entry.parent()),
    entryTag(entry.entryTag),
    entryDirectory(entry.entryDirectory),
    entryMovie(entry.entryMovie),
    entryPath(entry.entryPath),
    entryAnalyzed(entry.entryAnalyzed),
    entryLength(entry.entryLength),
    entrySize(entry.entrySize),
    entryChapterBegin(entry.entryChapterBegin),
    entryChapterLength(entry.entryChapterLength) {}

const QString& xMovieLibraryEntry::getTagName() const {
    return entryTag;
}

const QString& xMovieLibraryEntry::getDirectoryName() const {
    return entryDirectory;
}

const QString& xMovieLibraryEntry::getMovieName() const {
    return entryMovie;
}

std::uintmax_t xMovieLibraryEntry::getFileSize() const {
    scan();
    return entrySize;
}

qint64 xMovieLibraryEntry::getLength() const {
    scan();
    return entryLength;
}

QVector<qint64> xMovieLibraryEntry::getChapterLength() const {
    analyze();
    return entryChapterLength;
}

QVector<qint64> xMovieLibraryEntry::getChapterBegin() const {
    analyze();
    return entryChapterBegin;
}

const std::filesystem::path& xMovieLibraryEntry::getPath() const {
    return entryPath;
}

void xMovieLibraryEntry::scan() const {
    if ((entrySize > 0) && (entryLength > 0)) {
        return;
    }
    // Determine file size.
    try {
        entrySize = std::filesystem::file_size(entryPath);
    } catch (const std::filesystem::filesystem_error& error) {
        qCritical() << "Unable to access track: "
                    << QString::fromStdString(entryPath.generic_string()) << ", error: " << error.what();
        entrySize = static_cast<std::uintmax_t>(-1);
        entryLength = -1;
        return;
    }

    auto [movieSize, movieLength] = xPlayerDatabase::database()->getMovieFileLength(entryTag, entryDirectory, entryMovie);
    if ((entrySize == static_cast<std::uintmax_t>(movieSize)) && (movieLength > 0)) {
        entryLength = movieLength;
        return;
    }

    analyze();
}

void xMovieLibraryEntry::analyze() const {
    // Do not analyze if already analyzed.
    if (entryAnalyzed) {
        return;
    }
    entryAnalyzed = true;
    // Start ffprobe in order to analyze the movie file.
    auto process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start("/usr/bin/ffprobe", { {"-v"}, {"quiet"}, {"-print_format"}, {"json"},
                                        {"-show_streams"}, {"-show_chapters"}, {"-show_format"}, entryPath.generic_string().c_str() });
    process->waitForFinished(-1);
    // Process standard output (includes stderr) and interpret json.
    std::stringstream processOutput;
    processOutput << process->readAllStandardOutput().toStdString();
    // Delete process.
    delete process;
    boost::property_tree::ptree movieFileInfo;
    try {
        boost::property_tree::read_json(processOutput, movieFileInfo);
    } catch (boost::property_tree::json_parser_error& e) {
        qCritical() << "xMovieFile::analyze: unable to read movie file info.";
        return;
    }
    entryChapterLength.clear();
    entryChapterBegin.clear();
    auto childChapters = movieFileInfo.get_child("chapters");
    for (boost::property_tree::ptree::value_type& childChapter : childChapters) {
        // Retrieve start and end time for each track (chapter)
        auto chapterBegin = childChapter.second.get<double>("start_time", 0.0);
        auto chapterEnd = childChapter.second.get<double>("end_time", 0.0);
        // Convert to ms
        entryChapterBegin.push_back(static_cast<qint64>(chapterBegin*1000));
        entryChapterLength.push_back(static_cast<qint64>((chapterEnd-chapterBegin)*1000));
    }
    auto childFormat = movieFileInfo.get_child("format");
    auto duration = childFormat.get<double>("duration", 0.0);

    entryLength = static_cast<qint64>(duration*1000);
    entryAnalyzed = true;
}

bool xMovieLibraryEntry::isScanned() const {
    return (entryLength > 0);
}