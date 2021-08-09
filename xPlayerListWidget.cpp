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

#include <QApplication>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>

xPlayerListItemWidget::xPlayerListItemWidget(const QString& text, QWidget* parent):
        QWidget(parent),
        itemTimeLabel(nullptr),
        itemTimeUpdated(true),
        itemTime(0),
        itemText(text),
        itemTextShortened(false),
        itemInitialized(false),
        itemCurrentWidth(0),
        itemToolTip(),
        itemFile(nullptr) {
    itemLayout = new xPlayerLayout();
    itemLayout->setSpacing(0);
    itemLayout->setAlignment(Qt::AlignCenter);
    itemIconLabel = new QLabel(this);
    itemIconLabel->setAlignment(Qt::AlignCenter);
    itemTextLabel = new QLabel(text, this);
    itemTextLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    setContentsMargins(0, 0, 0, 0);
    // The overlay of icon and name if on purpose.
    itemLayout->addWidget(itemIconLabel, 0, 0);
    itemLayout->addColumnSpacer(1, xPlayerLayout::SmallSpace);
    itemLayout->addWidget(itemTextLabel, 0, 0, 1, 10);
    itemLayout->setColumnStretch(2, 2);
    itemLayout->addColumnSpacer(10, xPlayerLayout::SmallSpace);
    setLayout(itemLayout);
    // Setting fixed height will trigger update event. We will ignore this first one.
    setFixedHeight(sizeHint().height()); // NOLINT
}

xPlayerListItemWidget::xPlayerListItemWidget(xMusicFile* file, QWidget* parent):
        QWidget(parent),
        itemTimeUpdated(false),
        itemTime(0),
        itemText(),
        itemTextShortened(false),
        itemInitialized(false),
        itemCurrentWidth(0),
        itemToolTip(),
        itemFile(file),
        itemTextInitialized(false) {
    static auto timeLabelWidth = QFontMetrics(QApplication::font()).width("99:99");
    itemLayout = new xPlayerLayout();
    itemLayout->setSpacing(0);
    itemLayout->setAlignment(Qt::AlignCenter);
    itemIconLabel = new QLabel(this);
    itemIconLabel->setAlignment(Qt::AlignCenter);
    if (itemFile) {
        itemText = itemFile->getTrackName();
    }
    itemTextLabel = new QLabel(this);
    itemTextLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    itemTimeLabel = new QLabel(this);
    itemTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    itemTimeLabel->setFixedWidth(timeLabelWidth);
    // Only update the time in the constructor if it was already scanned. The update will force a scan if necessary.
    if (itemFile->isScanned()) {
        updateTime();
    }
    setContentsMargins(0, 0, 0, 0);
    // The overlay of icon and name if on purpose.
    itemLayout->addWidget(itemIconLabel, 0, 0);
    itemLayout->addColumnSpacer(1, xPlayerLayout::SmallSpace);
    itemLayout->addWidget(itemTextLabel, 0, 0, 1, 10);
    itemLayout->setColumnStretch(2, 2);
    itemLayout->addColumnSpacer(10, xPlayerLayout::SmallSpace);
    itemLayout->addWidget(itemTimeLabel, 0, 11, 1, 2);
    itemLayout->setColumnStretch(11, 0);
    itemLayout->setColumnStretch(12, 0);
    setLayout(itemLayout);
    itemInitialized = false;
    // Setting fixed height will trigger update event. We will ignore this first one.
    setFixedHeight(sizeHint().height()); // NOLINT
    // Compute font related sizes.
    static auto textFontMetrics = QFontMetrics(QApplication::font());
    itemTextWidth = textFontMetrics.width(itemText);
    // Average width of a single character, round upwards.
    itemTextCharacterWidth = (itemTextWidth + itemText.length() - 1) / itemText.length();
    itemFixedWidth = itemTimeLabel->width() + xPlayerLayout::HugeSpace + xPlayerLayout::MediumSpace;
}

void xPlayerListItemWidget::setIcon(const QString& fileName) {
    if (fileName.isEmpty()) {
        removeIcon();
    } else {
        itemIconLabel->setPixmap(QIcon(fileName).pixmap(xPlayerLayout::MediumSpace, xPlayerLayout::MediumSpace));
        itemIconLabel->setFixedWidth(xPlayerLayout::MediumSpace);
        // Re-align due to extra space in second layout columns.
        itemLayout->removeWidget(itemTextLabel);
        itemLayout->addWidget(itemTextLabel, 0, 2, 1, 8);
    }
}

void xPlayerListItemWidget::removeIcon() {
    if (itemIconLabel != nullptr) {
        itemIconLabel->clear();
        itemLayout->removeWidget(itemTextLabel);
        // Re-align due to extra space in second layout columns.
        itemLayout->addWidget(itemTextLabel, 0, 0, 1, 10);
    }
}

void xPlayerListItemWidget::addToolTip(const QString& text) {
    itemToolTip = text;
    updateToolTip();
}

void xPlayerListItemWidget::removeToolTip() {
    itemToolTip.clear();
    updateToolTip();
}

const QString& xPlayerListItemWidget::text() const {
    return itemText;
}

xMusicFile* xPlayerListItemWidget::musicFile() const {
    return itemFile;
}

qint64 xPlayerListItemWidget::updateTime() {
    if ((!itemTimeUpdated) && (itemFile)) {
        itemTime = itemFile->getLength();
        itemTimeLabel->setText(QString("%1:%2").arg(itemTime/60000).
                arg((itemTime/1000)%60, 2, 10, QChar('0')));
        itemTimeUpdated = true;
    }
    return itemTime;
}

void xPlayerListItemWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (event != nullptr) {
        auto itemWidth = event->size().width();
        if (itemCurrentWidth != itemWidth) {
            itemCurrentWidth = itemWidth;
            if (itemInitialized) {
                updateTrackName(event->size().width());
            } else {
                itemInitialized = true;
            }
        }
    }
}

void xPlayerListItemWidget::updateTrackName(int width) {
    // We do not shorten the track name if we do not have displayed time or the width is sufficient.
    if ((itemTimeLabel == nullptr) || ((itemTextInitialized) && (width > itemTextWidth+itemFixedWidth))) {
        return;
    }
    itemTextInitialized = true;
    if (width < itemTextWidth+itemFixedWidth) {
        itemTextLabel->setText(shortenedTrackName(width-itemFixedWidth));
        // Only update if the state has changed.
        if (!itemTextShortened) {
            itemTextShortened = true;
            updateToolTip();
        }
    } else {
        // Only update if the state has changed.
        if (itemTextShortened) {
            itemTextShortened = false;
            updateToolTip();
        }
        itemTextLabel->setText(itemText);
    }
}

void xPlayerListItemWidget::updateToolTip() {
    if (itemToolTip.isEmpty()) {
        if (itemTextShortened) {
            setToolTip(itemText);
        } else {
            setToolTip(QString());
        }
    } else {
        if (itemTextShortened) {
            setToolTip(itemText + "\n" + itemToolTip);
        } else {
            setToolTip(itemToolTip);
        }
    }
}

QString xPlayerListItemWidget::shortenedTrackName(int width) {
    auto text = itemText;
    // Determine the number of characters to be removed in order to fit.
    auto removeCharacters = 3 + (itemTextWidth - width + itemTextCharacterWidth - 1) / itemTextCharacterWidth;
    text.remove(-removeCharacters, removeCharacters);
    return text+"...";
}


xPlayerListWidget::xPlayerListWidget(QWidget* parent):
        QListWidget(parent),
        sortItems(false),
        mapItems(),
        updateItemsThread(nullptr),
        dragDropFromIndex(-1),
        dragDropToIndex(-1),
        currentMatch() {
    connect(this, &xPlayerListWidget::itemWidgetsIterate, this, &xPlayerListWidget::addItemWidgetsWorker);
}

void xPlayerListWidget::enableSorting(bool sorted) {
    sortItems = sorted;
}
void xPlayerListWidget::addItemWidget(const QString& text) {
    addItemWidget(text, QString());
}

void xPlayerListWidget::addItemWidget(const QString& text, const QString& tooltip) {
    auto item = new QListWidgetItem();
    addListWidgetItem(item, text);
    auto widget = new xPlayerListItemWidget(text, this);
    // Update tooltip.
    if (tooltip.isEmpty()) {
        widget->removeToolTip();
    } else {
        widget->addToolTip(tooltip);
    }
    setItemWidget(item, widget);
    item->setSizeHint(widget->sizeHint());
    mapItems[item] = widget;
    // Update filter.
    if (!currentMatch.isEmpty()) {
        item->setHidden(!text.contains(currentMatch, Qt::CaseInsensitive));
    }
}

void xPlayerListWidget::addItemWidget(xMusicFile* file) {
    addItemWidget(file, QString());
}

void xPlayerListWidget::addItemWidgets(const QStringList& list) {
    for (const auto& element : list) {
        addItemWidget(element);
    }
}

void xPlayerListWidget::addItemWidget(xMusicFile* file, const QString& tooltip) {
    auto item = new QListWidgetItem();
    addListWidgetItem(item, file->getTrackName());
    auto widget = new xPlayerListItemWidget(file, this);
    // Update tooltip.
    if (!tooltip.isEmpty()) {
        widget->addToolTip(tooltip);
    }
    setItemWidget(item, widget);
    item->setSizeHint(widget->sizeHint());
    mapItems[item] = widget;
    if (!currentMatch.isEmpty()) {
        updateFilter(currentMatch);
    }
}

void xPlayerListWidget::addItemWidgets(const std::vector<xMusicFile*>& files, const QString& tooltip) {

    auto addTooltip = !tooltip.isEmpty();
    for (const auto& file : files) {
        auto item = new QListWidgetItem();
        if (!sortItems) {
            QListWidget::addItem(item);
        } else {
            addListWidgetItem(item, file->getTrackName());
        }
        auto widget = new xPlayerListItemWidget(file, this);
        // Update tooltip.
        if (addTooltip) {
            widget->addToolTip(tooltip);
        }
        setItemWidget(item, widget);
        item->setSizeHint(widget->sizeHint());
        mapItems[item] = widget;
        // Update filter.
        if (!currentMatch.isEmpty()) {
            item->setHidden(!item->text().contains(currentMatch, Qt::CaseInsensitive));
        }
    }
}
void xPlayerListWidget::addItemWidgets(const QList<std::pair<QString, std::vector<xMusicFile*>>>& files) {
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
    addItemWidgets(filesIterator->second, filesIterator->first);
    currentFiles += static_cast<int>(filesIterator->second.size());
    emit itemWidgetsProgress(currentFiles, maxFiles);
    if (filesIterator != files.end()) {
        auto nextFilesIterator = ++filesIterator;
        QTimer::singleShot(50, [=]() { emit itemWidgetsIterate(files, nextFilesIterator, currentFiles, maxFiles); });
    }
}


xPlayerListItemWidget* xPlayerListWidget::itemWidget(int index) {
    auto item = QListWidget::item(index);
    auto pos = mapItems.find(item);
    if (pos != mapItems.end()) {
        return pos->second;
    }
    return nullptr;
}

void xPlayerListWidget::takeItemWidget(int index) {
    auto item = QListWidget::item(index);
    auto pos = mapItems.find(item);
    if (pos != mapItems.end()) {
        mapItems.erase(pos);
        delete QListWidget::takeItem(index);
        updateItems();
    }
}

xPlayerListItemWidget* xPlayerListWidget::itemWidgetAt(const QPoint& point) {
    auto item = QListWidget::itemAt(point);
    auto pos = mapItems.find(item);
    if (pos != mapItems.end()) {
        return pos->second;
    }
    return nullptr;
}

xPlayerListItemWidget* xPlayerListWidget::currentItemWidget() {
    return itemWidget(QListWidget::currentRow());
}

QList<xPlayerListItemWidget*> xPlayerListWidget::findItemWidgets(const QString& text) {
    QList<xPlayerListItemWidget*> findWidgets;
    for (const auto& entry : mapItems) {
        if (entry.second->text() == text) {
            findWidgets.push_back(entry.second);
        }
    }
    return findWidgets;
}

bool xPlayerListWidget::setCurrentWidgetItem(const QString& text) {
    for (const auto& entry : mapItems) {
        if (entry.second->text() == text) {
            QListWidget::setCurrentItem(entry.first);
            return true;
        }
    }
    return false;
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
    mapItems.clear();
    QListWidget::clear();
    // Clear any total time displayed.
    emit totalTime(0);
}

void xPlayerListWidget::updateFilter(const QString &match) {
    currentMatch = match;
    if (match.isEmpty()) {
        for (auto& item : mapItems) {
            item.first->setHidden(false);
        }
    } else {
        for (auto& item : mapItems) {
            item.first->setHidden(!item.second->text().contains(match, Qt::CaseInsensitive));
        }
    }
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
    for (auto& entry : mapItems) {
        total += entry.second->updateTime();
        // Check if we want to end the thread.
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
    }
    emit totalTime(total);
}

void xPlayerListWidget::updateItemsWorkerFinished() {
}

void xPlayerListWidget::addListWidgetItem(QListWidgetItem *item, const QString& text) {
    if ((sortItems) && (QListWidget::count() > 0)) {
        int insertPos = 0;
        for (; insertPos < QListWidget::count(); ++insertPos) {
            if (text < this->itemWidget(insertPos)->text()) {
                break;
            }
        }
        QListWidget::insertItem(insertPos, item);
    } else {
        QListWidget::addItem(item);
    }
}

void xPlayerListWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event) {
        // currentRow is more accurate than using the event position.
        // converting the position sometimes leads to an incorrect index.
        dragDropFromIndex = currentRow();
        QListWidget::dragEnterEvent(event);
    }
}

void xPlayerListWidget::dropEvent(QDropEvent* event) {
    if (event) {
        auto dragDropToItem = itemAt(event->pos());
        if (dragDropToItem) {
            dragDropToIndex = row(dragDropToItem);
            // We insert before the current element if the position is in the upper half of the item widget.
            auto toItemHeight = QListWidget::itemWidget(dragDropToItem)->height() / 2;
            auto dragDropToIndexMinus = row(itemAt(event->pos()-QPoint(0, toItemHeight)));
            // Determine if we insert after or before the drop index.
            if (dragDropToIndex == dragDropToIndexMinus) {
                ++dragDropToIndex;
            }
        } else {
            // No item at position means move to the end of the list.
            dragDropToIndex = count();
        }
        QListWidget::dropEvent(event);
        emit dragDrop(dragDropFromIndex, dragDropToIndex);
    }
}

