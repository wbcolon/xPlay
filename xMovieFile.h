/*
 * This file is part of xPlay.
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

#include <QThread>
#include <QProcess>
#include <QVector>

#ifndef __XMOVIEFILE_H__
#define __XMOVIEFILE_H__

class xMovieFile:public QObject {
    Q_OBJECT

public:
    explicit xMovieFile(QObject* parent=nullptr);
    ~xMovieFile() override = default;
    /**
     * Analyze the given file.
     *
     * @param file path to the movie file as string.
     */
    void analyze(const QString& file);
    /**
     * Determine the number of chapters in the movie file.
     *
     * @return the number of chapters.
     */
    [[nodiscard]] int getChapters() const;
    /**
     * Compute the times for each chapter in the movie file.
     *
     * @return a vector of chapter length in milliseconds.
     */
    [[nodiscard]] QVector<qint64> getChapterLength() const;
    /**
     * Compute the beginning of each chapter in the movie file.
     *
     * @return a vector of chapter beginning in milliseconds.
     */
    [[nodiscard]] QVector<qint64> getChapterBegin() const;

private:
    QString movieFile;
    QString movieFilePath;
    QVector<qint64> movieChapterLength;
    QVector<qint64> movieChapterBegin;
    QProcess* process;
};

#endif
