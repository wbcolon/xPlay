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
#include <QCheckBox>
#include <QProgressBar>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>

#include <filesystem>


class xMainMobileSyncWidget:public QWidget {
    Q_OBJECT

public:
    explicit xMainMobileSyncWidget(xMusicLibrary* library, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~xMainMobileSyncWidget() override;
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
    /**
     * Internal signal used to update the mobile sync progress bar.
     *
     * @param progress the progress in precent as integer.
     */
    void actionApplyProgress(int progress);
    /**
     * Signal emitted whenever we enable/disable the music library scanning.
     *
     * @param enabled scanning of music library allowed if true, not allowed otherwise.
     */
    void enableMusicLibraryScanning(bool enabled);
    /**
     * Signal emitted with read/write IO for mobile library.
     *
     * @param readBytes no of bytes per second read.
     * @param writeBytes no of bytes per second written.
     */
    void mobileLibraryIO(quint64 readBytes, quint64 writeBytes);

private slots:
    /**
     * Disable mobile sync widget if BluOS player library is used.
     */
    void useMusicLibraryBluOS();
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
     * Save the existing mobile library entries for later marking.
     */
    void mobileLibrarySaveToExisting();
    /**
     * Remove selected saved mobile library.
     */
    void mobileLibraryRemoveSaved();
    /**
     * Remove all saved mobile libraries.
     */
    void mobileLibraryRemoveAllSaved();
    /**
     * Update mobile library view after scan is finished.
     */
    void mobileLibraryReady();
    /**
     * Read IO stats for the mobile library access.
     */
    void mobileLibraryReadIOThread(const QString& mobileLibraryPath);
    /**
     * Update progess bars with read and write bytes per second.
     *
     * @param readBytes no of bytes per second read.
     * @param writeBytes no of bytes per second written.
     */
    void mobileLibraryUpdateIO(quint64 readBytes, quint64 writeBytes);
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
    /**
     * Mark all existing items in the music library.
     */
    void musicLibraryMarkExisting();
    /**
     * Update music library view after scan is finished.
     */
    void musicLibraryReady();
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
     * Perform rescan after the action is finished.
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
    QLineEdit* musicLibraryFilterWidget;
    QGroupBox* musicLibraryFilter;
    QPushButton* musicLibraryCompareButton;
    QCheckBox* musicLibrarySortBySize;
    QPushButton* musicLibraryMarksButton;
    QPushButton* musicLibraryExistingButton;
    QListWidget* musicLibraryExistingWidget;
    std::map<std::filesystem::path, std::map<QString, std::map<QString, std::list<xMusicLibraryTrackEntry*>>>> musicLibraryExisting;
    xMusicLibrary* mobileLibrary;
    xPlayerMusicLibraryWidget* mobileLibraryWidget;
    QGroupBox* mobileLibraryFilter;
    QLineEdit* mobileLibraryFilterWidget;
    QPushButton* mobileLibraryDirectoryButton;
    QPushButton* mobileLibraryScanClearButton;
    QLineEdit* mobileLibraryDirectoryWidget;
    QProgressBar* mobileLibraryStorageBar;
    QProgressBar* mobileLibraryIOReadBar;
    QProgressBar* mobileLibraryIOWriteBar;
    std::filesystem::space_info mobileLibrarySpaceInfo;
    QThread* mobileLibraryIOThread;
    QListWidget* actionAddToWidget;
    QGroupBox* actionAddToGroupBox;
    QListWidget* actionRemoveFromWidget;
    QGroupBox* actionRemoveFromGroupBox;
    QProgressBar* actionStorageBar;
    QLabel* actionBarLabel;
    QProgressBar* actionBar;
    QPushButton* actionApplyButton;
    std::vector<xPlayerMusicLibraryWidgetItem*> actionAddToItems;
    std::vector<xPlayerMusicLibraryWidgetItem*> actionRemoveFromItems;
    QThread* actionThread;
};

#endif
