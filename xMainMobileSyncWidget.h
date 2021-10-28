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

#ifndef __XMAINMOBILESYNCWIDGET_H__
#define __XMAINMOBILESYNCWIDGET_H__

#include "xPlayerMusicLibraryWidget.h"

#include <QThread>
#include <QPushButton>
#include <QProgressBar>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>


class xMainMobileSyncWidget:public QWidget {
    Q_OBJECT

public:
    explicit xMainMobileSyncWidget(xMusicLibrary* library, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~xMainMobileSyncWidget() override = default;
    /**
     * Perform initial commands required when switching to this view.
     */
    void initializeView();

public slots:
    /**
     * Clear the mobile library, actions and library markings.
     */
    void clear();

signals:
    void actionApplyProgress(int progress);

private slots:
    /**
     * Open a file dialog.
     *
     * The dialog is opened when the open directory button is pressed.
     * It allows selecting the directory for the mobile directory.
     */
    void mobileLibraryOpenDirectory();
    /**
     * Perform a scan of the mobile library.
     *
     * The scan is performed when the scan button is pressed. It is only
     * performed if a valid directory is given.
     */
    void mobileLibraryScanClear();
    /**
     * Select the corresponding item in the mobile library.
     *
     * @param item pointer to an item in the music library.
     */
    void mobileLibraryFindItem(xPlayerMusicLibraryWidgetItem* item);
    /**
     * Compare the music with the mobile directory.
     *
     * The comparison of music and mobile library is perfomed only if both
     * libraries are ready. The missing and different items for each of the
     * libraries are marked.
     */
    void musicLibraryCompare();
    /**
     * Select the corresponding item in the music library.
     *
     * @param item pointer to an item in the mobile library.
     */
    void musicLibraryFindItem(xPlayerMusicLibraryWidgetItem* item);

    void musicLibrarySaveExisting();
    /**
     * Mark all existing items in the music library.
     */
    void musicLibraryMarkExisting();
    /**
     * Clear all stored existing items.
     */
    void musicLibraryClearExisting();
    /**
     * Insert an "add to" action.
     *
     * Called if an item in the music library is ctrl+right-clicked. The
     * corresponding artist, album or track will be added to the mobile
     * library if the action is applied.
     *
     * @param item pointer to an item in the music library.
     */
    void actionAddTo(xPlayerMusicLibraryWidgetItem* item);
    /**
     * Insert an "remove from" action.
     *
     * Called if an item in the mobile library is ctrl+right-clicked. The
     * corresponding artist, album or track will be removed from the mobile
     * library if the action is applied.
     *
     * @param item pointer to an item in the music library.
     */
    void actionRemoveFrom(xPlayerMusicLibraryWidgetItem* item);
    /**
     * Delete an "add to" action.
     *
     * @param point position of an item in the "add to" action list.
     */
    void actionAddToDelete(const QPoint& point);
    /**
     * Delete an "remove from" action.
     *
     * @param point position of an item in the "remove from" action list.
     */
    void actionRemoveFromDelete(const QPoint& point);
    /**
     * Apply all "remove from" and "add to" actions.
     *
     * The function is called if the apply button is pressed. The removal
     * operation are called first, then the add actions. A simple progress
     * bar is shown and updated.
     */
    void actionApply();
    /**
     * The actual remove and copy operations which are executed in a thread.
     */
    void actionApplyThread(const std::list<xPlayerMusicLibraryWidgetItem*>& actionAddToExpandedItems);
    /**
     * Update the action progress and storage bar.
     *
     * @param progress the action progress as int.
     */
    void actionApplyUpdate(int progress);
    /**
     * Perform cleanup and rescan after the action apply thread is finished.
     */
    void actionApplyFinished();

private:
    /**
     * Update the action storage bar based on the proposed actions.
     */
    void updateActionStorage();
    /**
     * Update the list of existing mobile libraries.
     */
    void updateExistingList();
    /**
     * Create format string for storage bars.
     */
    static QString storageBarFormat(std::uintmax_t available, std::uintmax_t capacity);

    xMusicLibrary* musicLibrary;
    xPlayerMusicLibraryWidget* musicLibraryWidget;
    QListWidget* musicLibraryExistingWidget;
    std::map<std::filesystem::path, std::map<xMusicDirectory, std::map<xMusicDirectory, std::list<xMusicFile*>>>> musicLibraryExisting;
    xMusicLibrary* mobileLibrary;
    QLineEdit* mobileLibraryDirectoryWidget;
    xPlayerMusicLibraryWidget* mobileLibraryWidget;
    QProgressBar* mobileLibraryStorageBar;
    std::filesystem::space_info mobileLibrarySpaceInfo;
    QListWidget* actionAddToWidget;
    QListWidget* actionRemoveFromWidget;
    QProgressBar* actionStorageBar;
    QProgressBar* actionBar;
    QPushButton* actionApplyButton;
    std::vector<xPlayerMusicLibraryWidgetItem*> actionAddToItems;
    std::vector<xPlayerMusicLibraryWidgetItem*> actionRemoveFromItems;
    QThread* actionThread;
};

#endif
