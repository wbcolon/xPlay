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
#ifndef __XPLAYERMUSICLIBRARYWIDGET_H__
#define __XPLAYERMUSICLIBRARYWIDGET_H__

#include "xMusicLibrary.h"

#include <QGroupBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class xPlayerMusicLibraryWidgetItem:public QTreeWidgetItem {

public:
    explicit xPlayerMusicLibraryWidgetItem(const QString& entryName, const QUrl& entryUrl,
                                           xPlayerMusicLibraryWidgetItem* parent=nullptr);
    explicit xPlayerMusicLibraryWidgetItem(xMusicLibraryTrackEntry* file, xPlayerMusicLibraryWidgetItem* parent);
    ~xPlayerMusicLibraryWidgetItem() override = default;
    /**
     * Return the total size cached in the item.
     *
     * @return the total size in bytes.
     */
    [[nodiscard]] std::uintmax_t getTotalSize() const;
    /**
     * Add total size information for item.
     *
     * @param size the total size in bytes.
     */
    void updateTotalSize(std::uintmax_t size);
    /**
     * Clear any size information.
     */
    void clearTotalSize();
    /**
     * Mark the item (and possibly its children).
     *
     * @param brushStyle the brush style used to mark the item.
     * @param children mark the entries children if true.
     */
    void mark(const Qt::BrushStyle& brushStyle, bool children=false);
    /**
     * Mark the item (and possibly its children) and save the brush.
     *
     * @param brush the brush used to mark the item.
     * @param children mark the entries children if true.
     */
    void mark(const QBrush& brush, bool children=false);
    /**
     * Restore the mark for the item (and possibly its children).
     *
     * @param children mark the entries children if true.
     */
    void restoreMark(bool children=false);
    /**
     * Return if the current item is the parent of the given one.
     *
     * @param item the pointer to the given item.
     * @return true, if the current item is a parent of the given item.
     */
    [[nodiscard]] bool isParentOf(xPlayerMusicLibraryWidgetItem* item) const;
    /**
     * Return if the current item is the child of the given one.
     *
     * @param item the pointer to the given item.
     * @return true, if the current item is a child of the given item.
     */
    [[nodiscard]] bool isChildOf(xPlayerMusicLibraryWidgetItem* item) const;
    /**
     * Return the artist item for this item in the tree.
     *
     * @return a pointer to the artist item.
     */
    [[nodiscard]] xPlayerMusicLibraryWidgetItem* artist() const;
    /**
     * Return the album item for this item in the tree.
     *
     * @return a pointer to the album item.
     */
    [[nodiscard]] xPlayerMusicLibraryWidgetItem* album() const;
    /**
     * Return the name for this item in the tree.
     *
     * @return the entry name as string.
     */
    [[nodiscard]] const QString& entryName() const;
    /**
     * Return the path for this item in the tree.
     *
     * @return the path for the entry.
     */
    [[nodiscard]] const QUrl& entryUrl() const;
    /**
     * Return the track entry object for this item in the tree.
     *
     * @return a pointer to the track entry.
     */
    [[nodiscard]] xMusicLibraryTrackEntry* trackEntry() const;
    /**
     * Return a description of the entry.
     *
     * @return the description as string.
     */
    [[nodiscard]] QString description() const;

private:
    QString itemEntryName;
    QUrl itemEntryUrl;
    xMusicLibraryTrackEntry* itemTrackEntry;
    std::uintmax_t itemTotalSize;
    xPlayerMusicLibraryWidgetItem* itemParent;
    QBrush itemSaveBackground;
};


class xPlayerMusicLibraryWidget:public QGroupBox {
    Q_OBJECT

public:
    xPlayerMusicLibraryWidget(xMusicLibrary* library, const QString& name, QWidget* parent=nullptr);
    ~xPlayerMusicLibraryWidget() override = default;
    /**
     * Set base directory for the music library and initiate a scan.
     *
     * @param base the new base directory as path.
     * @param force force scanning of the library if true.
     */
    void setUrl(const QUrl& base, bool force=false);
    /**
     * Enable or disable the sorting by size mode.
     *
     * @param enabled sort by size if true, by name otherwise.
     */
    void setSortBySize(bool enabled);
    /**
     * Determine if the widget is ready.
     *
     * @return true, if the scan is finished, false otherwise.
     */
    [[nodiscard]] bool isReady() const;
    /**
     * Select a specific artist item.
     *
     * @param artist the given artist to be selected.
     */
    void selectItem(const QString& artist);
    /**
     * Select a specific album item for an artist.
     *
     * @param artist the given artist.
     * @param album the given album to be selected.
     */
    void selectItem(const QString& artist, const QString& album);
    /**
     * Return the item located at the given position.
     *
     * @param point the given position as point.
     * @return a pointer to the corresponding item.
     */
    [[nodiscard]] xPlayerMusicLibraryWidgetItem* itemAt(const QPoint& point) const;
    /**
     * Mark the items in the music library tree.
     *
     * @param missingArtists the list of missing artists.
     * @param missingAlbums the list of missing albums for existing artists.
     * @param missingTracks the list of missing tracks for existing artists and albums.
     * @param differentTracks the list of tracks that have the same relative path, but are different.
     */
    void markItems(const QStringList& missingArtists,
                   const std::map<QString, QStringList>& missingAlbums,
                   const std::list<xMusicLibraryTrackEntry*> &missingTracks,
                   const std::list<xMusicLibraryTrackEntry*> &differentTracks);
    /**
     * Mark the existing items in the music library tree.
     *
     * @param existingTracks the map of tracks that already exist.
     */
    void markExistingItems(const std::map<QString, std::map<QString, std::list<xMusicLibraryTrackEntry*>>>& existingTracks);

public slots:
    /**
     * Filter the toplevel artist items according to the given filter.
     *
     * Use an empty string to disable the artist item filter.
     *
     * @param filter the filter as string the artist name must contain.
     */
    void filterArtistItems(const QString& filter);
    /**
     * Clear any markings.
     */
    void clearItems();
    /**
     * Clear the item (and possibly its children).
     *
     * @param item a pointer to the item to be cleared.
     * @param markChildren Clear the entries children if true.
     */
    void clearItem(QTreeWidgetItem* item, bool clearChildren=true);
    /**
     * Remove all items (including children).
     */
    void clear();

signals:
    /**
     * Signal emitted if an item in the music library tree is ctrl+right-clicked.
     *
     * @param item a pointer to the item in the tree.
     */
    void treeItemCtrlRightClicked(xPlayerMusicLibraryWidgetItem* item);
    /**
     * Signal emitted if an item in the music library tree is ctrl+clicked.
     *
     * @param item a pointer to the item in the tree.
     */
    void treeItemCtrlClicked(xPlayerMusicLibraryWidgetItem* item);
    /**
     * Signal emitted if the tree is fully constructed.
     */
    void treeReady();

private slots:
    /**
     * Update the music library tree once the music library scan is finished.
     */
    void scanningFinished();
    /**
     * Show context menu on right mouse click.
     */
    void openContextMenu(const QPoint& point);
    /**
     * Recreate music library tree.
     */
    void updateMusicLibraryTree();
    /**
     * Update the visibility of missing artists.
     */
    void updateMissingArtists();

private:
    QTreeWidget* musicLibraryTree;
    unsigned musicLibraryTreeLevel;
    QString musicLibraryFilter;
    xMusicLibrary* musicLibrary;
    bool musicLibraryReady;
    bool musicLibrarySortBySize;
    QBrush musicLibraryItemBackground;
    bool musicLibraryHideMissingArtists;
    QStringList musicLibraryHiddenArtists;
    // maps to enable fast access to items.
    std::map<QString,xPlayerMusicLibraryWidgetItem*> mapArtists;
    std::map<QString, std::map<QString, xPlayerMusicLibraryWidgetItem*>> mapAlbums;
    std::map<xMusicLibraryTrackEntry*, xPlayerMusicLibraryWidgetItem*> mapTracks;
};

#endif
