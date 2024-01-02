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

#ifndef __XPLAYERLISTWIDGETITEM_H__
#define __XPLAYERLISTWIDGETITEM_H__

#include "xPlayerUI.h"
#include "xPlayerTypes.h"
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"
#include "xMovieLibraryEntry.h"

#include <QTreeWidgetItem>
#include <QString>

#include <functional>

/**
 * @Note: Qt does not support templates with signals and slots mechanism.
 */
class xPlayerListWidgetItem:public QTreeWidgetItem {
public:
    explicit xPlayerListWidgetItem(const QString& text, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(std::function<QString ()> getText, std::function<qint64 ()> getLength,
                                   xPlayerTimeMode timeMode, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMusicLibraryArtistEntry* artist, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMusicLibraryAlbumEntry* album, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMusicLibraryTrackEntry* track, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMovieLibraryEntry* movie, QTreeWidget* parent);
    ~xPlayerListWidgetItem() override = default;
    /**
     * Set an icon for the list item.
     *
     * @param iconPath path to the icon/pixmap file as string.
     */
    void setIcon(const QString& iconPath);
    /**
     * Remove the icon from the list item.
     */
    void removeIcon();
    /**
     * Add a tooltip to the list item.
     *
     * The tooltip is added to a possibly shortened track name
     * separated by a newline.
     *
     * @param tooltip the added tooltip as string.
     */
    void addToolTip(const QString& tooltip);
    /**
     * Update the tooltip to the list item.
     *
     * The tooltip is added to a possibly shortened track name
     * separated by a newline.
     */
    void updateToolTip();
    /**
     * Remove the added tooltip from the list item.
     *
     * The tooltip used for a shortened track name will still be
     * visible.
     */
    void removeToolTip();
    /**
     * Add a red background pattern as error mark.
     */
    void addErrorMark();
    /**
     * Remove the error mark.
     */
    void removeErrorMark();
    /**
     * Check the item for an error mark.
     *
     * @return true if the item is marked, false otherwise.
     */
    bool hasErrorMark();
    /**
     * Return the text of the list item.
     *
     * @return the text of the name label as string.
     */
    [[nodiscard]] const QString& text() const;
    /**
     * Return the width of the list item text.
     *
     * @return the width as int.
     */
    [[nodiscard]] int textWidth() const;
    /**
     * Update the text of the list item.
     *
     * The text is updated from a connected artist, album and track entry object.
     */
    void updateText();
    /**
     * Determine the time for the list item. No UI update.
     */
    qint64 updateTime();
    /**
     * Update the displayed time in the list item.
     */
    void updateTimeDisplay();
    /**
     * Return the entry object associated with the list item.
     *
     * @return a pointer to the artist, album or track object, nullptr if no attached.
     */
    [[nodiscard]] xMusicLibraryArtistEntry* artistEntry() const;
    [[nodiscard]] xMusicLibraryAlbumEntry* albumEntry() const;
    [[nodiscard]] xMusicLibraryTrackEntry* trackEntry() const;
    [[nodiscard]] xMovieLibraryEntry* movieEntry() const;

protected:
    bool itemTimeUpdated;
    qint64 itemTime;
    QString itemText;
    int itemTextWidth;
    QString itemTooltip;
    std::function<QString ()> itemGetText;
    std::function<qint64 ()> itemGetLength;
    xPlayerTimeMode itemTimeMode;
    xMusicLibraryArtistEntry* itemArtistEntry{};
    xMusicLibraryAlbumEntry* itemAlbumEntry{};
    xMusicLibraryTrackEntry* itemTrackEntry{};
    xMovieLibraryEntry* itemMovieEntry{};
    QBrush itemBackgroundBrush;
    bool itemErrorMark;
};

#endif