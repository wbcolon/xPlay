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
#include "xMusicLibraryTrackEntry.h"
#include "xPlayerConfiguration.h"

#include <QApplication>
#include <QMenu>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QProgressBar>
#include <QCheckBox>
#include <QDebug>

#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <fcntl.h>


const QString pathToDeviceSectorSize { "/sys/dev/block/%1:%2/queue/hw_sector_size" }; // NO_LINT
const QString pathToDeviceStat { "/sys/dev/block/%1:%2/stat" }; // NO_LINT

xMainMobileSyncWidget::xMainMobileSyncWidget(xMusicLibrary* library, QWidget* parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        musicLibrary(library),
        musicLibraryExisting(),
        mobileLibrarySpaceInfo(),
        mobileLibraryIOThread(nullptr),
        actionThread(nullptr) {
    // Library tree sections.
    auto layout = new xPlayerLayout(this);
    musicLibraryWidget = new xPlayerMusicLibraryWidget(musicLibrary, tr("Music Library"), this);
    musicLibraryFilter = new QGroupBox(tr("Filter Artists"), this);
    musicLibraryFilter->setFlat(xPlayer::UseFlatGroupBox);
    auto musicLibraryFilterLayout = new xPlayerLayout(musicLibraryFilter);
    musicLibraryFilterWidget = new QLineEdit(musicLibraryFilter);
    musicLibraryFilterLayout->addWidget(musicLibraryFilterWidget);
    musicLibraryFilter->setLayout(musicLibraryFilterLayout);
    musicLibraryExistingWidget = new QListWidget(this);
    musicLibraryExistingWidget->setEnabled(false);
    musicLibraryMarksButton = new QPushButton(tr("Marks"), this);
    musicLibraryMarksButton->setEnabled(false);
    musicLibraryExistingButton = new QPushButton(tr("Existing"), this);
    musicLibraryExistingButton->setEnabled(false);
    // Mobile library.
    mobileLibrary = new xMusicLibrary();
    mobileLibraryWidget = new xPlayerMusicLibraryWidget(mobileLibrary, tr("Mobile Library"), this);
    mobileLibraryFilter = new QGroupBox(tr("Filter Artists"), this);
    mobileLibraryFilter->setFlat(xPlayer::UseFlatGroupBox);
    auto mobileLibraryFilterLayout = new xPlayerLayout(mobileLibraryFilter);
    mobileLibraryFilterWidget = new QLineEdit(this);
    mobileLibraryFilterLayout->addWidget(mobileLibraryFilterWidget);
    mobileLibraryFilter->setLayout(mobileLibraryFilterLayout);
    // Button bar for libraries.
    musicLibraryCompareButton = new QPushButton(tr("Compare"), this);
    musicLibraryCompareButton->setEnabled(false);
    musicLibrarySortBySize = new QCheckBox(tr("Sort by Size"), this);
    musicLibrarySortBySize->setEnabled(false);
    // Storage and IO bars.
    mobileLibraryStorageBar = new QProgressBar(this);
    auto mobileLibraryIOReadLabel = new QLabel(tr("Reading"), this);
    mobileLibraryIOReadLabel->setAlignment(Qt::AlignCenter);
    mobileLibraryIOReadBar = new QProgressBar(this);
    mobileLibraryIOReadBar->setRange(0, 1);
    auto mobileLibraryIOWriteLabel = new QLabel(tr("Writing"), this);
    mobileLibraryIOWriteLabel->setAlignment(Qt::AlignCenter);
    mobileLibraryIOWriteBar = new QProgressBar(this);
    mobileLibraryIOWriteBar->setRange(0, 1);
    mobileLibraryDirectoryButton = new QPushButton(tr("Open..."), this);
    mobileLibraryDirectoryWidget = new QLineEdit(this);
    mobileLibraryScanClearButton = new QPushButton(tr("Clear"), this);

    auto musicLibraryExistingMenu = new QMenu(this);
    musicLibraryExistingMenu->addAction(tr("Save Mobile Library to Existing"), this,
                                     &xMainMobileSyncWidget::mobileLibrarySaveToExisting);
    musicLibraryExistingMenu->addAction(tr("Remove Saved Mobile Library"), this,
                                        &xMainMobileSyncWidget::mobileLibraryRemoveSaved);
    musicLibraryExistingMenu->addAction(tr("Remove all Saved Mobile Libraries"), this,
                                        &xMainMobileSyncWidget::mobileLibraryRemoveAllSaved);
    musicLibraryExistingButton->setMenu(musicLibraryExistingMenu);

    auto musicLibraryMarksMenu = new QMenu(this);
    musicLibraryMarksMenu->addAction(tr("Add Marks for Existing"), this, &xMainMobileSyncWidget::musicLibraryMarkExisting);
    musicLibraryMarksMenu->addAction(tr("Clear all Marks"), musicLibraryWidget, &xPlayerMusicLibraryWidget::clearItems);
    musicLibraryMarksButton->setMenu(musicLibraryMarksMenu);
    // Action section.
    actionAddToGroupBox = new QGroupBox(tr("Add to Mobile Library"), this);
    actionAddToGroupBox->setFlat(xPlayer::UseFlatGroupBox);
    auto actionAddToLayout = new xPlayerLayout(actionAddToGroupBox);
    actionAddToWidget = new QListWidget(actionAddToGroupBox);
    actionAddToWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    actionAddToLayout->addWidget(actionAddToWidget);
    actionAddToGroupBox->setLayout(actionAddToLayout);
    actionRemoveFromGroupBox = new QGroupBox(tr("Remove from Mobile Library"), this);
    actionRemoveFromGroupBox->setFlat(xPlayer::UseFlatGroupBox);
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
    actionApplyButton->setEnabled(false);
    // Action bar is only visible if action is applied.
    actionBarLabel = new QLabel(tr("Syncing"), this);
    actionBarLabel->setAlignment(Qt::AlignCenter);
    actionBarLabel->setVisible(false);
    actionBar = new QProgressBar(this);
    actionBar->setVisible(false);
    // Setup layout.
    // Music library layout
    layout->addWidget(musicLibraryWidget, 0, 0, 13, 10);
    layout->addWidget(musicLibraryFilter, 13, 0, 1, 10);
    layout->addRowSpacer(14, xPlayerLayout::SmallSpace);
    layout->addWidget(musicLibraryExistingButton, 15, 0, 1, 2);
    layout->addWidget(musicLibraryMarksButton, 16, 0, 1, 2);
    layout->addWidget(musicLibraryExistingWidget, 15, 2, 3, 6);
    layout->addWidget(musicLibrarySortBySize, 15, 8, 1, 2);
    layout->addWidget(musicLibraryCompareButton, 17, 8, 1, 2);
    layout->addColumnSpacer(10, xPlayerLayout::SmallSpace);
    // Action layout.
    layout->addLayout(actionLayout, 0, 11, 14, 8);
    layout->addWidget(actionStorageBar, 15, 11, 1, 8);
    layout->addWidget(actionBarLabel, 16, 11, 1, 2);
    layout->addWidget(actionBar, 16, 13, 1, 6);
    layout->addWidget(actionApplyButton, 17, 17, 1, 2);
    layout->addColumnSpacer(19, xPlayerLayout::SmallSpace);
    // Mobile library layout
    layout->addWidget(mobileLibraryWidget, 0, 20, 13, 10);
    layout->addWidget(mobileLibraryFilter, 13, 20, 1, 10);
    layout->addWidget(mobileLibraryStorageBar, 15, 20, 1, 10);
    layout->addWidget(mobileLibraryIOReadLabel, 16, 20, 1, 2);
    layout->addWidget(mobileLibraryIOReadBar, 16, 22, 1, 3);
    layout->addWidget(mobileLibraryIOWriteBar, 16, 25, 1, 3);
    layout->addWidget(mobileLibraryIOWriteLabel, 16, 28, 1, 2);
    layout->addWidget(mobileLibraryDirectoryButton, 17, 20, 1, 2);
    layout->addWidget(mobileLibraryDirectoryWidget, 17, 22, 1, 6);
    layout->addWidget(mobileLibraryScanClearButton, 17, 28, 1, 2);
    this->setLayout(layout);
    // Connect signals.
    connect(this, &xMainMobileSyncWidget::mobileLibraryIO, this, &xMainMobileSyncWidget::mobileLibraryUpdateIO);
    connect(mobileLibraryDirectoryButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::mobileLibraryOpenDirectory);
    connect(mobileLibraryScanClearButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::mobileLibraryScanClear);
    connect(mobileLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlClicked, this, &xMainMobileSyncWidget::musicLibraryFindItem);
    connect(mobileLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlRightClicked, this, &xMainMobileSyncWidget::actionRemoveFrom);
    connect(mobileLibraryWidget, &xPlayerMusicLibraryWidget::treeReady, this, &xMainMobileSyncWidget::mobileLibraryReady);
    connect(mobileLibraryFilterWidget, &QLineEdit::textChanged, mobileLibraryWidget, &xPlayerMusicLibraryWidget::filterArtistItems);
    connect(musicLibraryCompareButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::musicLibraryCompare);
    connect(musicLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlClicked, this, &xMainMobileSyncWidget::mobileLibraryFindItem);
    connect(musicLibraryWidget, &xPlayerMusicLibraryWidget::treeItemCtrlRightClicked, this, &xMainMobileSyncWidget::actionAddTo);
    connect(musicLibraryWidget, &xPlayerMusicLibraryWidget::treeReady, this, &xMainMobileSyncWidget::musicLibraryReady);
    connect(musicLibraryFilterWidget, &QLineEdit::textChanged, musicLibraryWidget, &xPlayerMusicLibraryWidget::filterArtistItems);
    connect(musicLibrarySortBySize, &QCheckBox::clicked, musicLibraryWidget, &xPlayerMusicLibraryWidget::setSortBySize);
    connect(actionAddToWidget, &QListWidget::customContextMenuRequested, this, &xMainMobileSyncWidget::actionAddToDelete);
    connect(actionRemoveFromWidget, &QListWidget::customContextMenuRequested, this, &xMainMobileSyncWidget::actionRemoveFromDelete);
    connect(actionApplyButton, &QPushButton::pressed, this, &xMainMobileSyncWidget::actionApply);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedUseMusicLibraryBluOS,
            this, &xMainMobileSyncWidget::useMusicLibraryBluOS);
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
    // Clear artist item filter on Scan and Clear.
    mobileLibraryFilterWidget->clear();
    mobileLibraryWidget->filterArtistItems(QString());
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
            if (removeFromItem->trackEntry()) {
                std::filesystem::remove(removeFromItem->trackEntry()->getUrl().toLocalFile().toStdString());
            } else {
                std::filesystem::remove_all(removeFromItem->entryUrl().toLocalFile().toStdString());
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
            destinationPath = mobileLibrary->getUrl().toLocalFile().toStdString();
            if (addToItem->trackEntry()) {
                sourcePath = addToItem->trackEntry()->getUrl().toLocalFile().toStdString();
                destinationPath = destinationPath /
                                  addToItem->artist()->entryName().toStdString() /
                                  addToItem->album()->entryName().toStdString();
            } else {
                sourcePath = addToItem->entryUrl().toLocalFile().toStdString();
                if (addToItem->artist()) {
                    destinationPath = destinationPath /
                                      addToItem->artist()->entryName().toStdString() /
                                      addToItem->entryName().toStdString();
                } else {
                    destinationPath = destinationPath /
                                      addToItem->entryName().toStdString();
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
    // Sleep a few ms before finishing thread. Give emitted signals time.
    QThread::msleep(250);
}

void xMainMobileSyncWidget::actionApplyUpdate(int progress) {
    actionBar->setValue(progress);
    // Determine available space and capacity.
    auto currentSpaceInfo = std::filesystem::space(mobileLibrary->getUrl().toLocalFile().toStdString());
    actionStorageBar->setRange(0, static_cast<int>(currentSpaceInfo.capacity / 1048576));
    actionStorageBar->setValue(static_cast<int>((currentSpaceInfo.capacity-currentSpaceInfo.available) / 1048576));
    // Set the progress bar text.
    actionStorageBar->setFormat(storageBarFormat(currentSpaceInfo.available, currentSpaceInfo.capacity));
}

void xMainMobileSyncWidget::actionApplyFinished() {
    qDebug() << "Sync of mobile library finished...";
    // Rescan the library.
    mobileLibraryScanClear();
    // Hide action bar.
    actionBar->setVisible(false);
    actionBarLabel->setVisible(false);
    // Enable widgets.
    musicLibraryWidget->setEnabled(true);
    musicLibraryMarksButton->setEnabled(true);
    musicLibraryExistingButton->setEnabled(true);
    musicLibraryCompareButton->setEnabled(true);
    musicLibraryExistingWidget->setEnabled(true);
    musicLibrarySortBySize->setEnabled(true);
    musicLibraryFilter->setEnabled(true);
    mobileLibraryWidget->setEnabled(true);
    mobileLibraryScanClearButton->setEnabled(true);
    mobileLibraryDirectoryButton->setEnabled(true);
    mobileLibraryDirectoryWidget->setEnabled(true);
    mobileLibraryFilter->setEnabled(true);
    actionAddToWidget->setEnabled(true);
    actionAddToGroupBox->setEnabled(true);
    actionRemoveFromWidget->setEnabled(true);
    actionRemoveFromGroupBox->setEnabled(true);
    disconnect(this, &xMainMobileSyncWidget::actionApplyProgress, this, &xMainMobileSyncWidget::actionApplyUpdate);
    actionApplyButton->setText("Apply");
    // Allow music library scanning again.
    emit enableMusicLibraryScanning(true);
}

xMainMobileSyncWidget::~xMainMobileSyncWidget() {
   if (mobileLibraryIOThread) {
       qDebug() << "Waiting for IO thread to finish.";
       mobileLibraryIOThread->requestInterruption();
       mobileLibraryIOThread->wait();
   }
}

void xMainMobileSyncWidget::actionApply() {
    // Check if thread is already running.
    if ((actionThread) && (actionThread->isRunning())) {
        actionThread->requestInterruption();
        actionThread->wait();
        // Enable widgets.
        musicLibraryWidget->setEnabled(true);
        musicLibraryFilter->setEnabled(true);
        mobileLibraryWidget->setEnabled(true);
        mobileLibraryFilter->setEnabled(true);
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
    musicLibraryMarksButton->setEnabled(false);
    musicLibraryExistingButton->setEnabled(false);
    musicLibraryCompareButton->setEnabled(false);
    musicLibraryExistingWidget->setEnabled(false);
    musicLibrarySortBySize->setEnabled(false);
    musicLibraryFilter->setEnabled(false);
    mobileLibraryWidget->setEnabled(false);
    mobileLibraryScanClearButton->setEnabled(false);
    mobileLibraryDirectoryButton->setEnabled(false);
    mobileLibraryDirectoryWidget->setEnabled(false);
    mobileLibraryFilter->setEnabled(false);
    actionAddToWidget->setEnabled(false);
    actionAddToGroupBox->setEnabled(false);
    actionRemoveFromWidget->setEnabled(false);
    actionRemoveFromGroupBox->setEnabled(false);
    // Prepare action bar and make it visible.
    actionBar->setVisible(true);
    actionBar->setRange(0, static_cast<int>(actionRemoveFromItems.size()+actionAddToExpandedItems.size()));
    actionBarLabel->setVisible(true);

    connect(this, &xMainMobileSyncWidget::actionApplyProgress, this, &xMainMobileSyncWidget::actionApplyUpdate);
    actionThread = QThread::create([=]() { actionApplyThread(actionAddToExpandedItems); });
    connect(actionThread, &QThread::finished, this, &xMainMobileSyncWidget::actionApplyFinished);
    // Disable music library scanning.
    emit enableMusicLibraryScanning(false);
    actionThread->start();
    // Update apply to cancel button.
    actionApplyButton->setText(tr("Cancel"));
}

void xMainMobileSyncWidget::useMusicLibraryBluOS() {
    if (xPlayerConfiguration::configuration()->useMusicLibraryBluOS()) {
        clear();
        setEnabled(false);
    } else {
        setEnabled(true);
    }
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
        // Disable UI elements.
        mobileLibraryScanClearButton->setEnabled(false);
        mobileLibraryDirectoryButton->setEnabled(false);
        mobileLibraryDirectoryWidget->setEnabled(false);

        auto mobileLibraryPath = std::filesystem::path(mobileLibraryDirectory.toStdString());
        mobileLibraryWidget->setUrl(QUrl::fromLocalFile(mobileLibraryDirectory));
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
            // Stop old IO thread and start new IO Thread.
            if (mobileLibraryIOThread) {
                mobileLibraryIOThread->requestInterruption();
                mobileLibraryIOThread->wait();
            }
            mobileLibraryIOThread = QThread::create([=]() {
                mobileLibraryReadIOThread(QString::fromStdString(mobileLibraryPath));
            });
            mobileLibraryIOThread->start();
        } catch (const std::filesystem::filesystem_error& error) {
            qCritical() << "Unable to determine capacity for mobile library.";
        }
        // Remove map of existing files for mobile library on scan.
        musicLibraryExisting.erase(mobileLibraryPath);
        updateExistingList();
    } else {
        if (mobileLibraryIOThread) {
            mobileLibraryIOThread->requestInterruption();
            mobileLibraryIOThread->wait();
            mobileLibraryIOThread = nullptr;
        }
        mobileLibraryWidget->clear();
        // Clear storage bars.
        mobileLibrarySpaceInfo = std::filesystem::space_info{};
        mobileLibraryStorageBar->setRange(0, 1);
        mobileLibraryStorageBar->setValue(0);
        mobileLibraryStorageBar->setFormat("");
        // Clear IO bars.
        mobileLibraryIOReadBar->setRange(0, 1);
        mobileLibraryIOReadBar->setValue(0);
        mobileLibraryIOReadBar->setFormat("");
        mobileLibraryIOWriteBar->setRange(0, 1);
        mobileLibraryIOWriteBar->setValue(0);
        mobileLibraryIOWriteBar->setFormat("");
        // Clear action bar.
        actionStorageBar->setRange(0, 1);
        actionStorageBar->setValue(0);
        actionStorageBar->setFormat("");
    }
}

void xMainMobileSyncWidget::mobileLibraryFindItem(xPlayerMusicLibraryWidgetItem* item) {
    if (item->trackEntry()) {
        mobileLibraryWidget->selectItem(item->trackEntry()->getArtistName(), item->trackEntry()->getAlbumName());
    } else {
        if (item->artist()) {
            mobileLibraryWidget->selectItem(item->artist()->entryName(), item->entryName());
        } else {
            mobileLibraryWidget->selectItem(item->entryName());
        }
    }
}

void xMainMobileSyncWidget::mobileLibrarySaveToExisting() {
    std::map<QString, std::map<QString, std::list<xMusicLibraryTrackEntry*>>> equalTracks;
    // Determine equal tracks. The map contains music file pointer from the music library.
    musicLibrary->compare(mobileLibrary, equalTracks);
    musicLibraryExisting[mobileLibrary->getUrl().toLocalFile().toStdString()] = equalTracks;
    // Update list of existing mobile libraries.
    updateExistingList();
}

void xMainMobileSyncWidget::mobileLibraryRemoveSaved() {
    auto selectedExisting = musicLibraryExistingWidget->selectedItems();
    if (!selectedExisting.isEmpty()) {
        // Remove selected entries.
        for (auto selected : selectedExisting) {
            musicLibraryExisting.erase(std::filesystem::path(selected->text().toStdString()));
        }
        // Update list of existing mobile libraries.
        updateExistingList();
    }
}

void xMainMobileSyncWidget::mobileLibraryRemoveAllSaved() {
    musicLibraryExisting.clear();
    // Update list of existing mobile libraries.
    updateExistingList();
}

void xMainMobileSyncWidget::mobileLibraryReady() {
    mobileLibraryScanClearButton->setEnabled(true);
    mobileLibraryDirectoryButton->setEnabled(true);
    mobileLibraryDirectoryWidget->setEnabled(true);
}

void xMainMobileSyncWidget::mobileLibraryUpdateIO(quint64 readBytes, quint64 writeBytes) {
    int readKiloBytes = static_cast<int>(readBytes/1024);
    int writeKiloBytes = static_cast<int>(writeBytes/1024);
    if (mobileLibraryIOReadBar->maximum() < readKiloBytes) {
        mobileLibraryIOReadBar->setRange(0, readKiloBytes);
    }
    if (mobileLibraryIOWriteBar->maximum() < writeKiloBytes) {
        mobileLibraryIOWriteBar->setRange(0, writeKiloBytes);
    }
    mobileLibraryIOReadBar->setValue(readKiloBytes);
    mobileLibraryIOReadBar->setFormat(QString("%1 MB/sec").arg(readKiloBytes/1024.0, -1, 'f', 2));
    mobileLibraryIOWriteBar->setValue(writeKiloBytes);
    mobileLibraryIOWriteBar->setFormat(QString("%1 MB/sec").arg(writeKiloBytes/1024.0, -1, 'f', 2));
}

void xMainMobileSyncWidget::mobileLibraryReadIOThread(const QString &mobileLibraryPath) {
    // Determine device from path.
    struct stat pathStat{};
    if (stat(mobileLibraryPath.toStdString().c_str(), &pathStat) < 0) {
        qCritical() << "Unable to call stat for path: " << mobileLibraryPath;
        return;
    }
    auto deviceMajor = major(pathStat.st_dev);
    auto deviceMinor = minor(pathStat.st_dev);
    qDebug() << "MAJOR: " << deviceMajor << ", MINOR: " << deviceMinor;
    // Determine the sector size
    unsigned long deviceSectorSize = 512;
    QFile deviceSectorSizeFile(pathToDeviceSectorSize.arg(deviceMajor).arg(deviceMinor));
    if (deviceSectorSizeFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream deviceSectorSizeStream(&deviceSectorSizeFile);
        deviceSectorSizeFile.close();
    } else {
        qWarning() << "Unable to read sector size for device. Defaulting to 512 Bytes";
    }
    // Open the device stat file.
    QFile deviceStatFile(pathToDeviceStat.arg(deviceMajor).arg(deviceMinor));
    if (!deviceStatFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qCritical() << "Unable to open device stat file: " << deviceStatFile.fileName();
        return;
    }
    // Initialize sectors.
    unsigned long currentSectorsRead = 0;
    unsigned long currentSectorsWrite = 0;
    unsigned long sectorsRead = 0;
    unsigned long sectorsWrite = 0;
    // Read sectors read/write initial values.
    QTextStream deviceStatStream(&deviceStatFile);
    QStringList deviceStat = deviceStatStream.readLine().split(' ', Qt::SkipEmptyParts);
    if (deviceStat.size() > 6) {
        currentSectorsRead = deviceStat[2].toULong();
        currentSectorsWrite = deviceStat[6].toULong();
    }
    while (!deviceStatStream.atEnd()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        // Reset stat file stream.
        deviceStatStream.seek(0);
        deviceStat = deviceStatStream.readLine().split(' ', Qt::SkipEmptyParts);
        if (deviceStat.size() > 6) {
            sectorsRead = deviceStat[2].toULong();
            sectorsWrite = deviceStat[6].toULong();
        }
        auto bytesRead = (sectorsRead - currentSectorsRead) * deviceSectorSize;
        auto bytesWrite = (sectorsWrite - currentSectorsWrite) * deviceSectorSize;
        // Emit IO data.
        emit mobileLibraryIO(bytesRead, bytesWrite);
        currentSectorsRead = sectorsRead;
        currentSectorsWrite = sectorsWrite;
        // Read every second.
        QThread::sleep(1);
    }
    // Close file.
    deviceStatFile.close();

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
    QStringList missingArtists, additionalArtists;
    std::map<QString, QStringList> missingAlbums, additionalAlbums;
    std::list<xMusicLibraryTrackEntry*> missingTracks, additionalTracks;
    std::pair<std::list<xMusicLibraryTrackEntry*>, std::list<xMusicLibraryTrackEntry*>> differentTracks;

    musicLibrary->compare(mobileLibrary, missingArtists, additionalArtists,
                          missingAlbums, additionalAlbums,
                          missingTracks, additionalTracks, differentTracks);
    // Mark music and mobile library.
    musicLibraryWidget->markItems(missingArtists, missingAlbums, missingTracks, differentTracks.first);
    mobileLibraryWidget->markItems(additionalArtists, additionalAlbums, additionalTracks, differentTracks.second);
}

void xMainMobileSyncWidget::musicLibraryFindItem(xPlayerMusicLibraryWidgetItem* item) {
    if (item->trackEntry()) {
        musicLibraryWidget->selectItem(item->trackEntry()->getArtistName(), item->trackEntry()->getAlbumName());
    } else {
        if (item->artist()) {
            musicLibraryWidget->selectItem(item->artist()->entryName(), item->entryName());
        } else {
            musicLibraryWidget->selectItem(item->entryName());
        }
    }
}

void xMainMobileSyncWidget::musicLibraryMarkExisting() {
    // Mark the items.
    for (auto& entry : musicLibraryExisting) {
        musicLibraryWidget->markExistingItems(entry.second);
    }
}

void xMainMobileSyncWidget::musicLibraryReady() {
    musicLibraryCompareButton->setEnabled(true);
    musicLibraryMarksButton->setEnabled(true);
    musicLibraryExistingButton->setEnabled(true);
    musicLibraryExistingWidget->setEnabled(true);
    musicLibrarySortBySize->setEnabled(true);
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
    if (actionTotalSize > mobileLibrarySpaceInfo.capacity) {
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
    // Enable Apply button if we have any entries, disable otherwise.
    actionApplyButton->setEnabled((actionAddToItems.size()+actionRemoveFromItems.size()) > 0);
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
