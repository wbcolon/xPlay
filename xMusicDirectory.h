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

#ifndef __XMUSICDIRECTORY_H__
#define __XMUSICDIRECTORY_H__

#include <filesystem>

#include <QObject>
#include <QString>


class xMusicDirectory:public QObject {
public:
    xMusicDirectory();
    explicit xMusicDirectory(const QString& name, QObject* parent=nullptr);
    explicit xMusicDirectory(const std::filesystem::directory_entry& entry, QObject* parent=nullptr);
    xMusicDirectory(const xMusicDirectory& dir);
    ~xMusicDirectory() override = default;
    /**
     * Retrieve the name of the directory entry.
     *
     * @return the name of the directory entry as string.
     */
    [[nodiscard]] const QString& name() const;
    /**
     * Retrieve the absolute path of the directory entry.
     *
     * @return the absolute path of the directory entry.
     */
    [[nodiscard]] const std::filesystem::path& path() const;
    /**
     * Retrieve the last written time stamp of the directory entry.
     *
     * @return the time stamp as file_time_type.
     */
    [[nodiscard]] const std::filesystem::file_time_type& lastWritten() const;
    /**
     * Lesser compare two directory entries.
     *
     * @param entry the other directory entry for the comparison
     * @return the result of the lesser comparison on the directory entry names.
     */
    bool operator < (const xMusicDirectory& entry) const;
    /**
     * Assignment operator.
     *
     * @param entry the directory entry on RHS of the operator.
     * @return a reference to the object.
     */
    xMusicDirectory& operator = (const xMusicDirectory& entry);

private:
    QString directoryName;
    std::filesystem::path directoryPath;
    std::filesystem::file_time_type directoryLastWritten;
};

Q_DECLARE_METATYPE(xMusicDirectory)

#endif
