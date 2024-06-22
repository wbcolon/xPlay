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

#include "xPlayerListWidgetItem.h"

#include <QResizeEvent>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>
#include <QTreeWidget>
#include <QCoreApplication>
#include <utility>

xPlayerListWidgetItem::xPlayerListWidgetItem(const QString& text, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(true),
        itemTime(0),
        itemText(text),
        itemTooltip(),
        itemGetText(),
        itemGetLength(),
        itemTimeMode(xPlayerTimeMode::NoTime),
        itemArtistEntry(nullptr),
        itemAlbumEntry(nullptr),
        itemTrackEntry(nullptr),
        itemMovieEntry(nullptr),
        itemErrorMark(false) {
    Q_ASSERT(parent != nullptr);
    setText(0, itemText);
    itemTextWidth = parent->fontMetrics().size(Qt::TextSingleLine, itemText+"...").width();
    itemBackgroundBrush = background(0);
}

xPlayerListWidgetItem::xPlayerListWidgetItem(std::function<QString ()> getText, std::function<qint64 ()> getLength,
                                             xPlayerTimeMode timeMode, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(timeMode == xPlayerTimeMode::NoTime),
        itemTime(0),
        itemTooltip(),
        itemGetText(std::move(getText)),
        itemGetLength(std::move(getLength)),
        itemTimeMode(timeMode),
        itemArtistEntry(nullptr),
        itemAlbumEntry(nullptr),
        itemTrackEntry(nullptr),
        itemMovieEntry(nullptr),
        itemErrorMark(false) {
    Q_ASSERT(parent != nullptr);
    itemText = itemGetText();
    itemTextWidth = parent->fontMetrics().size(Qt::TextSingleLine, itemText+"...").width();
    setText(0, itemText);
    itemBackgroundBrush = background(0);
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicLibraryArtistEntry* entry, QTreeWidget* parent):
        xPlayerListWidgetItem([entry] { return entry->getArtistName(); },
                              [] { return -1; }, xPlayerTimeMode::NoTime, parent) {
    itemArtistEntry = entry;
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicLibraryAlbumEntry* entry, QTreeWidget* parent):
        xPlayerListWidgetItem([entry] { return entry->getAlbumName(); },
                              [] { return -1; }, xPlayerTimeMode::NoTime, parent) {
    itemAlbumEntry = entry;
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicLibraryTrackEntry* entry, QTreeWidget* parent):
        xPlayerListWidgetItem([entry] { return entry->getTrackName(); },
                              [entry] { return entry->getLength(); }, xPlayerTimeMode::MinuteTimeShortMode, parent) {
    itemTrackEntry = entry;
    // Only update the time in the constructor if it was already scanned. The update will force a scan if necessary.
    setTextAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
    if (itemTrackEntry->isScanned()) {
        updateTime();
        updateTimeDisplay();
    }
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMovieLibraryEntry* entry, QTreeWidget* parent):
    xPlayerListWidgetItem([entry] { return entry->getMovieName(); },
                          [entry] { return entry->getLength(); }, xPlayerTimeMode::HourTimeShortMode, parent) {
    itemMovieEntry = entry;
    // Only update the time in the constructor if it was already scanned. The update will force a scan if necessary.
    setTextAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
    if (itemMovieEntry->isScanned()) {
        updateTime();
        updateTimeDisplay();
    }
}

void xPlayerListWidgetItem::setIcon(const QString& fileName) {
    if (fileName.isEmpty()) {
        removeIcon();
    } else {
        QTreeWidgetItem::setIcon(0, QIcon(fileName));
    }
}

void xPlayerListWidgetItem::removeIcon() {
    QTreeWidgetItem::setIcon(0, QIcon());
}

void xPlayerListWidgetItem::addToolTip(const QString& text) {
    itemTooltip = text;
    updateToolTip();
}

void xPlayerListWidgetItem::updateToolTip() {
    if (treeWidget()->columnWidth(0) < itemTextWidth) {
        if (itemTooltip.isEmpty()) {
            setToolTip(0, itemText);
        } else {
            setToolTip(0, QString("%1\n%2").arg(itemText, itemTooltip));
        }
    } else {
        setToolTip(0, itemTooltip);
    }
}

void xPlayerListWidgetItem::removeToolTip() {
    itemTooltip.clear();
    updateToolTip();
}

void xPlayerListWidgetItem::addErrorMark() {
    for (auto i = 0; i < columnCount(); ++i) {
        setBackground(i, QBrush(Qt::red, Qt::Dense6Pattern));
    }
    itemErrorMark = true;
}

void xPlayerListWidgetItem::removeErrorMark() {
    for (auto i = 0; i < columnCount(); ++i) {
        setBackground(i, itemBackgroundBrush);
    }
    itemErrorMark = false;
}

bool xPlayerListWidgetItem::hasErrorMark() {
    return itemErrorMark;
}

const QString& xPlayerListWidgetItem::text() const {
    return itemText;
}

int xPlayerListWidgetItem::textWidth() const {
    return itemTextWidth;
}

void xPlayerListWidgetItem::updateText() {
    if (itemGetText != nullptr) {
        itemText = itemGetText();
        // Update with for new text.
        itemTextWidth = treeWidget()->fontMetrics().size(Qt::TextSingleLine, itemText+"...").width();
    }
    setText(0, itemText);
    updateToolTip();
}

qint64 xPlayerListWidgetItem::updateTime() {
    // Read time only once. Use cached value later on.
    // Note: the corresponding entry has to be scanned beforehand.
    if (!itemTimeUpdated) {
        itemTime = itemGetLength();
        itemTimeUpdated = true;
    }
    return itemTime;
}

void xPlayerListWidgetItem::updateTimeDisplay() {
    if (itemTimeUpdated) {
        switch (itemTimeMode) {
            case xPlayerTimeMode::MinuteTimeShortMode: {
                setText(1, QString("%1:%2").arg(itemTime/60000).arg((itemTime/1000)%60, 2, 10, QChar('0')));
            } break;
            case xPlayerTimeMode::HourTimeShortMode: {
                setText(1, QString("%1:%2:%3").arg(itemTime/3600000).arg((itemTime/60000)%60, 2, 10, QChar('0')).arg((itemTime/1000)%60, 2, 10, QChar('0')));
            } break;
            default: break;
        }
        if (itemTime <= 0) {
            addErrorMark();
        } else {
            // Only remove the error mark if it was marked.
            if (hasErrorMark()) {
                removeErrorMark();
            }
        }
    }
}

xMusicLibraryArtistEntry* xPlayerListWidgetItem::artistEntry() const {
    return itemArtistEntry;
}

xMusicLibraryAlbumEntry* xPlayerListWidgetItem::albumEntry() const {
    return itemAlbumEntry;
}

xMusicLibraryTrackEntry* xPlayerListWidgetItem::trackEntry() const {
    return itemTrackEntry;
}

xMovieLibraryEntry* xPlayerListWidgetItem::movieEntry() const {
    return itemMovieEntry;
}

