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
#include "xMusicFile.h"
#include "xPlayerUI.h"

#include <QResizeEvent>
#include <QHeaderView>
#include <QTimer>
#include <QPointer>
#include <QDebug>
#include <QTreeWidget>
#include <QTreeWidgetItem>

xPlayerListWidgetItem::xPlayerListWidgetItem(const QString& text, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(true),
        itemTime(0),
        itemText(text),
        itemTooltip(),
        itemFile(nullptr) {
    Q_ASSERT(parent != nullptr);
    setText(0, text);
    itemTextWidth = parent->fontMetrics().width(itemText+"...");
}

xPlayerListWidgetItem::xPlayerListWidgetItem(xMusicFile* file, QTreeWidget* parent):
        QTreeWidgetItem(parent),
        itemTimeUpdated(false),
        itemTime(0),
        itemText(),
        itemTextWidth(0),
        itemTooltip(),
        itemFile(file) {
    Q_ASSERT(parent != nullptr);
    if (itemFile) {
        itemText = itemFile->getTrackName();
        // Only update the time in the constructor if it was already scanned. The update will force a scan if necessary.
        if (itemFile->isScanned()) {
            updateTime();
        }
    } else {
        itemText = "<Missing xMusicFile>";
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

xMusicFile* xPlayerListWidgetItem::musicFile() const {
    return itemFile;
}

qint64 xPlayerListWidgetItem::updateTime() {
    if ((!itemTimeUpdated) && (itemFile)) {
        itemTime = itemFile->getLength();
        setTextAlignment(1, Qt::AlignRight);
        setText(1, QString("%1:%2").arg(itemTime/60000).arg((itemTime/1000)%60, 2, 10, QChar('0')));
        itemTimeUpdated = true;
    }
    return itemTime;
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
        setColumnWidth(1, fontMetrics().width("99:99"));
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
        emit listItemDoubleClicked(reinterpret_cast<xPlayerListWidgetItem *>(item));
    });
    connect(this, &QTreeWidget::itemClicked, [=](QTreeWidgetItem* item, int) {
        emit listItemClicked(reinterpret_cast<xPlayerListWidgetItem *>(item));
    });
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

void xPlayerListWidget::addListItem(xMusicFile* file) {
    addListItem(file, QString());
}

void xPlayerListWidget::addListItems(const QStringList& list) {
    for (const auto& element : list) {
        addListItem(element);
    }
}

void xPlayerListWidget::addListItem(xMusicFile* file, const QString& tooltip) {
    auto item = new xPlayerListWidgetItem(file, this);
    // Add tooltip.
    item->addToolTip(tooltip);
    addListWidgetItem(item, file->getTrackName());
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addListItems(const std::vector<xMusicFile*>& files, const QString& tooltip) {
    for (const auto& file : files) {
        auto item = new xPlayerListWidgetItem(file, this);
        // Add tooltip.
        item->addToolTip(tooltip);
        addListWidgetItem(item, file->getTrackName());
        // Update filter.
        if (!currentMatch.isEmpty()) {
            item->setHidden(!item->text().contains(currentMatch, Qt::CaseInsensitive));
        }
    }
}
void xPlayerListWidget::addListItems(const QList<std::pair<QString, std::vector<xMusicFile*>>>& files) {
    int maxFiles = 0;
    for (const auto& file : files) {
        maxFiles += static_cast<int>(file.second.size());
    }
    addItemWidgetsWorker(files, files.begin(), 0, maxFiles);
}

void xPlayerListWidget::addItemWidgetsWorker(const QList<std::pair<QString, std::vector<xMusicFile*>>>& files,
                                             QList<std::pair<QString, std::vector<xMusicFile*>>>::const_iterator filesIterator,
                                             int currentFiles, int maxFiles) {
    qDebug() << "xPlayerListWidget::addItemWidgetsWorker: " << filesIterator->first;
    addListItems(filesIterator->second, filesIterator->first);
    currentFiles += static_cast<int>(filesIterator->second.size());
    emit itemWidgetsProgress(currentFiles, maxFiles);
    if (filesIterator != files.end()) {
        auto nextFilesIterator = ++filesIterator;
        QTimer::singleShot(50, [=]() { emit itemWidgetsIterate(files, nextFilesIterator, currentFiles, maxFiles); });
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
        total += listItem(index)->updateTime();
        // Check if we want to end the thread.
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
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

