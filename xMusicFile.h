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

#ifndef __XMUSICFILE_H__
#define __XMUSICFILE_H__

#include <filesystem>
#include <QObject>
#include <QString>


class xMusicFile:public QObject {
public:
    xMusicFile();
    xMusicFile(const std::filesystem::path& file, std::uintmax_t size,
               const QString& artist, const QString& album, const QString& trackName, QObject* parent=nullptr);
    xMusicFile(const xMusicFile& file);
    ~xMusicFile() override = default;
    /**
     * Initiate a onetime scan of the music file.
     *
     * Use taglib to determine the length, bitrate, bits per sample,
     * and the sample rate for the music file.
     */
    void scan() const;
    /**
     * Determine whether or not the music file was already scanned.
     *
     * @return true if the file was scanned, false otherwise.
     */
    [[nodiscard]] bool isScanned() const;
    /**
     * Retrieve the absolute path to the music file.
     *
     * @return the absolute path as std::filesystem::path object.
     */
    [[nodiscard]] const std::filesystem::path& getFilePath() const;
    /**
     * Retrieve the file size of the music file.
     *
     * @return the size of the file in bytes.
     */
    [[nodiscard]] std::uintmax_t getFileSize() const;
    /**
     * Retrieve the artist associated with the music file.
     *
     * @return the artist as string.
     */
    [[nodiscard]] const QString& getArtist() const;
    /**
     * Retrieve the album associated with the music file.
     *
     * @return the album as string.
     */
    [[nodiscard]] const QString& getAlbum() const;
    /**
     * Retrieve the track name associated with the music file.
     *
     * @return the track name as string.
     */
    [[nodiscard]] const QString& getTrackName() const;
    /**
     * Retrieve the length of the music file.
     *
     * The length is determined by taglib and requires an
     * additional scan of the music file.
     *
     * @return the length in ms as integer.
     */
    [[nodiscard]] qint64 getLength() const;
    /**
     * Retrieve the bits per sample for the music file.
     *
     * The bits per sample are determined by taglib and
     * require an additional scan of the music file.
     *
     * @return the bits per samples as integer.
     */
    [[nodiscard]] int getBitsPerSample() const;
    /**
     * Retrieve the sample rate for the music file.
     *
     * The sample rate is determined by taglib and
     * requires an additional scan of the music file.
     *
     * @return the sample rate as integer
     */
    [[nodiscard]] int getSampleRate() const;
    /**
     * Retrieve the bitrate for the music file.
     *
     * The bitrate is determined by taglib and
     * requires an additional scan of the music file.
     *
     * @return the bitrate as integer.
     */
    [[nodiscard]] int getBitrate() const;
    /**
     * Compare music files based on artist, album, track name and size.
     *
     * @param file the other music file to compare to.
     * @return true if the music files are equal, false otherwise.
     */
    bool equal(xMusicFile* file, bool checkFileSize=true) const;

private:
    std::filesystem::path filePath;
    QString fileArtist;
    QString fileAlbum;
    QString fileTrackName;
    std::uintmax_t fileSize;
    // make file properties mutable, because we scan the file only on demand.
    mutable qint64 fileLength;
    mutable int fileBitsPerSample;
    mutable int fileBitrate;
    mutable int fileSampleRate;
};

Q_DECLARE_METATYPE(xMusicFile)
Q_DECLARE_METATYPE(xMusicFile*)

#endif
