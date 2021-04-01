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

#include <QListWidget>
#include <QListWidgetItem>
#include <QThread>
#include <QLabel>
#include <QString>

#include <map>

class xMusicFile;

class xPlayerListItemWidget: public QWidget {
    Q_OBJECT

public:
    explicit xPlayerListItemWidget(const QString& text, QWidget* parent=nullptr);
    explicit xPlayerListItemWidget(xMusicFile* file, QWidget* parent=nullptr);
    ~xPlayerListItemWidget() override = default;
    /**
     * Set an icon for the list item widget.
     *
     * @param iconPath path to the icon/pixmap file as string.
     */
    void setIcon(const QString& iconPath);
    /**
     * Remove the icon from the list item widget.
     */
    void removeIcon();
    /**
     * Add a tooltip to the list item widget.
     *
     * The tooltip is added to a possibly shortened track name
     * separated by a newline.
     *
     * @param tooltip the added tooltip as string.
     */
    void addToolTip(const QString& tooltip);
    /**
     * Remove the added tooltip from the list item widget.
     *
     * The tooltip used for a shortened track name will still be
     * visible.
     */
    void removeToolTip();
    /**
     * Return the text of the list item widget.
     *
     * @return the text of the name label as string.
     */
    [[nodiscard]] const QString& text() const;
    /**
     * Update the time for the list item widget.
     */
    qint64 updateTime();

protected:
    /**
     * Overload the resizeEvent in order to shorten the text if necessary.
     *
     * @param event the resize event containing the new size.
     */
    void resizeEvent(QResizeEvent* event) override;

private:
    /**
     * Update the track name
     *
     * @param width the maximum with the text is allowed to have.
     */
    void updateTrackName(int width);
    /**
     * Shorten the text by cutting ending chars.
     *
     * @param width the maximum with the text is allowed to have.
     * @return the shortened text that fits the allowed width.
     */
    QString shortenedTrackName(int width);
    /**
     * Update the tool tip.
     */
    void updateToolTip();

private:
    xPlayerLayout* itemLayout;
    QLabel* itemIconLabel;
    QLabel* itemTextLabel;
    QLabel* itemTimeLabel;
    bool itemTimeUpdated;
    qint64 itemTime;
    QString itemText;
    bool itemTextShortened;
    QString itemToolTip;
    xMusicFile* itemFile;
};

class xPlayerListWidget:public QListWidget {
    Q_OBJECT

public:
    explicit xPlayerListWidget(QWidget* parent=nullptr);
    ~xPlayerListWidget() override = default;
    /**
     * Enable the sorted mode for the list widget.
     *
     * @param sorted items are sorted if true, unsorted otherwise.
     */
    void enableSorting(bool sorted);
    /**
     * Add item widget without time section to the list.
     *
     * @param text the text to be shown as string.
     */
    void addItemWidget(const QString& text);
    /**
     * Add item widget with tooltip and without time section to the list.
     *
     * @param text the text to be shown as string.
     * @param tooltip the text of the tooltip as string.
     */
    void addItemWidget(const QString& text, const QString& tooltip);
    /**
     * Add list of widgets without time section to the list.
     * @param list a string list of items to be added.
     */
    void addItemWidgets(const QStringList& list);
    /**
     * Add item widget to the list.
     *
     * @param file pointer to the associated music file object.
     */
    void addItemWidget(xMusicFile* file);
    /**
     * Add item widget with tooltip to the list.
     *
     * @param file pointer to the associated music file object.
     * @param tooltip the text of the tooltip as string.
     */
    void addItemWidget(xMusicFile* file, const QString& tooltip);
    /**
     * Find the item widgets that match the given text.
     *
     * @param text the text we are looking for as string.
     * @return a list of item widget with matching text.
     */
    QList<xPlayerListItemWidget*> findItemWidgets(const QString& text);
    /**
     * Return the item widget of the currently selected element.
     *
     * @return a pointer to the selected item widget.
     */
    xPlayerListItemWidget* currentItemWidget();
    /**
     * Return the item widget at the given index.
     *
     * @param index the index as integer.
     * @return a pointer to the item widget.
     */
    xPlayerListItemWidget* itemWidget(int index);
    /**
     * Remove the item including widget at the given index.
     * @param index the index as integer.
     */
    void takeItemWidget(int index);
    /**
     * Update all item widgets.
     *
     * Run time consuming item widget updates in a separate thread.
     */
    void updateItems();
    /**
     * Clear all item widgets.
     */
    void clearItems();

signals:
    void totalTime(qint64 total);

private:
    /**
     * Add list widget item with the given text.
     *
     * Add the item or insert it in ascending order.
     *
     * @param item the pointer to the list widget item.
     * @param text the text to be inserted as string.
     */
    void addListWidgetItem(QListWidgetItem* item, const QString& text);
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

    bool sortItems;
    std::map<QListWidgetItem*,xPlayerListItemWidget*> mapItems;
    QThread* updateItemsThread;
};

#endif
