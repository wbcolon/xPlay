/*
 * This file is part of xRipEncode.
 *
 * xRipEncode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * xRipEncode is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "xMovieFile.h"
#include <QRegularExpression>
#include <QDebug>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>

xMovieFile::xMovieFile(QObject* parent):QObject(parent) {
}

void xMovieFile::analyze(const QString& file) {
    // Check if we really have a file.
    if (!std::filesystem::is_regular_file(file.toStdString())) {
        qCritical() << "xMovieFile::analyze: illegal file name: " << file;
        return;
    }
    movieFile = file;
    // Start ffprobe in order to analyze the movie file.
    process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start("/usr/bin/ffprobe", { {"-v"}, {"quiet"}, {"-print_format"}, {"json"},
                                        {"-show_streams"}, {"-show_chapters"}, {"-show_format"}, movieFile });
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
    movieChapterLength.clear();
    movieChapterBegin.clear();
    auto childChapters = movieFileInfo.get_child("chapters");
    for (boost::property_tree::ptree::value_type& childChapter : childChapters) {
        // Retrieve start and end time for each track (chapter)
        auto chapterBegin = childChapter.second.get<double>("start_time", 0.0);
        auto chapterEnd = childChapter.second.get<double>("end_time", 0.0);
        // Convert to ms
        movieChapterBegin.push_back(static_cast<qint64>(chapterBegin*1000));
        movieChapterLength.push_back(static_cast<qint64>((chapterEnd-chapterBegin)*1000));
    }
}

QVector<qint64> xMovieFile::getChapterLength() const {
  return movieChapterLength;
}

QVector<qint64> xMovieFile::getChapterBegin() const {
  return movieChapterBegin;
}