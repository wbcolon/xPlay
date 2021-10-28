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

#include "xMainMobileSyncWidget.h"
#include "xPlayerUI.h"
#include "xMusicFile.h"

#include <QApplication>
#include <QMenu>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QProgressBar>
#include <QCheckBox>
#include <QDebug>

xMainMobileSyncWidget::xMainMobileSyncWidget(xMusicLibrary* library, QWidget* parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicLibrary(library),
        musicLibraryExisting(),
        mobileLibrarySpaceInfo(),
        actionThread(nullptr) {
    // Library tree sections.
    auto layout = new xPlayerLayout(this);
    musicLibraryWidget = new xPlayerMusicLibraryWidget(musicLibrary, tr("Music Library"), this);
    musicLibraryExistingWidget = new QListWidget(this);
    mobileLibrary = new xMusicLibrary();
    mobileLibraryWidget = new xPlayerMusicLibraryWidget(mobileLibrary, tr("Mobile Library"), this);
    // Button bar for libraries.
    auto musicLibraryCompareButton = new QPushButton(tr("Compare"), this);
    auto musicLibrarySortBySize = new QCheckBox(tr("Sort by Size"), this);
    mobileLibraryStorageBar = new QProgressBar(this);
    auto mobileLibraryDirectoryButton = new QPushButton(tr("Open..."), this);
    mobileLibraryDirectoryWidget = new QLineEdit(this);
    auto mobileLibraryScanClearButton = new QPushButton(tr("Clear"), this);
    auto musicLibraryMarksButton = new QPushButton(tr("Marks"), this);
    auto musicLibraryMarksMenu = new QMenu(this);
    musicLibraryMarksMenu->addAction(tr("Save Mobile Library to Existing"), this, &xMainMobileSyncWidget::musicLibrarySaveExisting);
    musicLibraryMarksMenu->addAction(tr("Mark Existing"), this, &xMainMobileSyncWidget::musicLibraryMarkExisting);
    musicLibraryMarksMenu->addAction(tr("Clear Marks for Existing"), this, &xMainMobileSyncWidget::musicLibraryClearExisting);
    musicLibraryMarksMenu->addAction(tr("Clear all Marks"), musicLibraryWidget, &xPlayerMusicLibraryWidget::clearItems);
    musicLibraryMarksButton->setMenu(musicLibraryMarksMenu);
    // Action section.
    auto actionAddToGroupBox = new QGroupBox(tr("Add to Mobile Library"), this);
    actionAddToGroupBox->setFlat(true);
    auto actionAddToLayout = new xPlayerLayout(actionAddToGroupBox);
    actionAddToWidget = new QListWidget(actionAddToGroupBox);
    actionAddToWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    actionAddToLayout->addWidget(actionAddToWidget);
    actionAddToGroupBox->setLayout(actionAddToLayout);
    auto actionRemoveFromGroupBox = new QGroupBox(tr("Remove from Mobile Library"), this);
    actionRemoveFromGroupBox->setFlat(true);
    auto actionRemoveFromLayout = new xPlayerLayout(actionRemoveFromGroupBox);
    actionRemoveFromWidget = new QListWidget(actionRemoveFromGroupBox);
    actionRemoveFromWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    actionRemoveFromLayout->addWidget(actionRemoveFromWidget);
    actionRemoveFromGroupBox->setLayout(actionRemoveFromLayout);
    auto actionLayout = new xPlayerLayout();
    actionLayout->addWidget(actionAddToGroupBox, 0, 0, 7, 4);
    actionLayout->addWidget(actionRemoveFromGroupBox, 7, 0, 7, 4);
    // Button bar for action section.
    actionStorageBar = new QProgressBar(this);
    actionApplyButton = new QPushButton(tr("Apply"), this);
    actionBar = new QProgressBar(this);
    actionBar->setVisible(false);
    // Setup layout.
    // Music library layout
    layout->addWidget(musicLibraryWidget, 0, 0, 14, 10);
    layout->addRowSpacer(14, xPlayerLayout::SmallSpace);
    layout->addWidget(musicLibraryMarksButton, 15, 0, 1, 2);
    layout->addWidget(musicLibraryCompareButton, 16, 0, 1, 2);
    layout->addWidget(musicLibraryExistingWidget, 15, 2, 2, 6);
    layout->addWidget(musicLibrarySortBySize, 16, 8, 1, 2);
    layout->addColumnSpacer(10, xPlayerLayout::SmallSpace);
    // Action layout.
    layout->addLayout(actionLayout, 0, 11, 14, 8);
    layout->addWidget(actionStorageBar, 15, 11, 1, 8);
    layout->addWidget(actionApplyButton, 16, 17, 1, 2);
    layout->addWidget(actionBar, 16, 11, 1, 6);
    layout->addColumnSpacer(19, xPlayerLayout::SmallSpace);
    // Mobile library layout
    layout->addWidget(mobileLibraryWidget, 0, 20, 14, 10);
    layout->addWidget(mobileLibraryStorageBar, 15, 20, 1, 10);
    layout->addWidget(mobileLibraryDirectoryButton, 16, 20, 1, 2);
    layout->addWidget(mobileLibraryDirectoryWidget, 16, 22, 1, 6);
    layout->addWidget(mobileLibraryScanClearButton, 16, 28, 1, 2);
    this->setLayout(layout);
    // Connect signals.
    connect(mobileLibraryDirectoryButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::mobileLibraryOpenDirectory);
    connect(mobileLibraryScanClearButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::mobileLibraryScanClear);
    connect(musicLibraryCompareButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::musicLibraryCompare);
    connect(mobileLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlClicked, this, &xMainMobileSyncWidget::musicLibraryFindItem);
    connect(musicLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlClicked, this, &xMainMobileSyncWidget::mobileLibraryFindItem);
    connect(mobileLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlRightClicked, this, &xMainMobileSyncWidget::actionRemoveFrom);
    connect(musicLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlRightClicked, this, &xMainMobileSyncWidget::actionAddTo);
    connect(musicLibrarySortBySize, &QCheckBox::clicked, musicLibraryWidget, &xPlayerMusicLibraryWidget::setSortBySize);
    connect(actionAddToWidget, &QListWidget::customContextMenuRequested, this, &xMainMobileSyncWidget::actionAddToDelete);
    connect(actionRemoveFromWidget, &QListWidget::customContextMenuRequested, this, &xMainMobileSyncWidget::actionRemoveFromDelete);
    connect(actionApplyButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::actionApply);
    // Update text on scan button based on the content of the mobile library directory entry.
    connect(mobileLibraryDirectoryWidget, &QLineEdit::textChanged, [=](const QString& text) {
        if (text.isEmpty()) {
            mobileLibraryScanClearButton->setText(tr("Clear"));
        } else {
            mobileLibraryScanClearButton->setText(tr("Scan"));
        }
    });
}

void xMainMobileSyncWidget::initializeView() {
}

void xMainMobileSyncWidget::clear() {
    // Clear all markings in library trees
    mobileLibraryWidget->clearItems();
    musicLibraryWidget->clearItems();
    // Remove all actions and clear lists.
    actionRemoveFromWidget->clear();
    actionRemoveFromItems.clear();
    actionAddToWidget->clear();
    actionAddToItems.clear();
    // Update action storage.
    updateActionStorage();
}

void xMainMobileSyncWidget::actionAddTo(xPlayerMusicLibraryWidgetItem* item) {
    for (auto addToItem : actionAddToItems) {
        if (item == addToItem) {
            return;
        }
        if (item->isChildOf(addToItem)) {
            return;
        }
    }
    // Check if we can remove other items when adding the new one.
    for (int i = static_cast<int>(actionAddToItems.size())-1; i >= 0; --i) {
        if (actionAddToItems[i]->isChildOf(item)) {
            actionAddToItems.erase(actionAddToItems.begin()+i);
            actionAddToWidget->takeItem(i);
        }
    }
    // Mark item.
    item->mark(Qt::BDiagPattern, true);
    actionAddToWidget->addItem(item->description());
    actionAddToItems.push_back(item);
    // Update action storage.
    updateActionStorage();
}

void xMainMobileSyncWidget::actionRemoveFrom(xPlayerMusicLibraryWidgetItem* item) {
    // Check if we need to add the item.
    for (auto removeFromItem : actionRemoveFromItems) {
        if (item == removeFromItem) {
            return;
        }
        if (item->isChildOf(removeFromItem)) {
            return;
        }
    }
    // Check if we can remove other items when adding the new one.
    for (int i = static_cast<int>(actionRemoveFromItems.size())-1; i >= 0; --i) {
        if (actionRemoveFromItems[i]->isChildOf(item)) {
            actionRemoveFromItems.erase(actionRemoveFromItems.begin()+i);
            actionRemoveFromWidget->takeItem(i);
        }
    }
    // Mark item.
    item->mark(Qt::FDiagPattern, true);
    actionRemoveFromWidget->addItem(item->description());
    actionRemoveFromItems.push_back(item);
    // Update action storage.
    updateActionStorage();
}

void xMainMobileSyncWidget::actionAddToDelete(const QPoint& point) {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        auto item = actionAddToWidget->itemAt(point);
        if (!item) {
            return;
        }
        auto index = actionAddToWidget->row(item);
        // remove element from vector.
        auto addToItemRemove = actionAddToItems[index];
        actionAddToItems.erase(actionAddToItems.begin()+index);
        // remove element from list widget.
        actionAddToWidget->takeItem(index);
        // clear the item and re-mark the remainder.
        addToItemRemove->restoreMark(true);
        for (auto addToItem : actionAddToItems) {
            addToItem->mark(Qt::BDiagPattern, true);
        }
        updateActionStorage();
    }
}

void xMainMobileSyncWidget::actionRemoveFromDelete(const QPoint& point) {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        auto item = actionRemoveFromWidget->itemAt(point);
        if (!item) {
            return;
        }
        auto index = actionRemoveFromWidget->row(item);
        // remove element from vector.
        auto removeFromItemRemove = actionRemoveFromItems[index];
        actionRemoveFromItems.erase(actionRemoveFromItems.begin()+index);
        // remove element from list widget.
        actionRemoveFromWidget->takeItem(index);
        // clear the item and re-mark the remainder.
        removeFromItemRemove->restoreMark(true);
        for (auto removeFromItem : actionRemoveFromItems) {
            removeFromItem->mark(Qt::FDiagPattern, true);
        }
        updateActionStorage();
    }
}

void xMainMobileSyncWidget::actionApplyThread(const std::list<xPlayerMusicLibraryWidgetItem*>& actionAddToExpandedItems) {
    // Remove files.
    int actionCount = 0;
    for (auto removeFromItem : actionRemoveFromItems) {
        // Allow interruptions of remove operations.
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        try {
            if (removeFromItem->musicFile()) {
                std::filesystem::remove(removeFromItem->musicFile()->getFilePath());
            } else {
                std::filesystem::remove_all(removeFromItem->musicDirectory().path());
            }
        } catch (const std::filesystem::filesystem_error& error) {
            qCritical() << "Unable to remove item: " << removeFromItem->description() << ", error: " << error.what();
        }
        // Update action progress.
        emit actionApplyProgress(++actionCount);
    }
    // Copy files.
    for (auto addToItem : actionAddToExpandedItems) {
        // Allow interruption of copy operations.
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }
        try {
            std::filesystem::path sourcePath;
            std::filesystem::path destinationPath;
            std::error_code errorCode;
            // Determine source and destination path.
            destinationPath = mobileLibrary->getBaseDirectory();
            if (addToItem->musicFile()) {
                sourcePath = addToItem->musicFile()->getFilePath();
                destinationPath = destinationPath /
                                  addToItem->artist()->musicDirectory().name().toStdString() /
                                  addToItem->album()->musicDirectory().name().toStdString();
            } else {
                sourcePath = addToItem->musicDirectory().path();
                if (addToItem->artist()) {
                    destinationPath = destinationPath /
                                      addToItem->artist()->musicDirectory().name().toStdString() /
                                      addToItem->musicDirectory().name().toStdString();
                } else {
                    destinationPath = destinationPath /
                                      addToItem->musicDirectory().name().toStdString();
                }
            }
            // We do ignore any error while creating directories.
            std::filesystem::create_directories(destinationPath, errorCode);
            // Copy recursively
            std::filesystem::copy(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing |
                                                               std::filesystem::copy_options::recursive);
            // Update action progress.
            emit actionApplyProgress(++actionCount);
        } catch (const std::filesystem::filesystem_error& error) {
            qCritical() << "Unable to copy item: " << addToItem->description() << ", error: " << error.what();
        }
    }
}

void xMainMobileSyncWidget::actionApplyUpdate(int progress) {
    actionBar->setValue(progress);
    // Determine available space and capacity.
    auto currentSpaceInfo = std::filesystem::space(mobileLibrary->getBaseDirectory());
    actionStorageBar->setRange(0, static_cast<int>(currentSpaceInfo.capacity / 1048576));
    actionStorageBar->setValue(static_cast<int>((currentSpaceInfo.capacity-currentSpaceInfo.available) / 1048576));
    // Set the progress bar text.
    actionStorageBar->setFormat(storageBarFormat(currentSpaceInfo.available, currentSpaceInfo.capacity));
}

void xMainMobileSyncWidget::actionApplyFinished() {
    // Rescan the library.
    mobileLibraryScanClear();
    // Hide action bar.
    actionBar->setVisible(false);
    // Enable widgets.
    musicLibraryWidget->setEnabled(true);
    mobileLibraryWidget->setEnabled(true);
    actionAddToWidget->setEnabled(true);
    actionRemoveFromWidget->setEnabled(true);
    disconnect(this, &xMainMobileSyncWidget::actionApplyProgress, this, &xMainMobileSyncWidget::actionApplyUpdate);
    actionApplyButton->setText("Apply");
}

void xMainMobileSyncWidget::actionApply() {
    // Check if thread is already running.
    if ((actionThread) && (actionThread->isRunning())) {
        actionThread->requestInterruption();
        actionThread->wait();
        // Enable widgets.
        musicLibraryWidget->setEnabled(true);
        mobileLibraryWidget->setEnabled(true);
        actionAddToWidget->setEnabled(true);
        actionRemoveFromWidget->setEnabled(true);
        // Cleanup.
        actionApplyButton->setText(tr("Apply"));
        mobileLibraryScanClear();
    }
    // Expand AddTo artist items to multiple artist/album items.
    std::list<xPlayerMusicLibraryWidgetItem*> actionAddToExpandedItems;
    for (auto addToItem : actionAddToItems) {
        // We do not need to expand if we at least have an artist.
        if (addToItem->artist()) {
            actionAddToExpandedItems.emplace_back(addToItem);
        } else {
            // Add all children of the artist instead of the artist itself for shorter recursive copies.
            for (int i = 0; i < addToItem->childCount(); ++i) {
                actionAddToExpandedItems.emplace_back(reinterpret_cast<xPlayerMusicLibraryWidgetItem*>(addToItem->child(i)));
            }
        }
    }
    // Disable widgets.
    musicLibraryWidget->setEnabled(false);
    mobileLibraryWidget->setEnabled(false);
    actionAddToWidget->setEnabled(false);
    actionRemoveFromWidget->setEnabled(false);
    // Prepare action bar and make it visible.
    actionBar->setVisible(true);
    actionBar->setRange(0, static_cast<int>(actionRemoveFromItems.size()+actionAddToExpandedItems.size()));

    connect(this, &xMainMobileSyncWidget::actionApplyProgress, this, &xMainMobileSyncWidget::actionApplyUpdate);
    actionThread = QThread::create([=]() { actionApplyThread(actionAddToExpandedItems); });
    connect(actionThread, &QThread::finished, this, &xMainMobileSyncWidget::actionApplyFinished);
    actionThread->start();
    // Update apply to cancel button.
    actionApplyButton->setText(tr("Cancel"));
}

void xMainMobileSyncWidget::mobileLibraryOpenDirectory() {
    QString mobileLibraryDirectoryPath =
            QFileDialog::getExistingDirectory(this, tr("Open Mobile Library"), mobileLibraryDirectoryWidget->text(),
                                              QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if (mobileLibraryDirectoryPath.isEmpty()) {
        mobileLibraryDirectoryWidget->clear();
    } else {
        mobileLibraryDirectoryWidget->setText(mobileLibraryDirectoryPath);
    }
}

void xMainMobileSyncWidget::mobileLibraryScanClear() {
    // Clear even if the mobile library directory is empty.
    clear();
    auto mobileLibraryDirectory = mobileLibraryDirectoryWidget->text();
    if (!mobileLibraryDirectory.isEmpty()) {
        auto mobileLibraryPath = std::filesystem::path(mobileLibraryDirectory.toStdString());
        mobileLibraryWidget->setBaseDirectory(mobileLibraryPath);
        try {
            // Determine available space and capacity.
            mobileLibrarySpaceInfo = std::filesystem::space(mobileLibraryPath);
            // Update storage bar position.
            mobileLibraryStorageBar->setRange(0, static_cast<int>(mobileLibrarySpaceInfo.capacity/1048576));
            mobileLibraryStorageBar->setValue(static_cast<int>((mobileLibrarySpaceInfo.capacity-mobileLibrarySpaceInfo.available)/1048576));
            // Set the progress bar text.
            mobileLibraryStorageBar->setFormat(storageBarFormat(mobileLibrarySpaceInfo.available,
                                                                mobileLibrarySpaceInfo.capacity));
            // Update action section.
            updateActionStorage();
        } catch (const std::filesystem::filesystem_error& error) {
            qCritical() << "Unable to determine capacity for mobile library.";
        }
        // Remove map of existing files for mobile library on scan.
        musicLibraryExisting.erase(mobileLibraryPath);
        updateExistingList();
    } else {
        mobileLibraryWidget->clear();
        // Clear storage bars.
        mobileLibrarySpaceInfo = std::filesystem::space_info{};
        mobileLibraryStorageBar->setRange(0, 1);
        mobileLibraryStorageBar->setValue(0);
        mobileLibraryStorageBar->setFormat("");
        actionStorageBar->setRange(0, 1);
        actionStorageBar->setValue(0);
        actionStorageBar->setFormat("");
    }
}

void xMainMobileSyncWidget::mobileLibraryFindItem(xPlayerMusicLibraryWidgetItem* item) {
    if (item->musicFile()) {
        mobileLibraryWidget->selectItem(xMusicDirectory(item->musicFile()->getArtist()),
                                        xMusicDirectory(item->musicFile()->getAlbum()));
    } else {
        if (item->artist()) {
            mobileLibraryWidget->selectItem(item->artist()->musicDirectory(), item->musicDirectory());
        } else {
            mobileLibraryWidget->selectItem(item->musicDirectory());
        }
    }
}

void xMainMobileSyncWidget::musicLibraryCompare() {
    // Sanity check.
    if ((!mobileLibraryWidget->isReady()) || (!musicLibraryWidget->isReady())) {
        return;
    }
    // Remove all actions and clear lists.
    actionRemoveFromWidget->clear();
    actionRemoveFromItems.clear();
    actionAddToWidget->clear();
    actionAddToItems.clear();
    updateActionStorage();
    // Perform compare.
    std::list<xMusicDirectory> missingArtists, additionalArtists;
    std::map<xMusicDirectory, std::list<xMusicDirectory>> missingAlbums, additionalAlbums;
    std::list<xMusicFile*> missingTracks, additionalTracks;
    std::pair<std::list<xMusicFile*>, std::list<xMusicFile*>> differentTracks;

    musicLibrary->getLibraryFiles()->compare(mobileLibrary->getLibraryFiles(),
                                             missingArtists, additionalArtists,
                                             missingAlbums, additionalAlbums,
                                             missingTracks, additionalTracks, differentTracks);
    // Mark music and mobile library.
    musicLibraryWidget->markItems(missingArtists, missingAlbums, missingTracks, differentTracks.first);
    mobileLibraryWidget->markItems(additionalArtists, additionalAlbums, additionalTracks, differentTracks.second);
}

void xMainMobileSyncWidget::musicLibraryFindItem(xPlayerMusicLibraryWidgetItem* item) {
    if (item->musicFile()) {
        musicLibraryWidget->selectItem(xMusicDirectory(item->musicFile()->getArtist()),
                                       xMusicDirectory(item->musicFile()->getAlbum()));
    } else {
        if (item->artist()) {
            musicLibraryWidget->selectItem(item->artist()->musicDirectory(), item->musicDirectory());
        } else {
            musicLibraryWidget->selectItem(item->musicDirectory());
        }
    }
}

void xMainMobileSyncWidget::musicLibrarySaveExisting() {
    std::map<xMusicDirectory, std::map<xMusicDirectory, std::list<xMusicFile*>>> equalTracks;
    // Determine equal tracks. The map contains music file pointer from the music library.
    musicLibrary->getLibraryFiles()->compare(mobileLibrary->getLibraryFiles(), equalTracks);
    musicLibraryExisting[mobileLibrary->getBaseDirectory()] = equalTracks;
    // Update list of existing mobile libraries.
    updateExistingList();
}

void xMainMobileSyncWidget::musicLibraryMarkExisting() {
    // Mark the items.
    for (auto& entry : musicLibraryExisting) {
        musicLibraryWidget->markExistingItems(entry.second);
    }
}

void xMainMobileSyncWidget::musicLibraryClearExisting() {
    musicLibraryExisting.clear();
    // Update list of existing mobile libraries.
    updateExistingList();

}

void xMainMobileSyncWidget::updateActionStorage() {
    // Determine total size of add operations.
    std::uintmax_t addSize = 0;
    for (auto addToItem : actionAddToItems) {
        addSize += addToItem->getTotalSize();
    }
    // Determine total size of all remove operations.
    std::uintmax_t removeSize = 0;
    for (auto removeFromItem : actionRemoveFromItems) {
        removeSize += removeFromItem->getTotalSize();
    }
    // Compute size after applied actions and update action storage bar.
    std::uintmax_t actionTotalSize = mobileLibrarySpaceInfo.capacity-mobileLibrarySpaceInfo.available+addSize-removeSize;
    auto actionStorageBarPalette = actionStorageBar->palette();
    if (actionTotalSize >= mobileLibrarySpaceInfo.capacity) {
        double dOverCapacity = static_cast<double>(actionTotalSize-mobileLibrarySpaceInfo.capacity) / 1073741824.0;
        // Set storage bar to max and color it in red.
        actionStorageBarPalette.setColor(QPalette::Highlight, Qt::red);
        actionStorageBar->setRange(0, 100);
        actionStorageBar->setValue(100);
        actionStorageBar->setFormat(QString("Missing %1 GB Storage Space").arg(dOverCapacity, -1, 'f', 2));
    } else {
        // Set storage bar to size after actions and color it in dark green.
        actionStorageBarPalette.setColor(QPalette::Highlight, Qt::darkGreen);
        // update storage bar position.
        actionStorageBar->setRange(0, static_cast<int>(mobileLibrarySpaceInfo.capacity / 1048576));
        actionStorageBar->setValue(static_cast<int>(actionTotalSize / 1048576));
        // set the progress bar text.
        actionStorageBar->setFormat(storageBarFormat(mobileLibrarySpaceInfo.capacity-actionTotalSize,
                                                     mobileLibrarySpaceInfo.capacity));
    }
    actionStorageBar->setPalette(actionStorageBarPalette);
}

void xMainMobileSyncWidget::updateExistingList() {
    musicLibraryExistingWidget->clear();
    for (const auto& existing : musicLibraryExisting) {
        musicLibraryExistingWidget->addItem(QString::fromStdString(existing.first.native()));
    }
}

QString xMainMobileSyncWidget::storageBarFormat(std::uintmax_t available, std::uintmax_t capacity) {
    // available and capacity in GB.
    double dAvailable = static_cast<double>(available) / 1073741824.0;
    double dCapacity = static_cast<double>(capacity) / 1073741824.0;
    return QString(tr("%1 GB of %2 GB available")).arg(dAvailable, -1, 'f', 2).arg(dCapacity, -1, 'f', 2);
}
