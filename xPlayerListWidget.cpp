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

#include "xPlayerListWidget.h"
#include "xMusicLibraryArtistEntry.h"
#include "xMusicLibraryAlbumEntry.h"
#include "xMusicLibraryTrackEntry.h"
#include "xPlayerUI.h"

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
        itemArtistEntry(nullptr),
        itemAlbumEntry(nullptr),
        itemTrackEntry(nullptr),
        itemMovieEntry(nullptr) {
    Q_ASSERT(parent != nullptr);
    setText(0, text);
    itemTextWidth = parent->fontMetrics().width(itemText+"...");
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicLibraryArtistEntry* artist, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(true),
        itemTime(0),
        itemTooltip(),
        itemArtistEntry(artist),
        itemAlbumEntry(nullptr),
        itemTrackEntry(nullptr),
        itemMovieEntry(nullptr) {
    Q_ASSERT(parent != nullptr);
    Q_ASSERT(artist != nullptr);
    itemText = itemArtistEntry->getArtistName();
    setText(0, itemText);
    itemTextWidth = parent->fontMetrics().width(itemText+"...");
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicLibraryAlbumEntry* album, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(true),
        itemTime(0),
        itemTooltip(),
        itemArtistEntry(nullptr),
        itemAlbumEntry(album),
        itemTrackEntry(nullptr),
        itemMovieEntry(nullptr) {
    Q_ASSERT(parent != nullptr);
    Q_ASSERT(album != nullptr);
    itemText = itemAlbumEntry->getAlbumName();
    setText(0, itemText);
    itemTextWidth = parent->fontMetrics().width(itemText+"...");
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicLibraryTrackEntry* track, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(false),
        itemTime(0),
        itemText(),
        itemTextWidth(0),
        itemTooltip(),
        itemArtistEntry(nullptr),
        itemAlbumEntry(nullptr),
        itemTrackEntry(track),
        itemMovieEntry(nullptr) {
    Q_ASSERT(parent != nullptr);
    Q_ASSERT(track != nullptr);
    setTextAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
    itemText = itemTrackEntry->getTrackName();
    // Only update the time in the constructor if it was already scanned. The update will force a scan if necessary.
    if (itemTrackEntry->isScanned()) {
        updateTime();
        updateTimeDisplay();
    }
    setText(0, itemText);
    itemTextWidth = parent->fontMetrics().width(itemText+"...");
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMovieLibraryEntry* movie, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(false),
        itemTime(0),
        itemText(),
        itemTextWidth(0),
        itemTooltip(),
        itemArtistEntry(nullptr),
        itemAlbumEntry(nullptr),
        itemTrackEntry(nullptr),
        itemMovieEntry(movie) {
    Q_ASSERT(parent != nullptr);
    Q_ASSERT(movie != nullptr);
    setTextAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
    itemText = itemMovieEntry->getMovieName();
    // Only update the time in the constructor if it was already scanned. The update will force a scan if necessary.
    if (itemMovieEntry->isScanned()) {
        updateTime();
        updateTimeDisplay();
    }
    setText(0, itemText);
    itemTextWidth = parent->fontMetrics().width(itemText+"...");
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

const QString& xPlayerListWidgetItem::text() const {
    return itemText;
}

int xPlayerListWidgetItem::textWidth() const {
    return itemTextWidth;
}

void xPlayerListWidgetItem::updateText() {
    if (itemTrackEntry) {
        itemText = itemTrackEntry->getTrackName();
    } else if (itemAlbumEntry) {
        itemText = itemAlbumEntry->getAlbumName();
    } else if (itemArtistEntry) {
        itemText = itemArtistEntry->getArtistName();
    }
    setText(0, itemText);
    // TODO: update itemWidth.
    updateToolTip();
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

qint64 xPlayerListWidgetItem::updateTime() {
    if ((itemTrackEntry) && (!itemTimeUpdated)) {
        itemTime = itemTrackEntry->getLength();
        itemTimeUpdated = true;
    } else if ((itemMovieEntry) && (!itemTimeUpdated)) {
        itemTime = itemMovieEntry->getLength();
        itemTimeUpdated = true;
    }
    return itemTime;
}

void xPlayerListWidgetItem::updateTimeDisplay() {
    if ((itemTrackEntry) && (itemTimeUpdated)) {
        setText(1, QString("%1:%2").arg(itemTime/60000).arg((itemTime/1000)%60, 2, 10, QChar('0')));
    }
    if ((itemMovieEntry) && (itemTimeUpdated)) {
        setText(1, QString("%1:%2:%3").arg(itemTime/3600000).arg((itemTime/60000)%60, 2, 10, QChar('0')).arg((itemTime/1000)%60, 2, 10, QChar('0')));
    }
}

xPlayerListWidget::xPlayerListWidget(QWidget* parent, bool displayTime):
        QTreeWidget(parent),
        sortItems(false),
        updateItemsThread(nullptr),
        dragDropFromIndex(-1),
        dragDropToIndex(-1),
        currentMatch() {
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    if (displayTime) {
        setColumnCount(2);
        setColumnWidth(1, fontMetrics().width("99:99:99"));
    } else {
        setColumnCount(1);
    }
    header()->setVisible(false);
    setRootIsDecorated(false);
    connect(this, &xPlayerListWidget::itemWidgetsIterate, this, &xPlayerListWidget::addItemWidgetsWorker);
    connect(this, &QTreeWidget::currentItemChanged, [=](QTreeWidgetItem* current, QTreeWidgetItem*) {
        if (current) {
            emit currentListIndexChanged(indexOfTopLevelItem(current));
        }
    });
    connect(this, &QTreeWidget::itemDoubleClicked, [=](QTreeWidgetItem* item, int) {
        emit listItemDoubleClicked(reinterpret_cast<xPlayerListWidgetItem*>(item));
    });
    connect(this, &QTreeWidget::itemClicked, [=](QTreeWidgetItem* item, int) {
        emit listItemClicked(reinterpret_cast<xPlayerListWidgetItem*>(item));
    });
    connect(this, &xPlayerListWidget::updateTime, [=](xPlayerListWidgetItem* item) {
        item->updateTimeDisplay();
    });
}

xPlayerListWidget::~xPlayerListWidget() {
    if (updateItemsThread) {
        qDebug() << "Waiting for timer thread to finish.";
        updateItemsThread->requestInterruption();
        updateItemsThread->wait();
    }
}

void xPlayerListWidget::enableSorting(bool sorted) {
    sortItems = sorted;
}
void xPlayerListWidget::addListItem(const QString& text) {
    addListItem(text, QString());
}

void xPlayerListWidget::addListItem(const QString& text, const QString& tooltip) {
    auto item = new xPlayerListWidgetItem(text, this);
    item->addToolTip(tooltip);
    addListWidgetItem(item, text);
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addListItem(xMusicLibraryArtistEntry* entry) {
    addListItem(entry, QString());
}

void xPlayerListWidget::addListItem(xMusicLibraryAlbumEntry* entry) {
    addListItem(entry, QString());
}

void xPlayerListWidget::addListItem(xMusicLibraryTrackEntry* entry) {
    addListItem(entry, QString());
}

void xPlayerListWidget::addListItem(xMovieLibraryEntry* entry) {
    addListItem(entry, QString());
}

void xPlayerListWidget::addListItems(const QStringList& list) {
    for (const auto& element : list) {
        addListItem(element);
    }
}

void xPlayerListWidget::addListItem(xMusicLibraryArtistEntry* entry, const QString& tooltip) {
    auto item = new xPlayerListWidgetItem(entry, this);
    // Add tooltip.
    item->addToolTip(tooltip);
    addListWidgetItem(item, entry->getArtistName());
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addListItem(xMusicLibraryAlbumEntry* entry, const QString& tooltip) {
    auto item = new xPlayerListWidgetItem(entry, this);
    // Add tooltip.
    item->addToolTip(tooltip);
    addListWidgetItem(item, entry->getAlbumName());
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addListItem(xMusicLibraryTrackEntry* entry, const QString& tooltip) {
    auto item = new xPlayerListWidgetItem(entry, this);
    // Add tooltip.
    item->addToolTip(tooltip);
    addListWidgetItem(item, entry->getTrackName());
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addListItem(xMovieLibraryEntry* entry, const QString& tooltip) {
    auto item = new xPlayerListWidgetItem(entry, this);
    // Add tooltip.
    item->addToolTip(tooltip);
    addListWidgetItem(item, entry->getMovieName());
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addListItems(const std::vector<xMusicLibraryTrackEntry*>& entries, const QString& tooltip) {
    for (const auto& entry : entries) {
        auto item = new xPlayerListWidgetItem(entry, this);
        // Add tooltip.
        item->addToolTip(tooltip);
        addListWidgetItem(item, entry->getName());
        // Update filter.
        if (!currentMatch.isEmpty()) {
            item->setHidden(!item->text().contains(currentMatch, Qt::CaseInsensitive));
        }
    }
}

void xPlayerListWidget::addListItems(const QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>& entries) {
    int maxFiles = 0;
    for (const auto& entry : entries) {
        maxFiles += static_cast<int>(entry.second.size());
    }
    addItemWidgetsWorker(entries, entries.begin(), 0, maxFiles);
}

void xPlayerListWidget::addItemWidgetsWorker(const QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>& entries,
                                             QList<std::pair<QString, std::vector<xMusicLibraryTrackEntry*>>>::const_iterator entriesIterator,
                                             int currentFiles, int maxFiles) {
    addListItems(entriesIterator->second, entriesIterator->first);
    currentFiles += static_cast<int>(entriesIterator->second.size());
    emit itemWidgetsProgress(currentFiles, maxFiles);
    if (entriesIterator != entries.end()) {
        auto nextEntriesIterator = ++entriesIterator;
        QTimer::singleShot(50, [=]() { emit itemWidgetsIterate(entries, nextEntriesIterator, currentFiles, maxFiles); });
    }
}

xPlayerListWidgetItem* xPlayerListWidget::listItem(int index) {
    return reinterpret_cast<xPlayerListWidgetItem*>(topLevelItem(index));
}

void xPlayerListWidget::takeListItem(int index) {
    delete takeTopLevelItem(index);
    updateItems();
}

xPlayerListWidgetItem* xPlayerListWidget::itemAt(const QPoint& point) {
    return reinterpret_cast<xPlayerListWidgetItem*>(QTreeWidget::itemAt(point));
}

xPlayerListWidgetItem* xPlayerListWidget::currentItem() {
    return reinterpret_cast<xPlayerListWidgetItem*>(QTreeWidget::currentItem());
}

QList<xPlayerListWidgetItem*> xPlayerListWidget::findListItems(const QString& text) {
    QList<xPlayerListWidgetItem*> findWidgets;
    for (auto item : QTreeWidget::findItems(text, Qt::MatchExactly)) {
        findWidgets.push_back(reinterpret_cast<xPlayerListWidgetItem*>(item));
    }
    return findWidgets;
}

bool xPlayerListWidget::setCurrentItem(const QString& text) {
    auto items = findListItems(text);
    for (auto item : items) {
        QTreeWidget::setCurrentItem(item);
    }
    return !items.isEmpty();
}

void xPlayerListWidget::clearItems() {
    if ((updateItemsThread) && (updateItemsThread->isRunning())){
        // We need to cleanly end the update thread.
        updateItemsThread->requestInterruption();
        updateItemsThread->wait();
        // Delete and reset to nullptr.
        delete updateItemsThread;
        updateItemsThread = nullptr;
    }
    // Clear all posted events for this since time update may still be in the queue.
    QCoreApplication::removePostedEvents(this);
    // Clear the tree widget.
    QTreeWidget::clear();
    // Clear any total time displayed.
    emit totalTime(0);
}

void xPlayerListWidget::updateFilter(const QString &match) {
    currentMatch = match;
    if (match.isEmpty()) {
        for (int i = 0; i < topLevelItemCount(); ++i) {
            topLevelItem(i)->setHidden(false);
        }
    } else {
        for (int i = 0; i < topLevelItemCount(); ++i) {
            auto item = topLevelItem(i);
            item->setHidden(!item->text(0).contains(match, Qt::CaseInsensitive));
        }
    }
}

void xPlayerListWidget::setCurrentListIndex(int index) {
    QTreeWidget::setCurrentItem(topLevelItem(index));
}

int xPlayerListWidget::currentListIndex() {
    auto item = QTreeWidget::currentItem();
    return (item) ? indexOfTopLevelItem(item) : -1;
}

int xPlayerListWidget::listIndex(xPlayerListWidgetItem* item) {
    return indexOfTopLevelItem(item);
}

int xPlayerListWidget::count() {
    return topLevelItemCount();
}

void xPlayerListWidget::scrollToIndex(int index) {
    scrollToItem(listItem(index));
}

void xPlayerListWidget::refreshItems(std::function<bool (xPlayerListWidgetItem*, xPlayerListWidgetItem*)> lesserThan) {
    std::vector<xPlayerListWidgetItem*> items(count());
    auto cItem = currentItem();
    for (auto index = count()-1; index >= 0; --index) {
        items[index] = reinterpret_cast<xPlayerListWidgetItem*>(QTreeWidget::takeTopLevelItem(index));
    }
    // Only sort item if a function is provided.
    if (lesserThan != nullptr) {
        std::sort(items.begin(), items.end(), std::move(lesserThan));
    }
    for (auto item : items) {
        addListWidgetItem(item, item->text());
    }
    // Set current item after rebuilding the list.
    if (cItem) {
        QTreeWidget::setCurrentItem(cItem);
    }
    // Update hidden items according to the current filter.
    updateFilter(currentMatch);
}

void xPlayerListWidget::updateItems() {
    if ((updateItemsThread) && (updateItemsThread->isRunning())) {
        updateItemsThread->requestInterruption();
        updateItemsThread->wait();
        delete updateItemsThread;
    }
    updateItemsThread = QThread::create([this]() { updateItemsWorker(); });
    connect(updateItemsThread, &QThread::finished, this, &xPlayerListWidget::updateItemsWorkerFinished);
    updateItemsThread->start();
}

void xPlayerListWidget::updateItemsWorker() {
    qint64 total = 0;
    for (int index = 0; index < topLevelItemCount(); ++index) {
        // Check if we want to end the thread.
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        auto item = listItem(index);
        total += item->updateTime();
        emit updateTime(item);
    }
    emit totalTime(total);
}

void xPlayerListWidget::updateItemsWorkerFinished() {
}

void xPlayerListWidget::addListWidgetItem(xPlayerListWidgetItem* item, const QString& text) {
    if ((sortItems) && (topLevelItemCount() > 0)) {
        int insertPos = 0;
        for (; insertPos < topLevelItemCount(); ++insertPos) {
            // Sorting is case-insensitive.
            if (text.compare(QTreeWidget::itemAt(insertPos, 0)->text(0), Qt::CaseInsensitive) < 0) {
                break;
            }
        }
        QTreeWidget::insertTopLevelItem(insertPos, item);
    } else {
        QTreeWidget::addTopLevelItem(item);
    }
}

void xPlayerListWidget::resizeEvent(QResizeEvent* event) {
    if (event) {
        if (event->oldSize().width() != event->size().width()) {
            for (int i = 0; i < topLevelItemCount(); ++i) {
                auto item = reinterpret_cast<xPlayerListWidgetItem*>(topLevelItem(i));
                item->updateToolTip();
            }
        }
    }
    QTreeWidget::resizeEvent(event);
}

void xPlayerListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event) {
        // currentListIndex is more accurate than using the event position.
        // converting the position sometimes leads to an incorrect listIndex.
        dragDropFromIndex = currentListIndex();
        QTreeWidget::dragEnterEvent(event);
    }
}

void xPlayerListWidget::dropEvent(QDropEvent* event) {
    if (event) {
        auto dropPosition = event->posF();
        auto dragDropToItem = itemAt(event->pos());
        if (dragDropToItem) {
            auto dropItemRect = visualItemRect(dragDropToItem);
            dragDropToIndex = indexOfTopLevelItem(dragDropToItem);
            // We insert before the current element if the position is in the upper half of the listItem widget.
            if (dropPosition.y() > dropItemRect.y() + (dropItemRect.height()/2.0)) {
                dropPosition.setY(dropItemRect.bottom());
                ++dragDropToIndex;
            } else {
                dropPosition.setY(dropItemRect.top());
            }
        } else {
            // No listItem at position means move to the end of the list.
            dragDropToIndex = topLevelItemCount();
        }
        auto adjustedEvent = new QDropEvent(dropPosition, event->possibleActions(), event->mimeData(),
                                            event->mouseButtons(), event->keyboardModifiers(), event->type());
        QTreeWidget::dropEvent(adjustedEvent);
        emit dragDrop(dragDropFromIndex, dragDropToIndex);
    }
}
