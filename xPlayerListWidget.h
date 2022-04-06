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

#ifndef __XPLAYERLISTWIDGET_H__
#define __XPLAYERLISTWIDGET_H__

#include "xPlayerUI.h"
#include "xMusicLibraryEntry.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QThread>
#include <QLabel>
#include <QString>

#include <map>


// Qt does not support templates with signals and slots mechanism.
class xPlayerListWidgetItem:public QTreeWidgetItem {
public:
    explicit xPlayerListWidgetItem(const QString& text, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMusicLibraryArtistEntry* artist, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMusicLibraryAlbumEntry* album, QTreeWidget* parent);
    explicit xPlayerListWidgetItem(xMusicLibraryTrackEntry* track, QTreeWidget* parent);
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
     * Return the entry object associated with the list item.
     *
     * @return a pointer to the artist, album or track object, nullptr if no attached.
     */
    [[nodiscard]] xMusicLibraryArtistEntry* artistEntry() const;
    [[nodiscard]] xMusicLibraryAlbumEntry* albumEntry() const;
    [[nodiscard]] xMusicLibraryTrackEntry* trackEntry() const;
    /**
     * Determine the time for the list item. No UI update.
     */
    qint64 updateTime();
    /**
     * Update the displayed time in the list item.
     */
    void updateTimeDisplay();

private:
    bool itemTimeUpdated;
    qint64 itemTime;
    QString itemText;
    int itemTextWidth;
    QString itemTooltip;
    xMusicLibraryArtistEntry* itemArtistEntry;
    xMusicLibraryAlbumEntry* itemAlbumEntry;
    xMusicLibraryTrackEntry* itemTrackEntry;
};


class xPlayerListWidget:public QTreeWidget {
    Q_OBJECT

public:
    explicit xPlayerListWidget(QWidget* parent=nullptr, bool displayTime=false);
    ~xPlayerListWidget() override = default;
    /**
     * Enable the sorted mode for the list widget.
     *
     * @param sorted items are sorted if true, unsorted otherwise.
     */
    void enableSorting(bool sorted);
    /**
     * Add item without time section to the list.
     *
     * @param text the text to be shown as string.
     */
    void addListItem(const QString& text);
    /**
     * Add item with tooltip and without time section to the list.
     *
     * @param text the text to be shown as string.
     * @param tooltip the text of the tooltip as string.
     */
    void addListItem(const QString& text, const QString& tooltip);
    /**
     * Add list of items without time section to the list.
     * @param list a string list of items to be added.
     */
    void addListItems(const QStringList& list);
    /**
     * Add item to the list.
     *
     * @param entry pointer to the associated music library entry object.
     */
    void addListItem(xMusicLibraryArtistEntry* entry);
    void addListItem(xMusicLibraryAlbumEntry* entry);
    void addListItem(xMusicLibraryTrackEntry* entry);
    /**
     * Add item with tooltip to the list.
     *
     * @param entry pointer to the associated music library entry object.
     * @param tooltip the text of the tooltip as string.
     */
    void addListItem(xMusicLibraryArtistEntry* entry, const QString& tooltip);
    void addListItem(xMusicLibraryAlbumEntry* entry, const QString& tooltip);
    void addListItem(xMusicLibraryTrackEntry* entry, const QString& tooltip);
    /**
     * Add vector of items with tooltip to the list.
     *
     * @param files vector of pointer to the associated music file objects.
     * @param tooltip  the text of the tooltip as string.
     */
    void addListItems(const std::vector<xMusicLibraryTrackEntry*>& entries, const QString& tooltip);
    /**
     * Add list of pairs of tooltip and item vector.
     *
     * @param files list of pairs of tooltip and vector of pointer to associated music file objects.
     */
    void addListItems(const QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>& entries);
    /**
     * Find the list item that match the given text.
     *
     * @param text the text we are looking for as string.
     * @return a list of item widget with matching text.
     */
    QList<xPlayerListWidgetItem*> findListItems(const QString& text);
    /**
     * Select the item widget that matches the given text.
     *
     * @param text the text of the item to be selected.
     * @return true if the item could be selected, false otherwise.
     */
    bool setCurrentItem(const QString& text);
    /**
     * Return the list item of the currently selected element.
     *
     * @return a pointer to the selected item.
     */
    xPlayerListWidgetItem* currentItem();
    /**
     * Return the list item at the given index.
     *
     * @param index the index as integer.
     * @return a pointer to the item.
     */
    xPlayerListWidgetItem* listItem(int index);
    /**
     * Remove the list item from the given index.
     *
     * @param index the index as integer.
     */
    void takeListItem(int index);
    /**
     * Return the list item at the given point.
     *
     * @param point the given point position.
     * @return pointer to the list item or nullptr.
     */
    xPlayerListWidgetItem* itemAt(const QPoint& point);
    /**
     * Select the given list item.
     *
     * @param index the index of the new currently selected list item.
     */
    void setCurrentListIndex(int index);
    /**
     * Retrieve the index of the currently selected list item.
     *
     * @return the index of the currently selected list item.
     */
    int currentListIndex();
    /**
     * Retrieve the index for a given list item.
     *
     * @param item a pointer to the list widget item.
     * @return the index of the list item.
     */
    int listIndex(xPlayerListWidgetItem* item);
    /**
     * Retrieve the number of list items.
     *
     * @return the number of list elements as integer.
     */
    int count();
    /**
     * Refresh item entries.
     *
     * Sort item entries according to the lessThan function if given. All entries are removed from
     * the list and then reinserted.
     *
     * @param lesserThan function that defines the order of two list widget items.
     */
    void refreshItems(std::function<bool (xPlayerListWidgetItem*, xPlayerListWidgetItem*)> lesserThan = nullptr);
    /**
     * Update all list items.
     *
     * Run time consuming updates in a separate thread.
     */
    void updateItems();
    /**
     * Clear all items.
     */
    void clearItems();

public slots:
    /**
     * Update filter used to decide which items to show.
     *
     * @param match the substring the items text must match.
     */
    void updateFilter(const QString& match);

signals:
    /**
     * Signal emitted whenever an item in the list is selected.
     *
     * @param index the index of the selected item.
     */
    void currentListIndexChanged(int index);
    /**
     * Signal emitted if an item in the list is double clicked.
     *
     * @param item pointer to the item.
     */
    void listItemDoubleClicked(xPlayerListWidgetItem* item);
    /**
     * Signal emitted if an item in the list is clicked.
     *
     * @param item pointer to the item.
     */
    void listItemClicked(xPlayerListWidgetItem* item);
    /**
     * Signal emitted if total time of all list elements has been computed.
     *
     * @param total the total time in ms.
     */
    void totalTime(qint64 total);
    /**
     * Signal emitted if the time is updated in a list element.
     *
     * The update has to be done via signals in order to perform the
     * UI update from the Qt main loop.
     *
     * @param item pointer to the item.
     */
    void updateTime(xPlayerListWidgetItem* item);
    /**
     * Sinal emitted if an element was moved via drag and drop.
     *
     * @param fromIndex the initial index of the element moved.
     * @param toIndex the index the element is inserted before.
     */
    void dragDrop(int fromIndex, int toIndex);
    /**
     * Signal used to update larger list in stages.
     *
     * @param entries list of pairs of tooltip and vector of pointer to associated music library entries.
     * @param entryIterator iterator to the list element to be added.
     * @param currentFiles current number of files inserted.
     * @param maxFiles maximal number of files inserted overall.
     */
    void itemWidgetsIterate(const QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>& entries,
                            QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>::const_iterator entryIterator,
                            int currentFiles, int maxFiles);
    /**
     * Signal emitted when inserted in stages.
     *
     * @param current the current number of items inserted.
     * @param max the maximal number of items to be inserted.
     */
    void itemWidgetsProgress(int current, int max);

protected:
    /**
     * Called upon resizing the tree widget.
     *
     * @param event a pointer to the resize event.
     */
    void resizeEvent(QResizeEvent* event) override;
    /**
     * Called upon the start of the drag-and-drop operation.
     *
     * @param event a pointer to the drag enter event.
     */
    void dragEnterEvent(QDragEnterEvent* event) override;
    /**
     * Called upon the end of the drag-and-drop operation.
     *
     * This function will emit a dragDrop signal upon successful
     * completion of the drag-and-drop operation.
     *
     * @param event a pointer to the drop event.
     */
    void dropEvent(QDropEvent* event) override;

private:
    /**
     * Add list item with the given text.
     *
     * Add the list item or insert it in ascending order.
     *
     * @param item the pointer to the list item.
     * @param text the text to be inserted as string.
     */
    void addListWidgetItem(xPlayerListWidgetItem* item, const QString& text);
    /**
     * Worker function for updateItems. Function is running in a separate thread.
     *
     * Requires fine grained reaction to interrupt cancellations.
     */
    void updateItemsWorker();
    /**
     * Called upon finishing the thread.
     */
    void updateItemsWorkerFinished();
    /**
     * Worker used to add items in stages.
     *
     * @param files list of pairs of tooltip and vector of pointer to associated music file objects.
     * @param fileIterator iterator to the list element to be added.
     * @param currentFiles current number of files inserted.
     * @param maxFiles maximal number of files inserted overall.
     */
    void addItemWidgetsWorker(const QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>& entries,
                              QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>::const_iterator entriesIterator,
                              int currentFiles, int maxFiles);

    bool sortItems;
    QThread* updateItemsThread;
    int dragDropFromIndex;
    int dragDropToIndex;
    QString currentMatch;
};

#endif
