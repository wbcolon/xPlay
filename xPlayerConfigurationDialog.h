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

#ifndef __XPLAYERCONFIGURATIONDIALOG_H__
#define __XPLAYERCONFIGURATIONDIALOG_H__

#include "xPlayerConfiguration.h"

#include <QDialog>
#include <QDateEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QSettings>

class xPlayerConfigurationDialog:public QDialog {
    Q_OBJECT

public:
    explicit xPlayerConfigurationDialog(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerConfigurationDialog() override = default;

public slots:
    /**
     * Load the current configuration settings.
     */
    void loadSettings();

private slots:
    /**
     * Save any updates into the configuration.
     */
    void saveSettings();
    /**
     * Open a dialog in order to select a music library directory.
     */
    void openMusicLibraryDirectory();
    /**
     * Open a dialog in order to select a movie library directory.
     */
    void openMovieLibraryDirectory();
    /**
     * Select a tag/directory from the movie library list.
     *
     * @param item pointer to the selected item.
     */
    void selectMovieLibrary(QListWidgetItem* item);
    /**
     * Add a tag/directory to the movie library list.
     */
    void movieLibraryAdd();
    /**
     * Remove the selected tag/directory from the movie library list.
     */
    void movieLibraryRemove();
    /**
     * Select a name/url from the streaming site list.
     *
     * @param item pointer to the selected item.
     */
    void selectStreamingSite(QListWidgetItem* item);
    /**
     * Add a name/url to the streaming site list.
     */
    void streamingSiteAdd();
    /**
     * Remove the selected name/url from the streaming site list.
     */
    void streamingSiteRemove();
    /**
     * Mark selected name/url from the streaming site list as default streaming site.
     */
    void streamingSiteDefault();
    /**
     * Toggle enable/disable for the Rotel configuration.
     */
    void toggleRotelWidget();

private:
    /**
     * Split an entry from the movie library list into tag and directory.
     *
     * @param entry the string containing tag and directory.
     * @return a pair of tag and directory as string.
     */
    static std::pair<QString,QString> splitMovieLibraryEntry(const QString& entry);
    /**
     * Split an entry from the streaming sites list into name and url.
     *
     * @param entry the string containing name and URL.
     * @return a pair of name and URL.
     */
    static std::pair<QString,QUrl> splitStreamingSiteEntry(const QString& entry);
    /**
     * Check whether the given pair is part of the given list widget.
     *
     * @param list pointer to the list widget to search through.
     * @param first the first string of the pair.
     * @param second the second string of the pair.
     * @return true if the pair of first and second is in the list widget, false otherwise.
     */
    static bool isEntryInListWidget(QListWidget* list, const QString& first, const QString& second);
    /**
     * Called upon an update to the streaming list default.
     */
    void updateStreamingSitesDefault();

    QLineEdit* musicLibraryDirectoryWidget;
    QLineEdit* musicLibraryExtensionsWidget;
    QLineEdit* movieLibraryTagWidget;
    QLineEdit* movieLibraryDirectoryWidget;
    QLineEdit* movieLibraryExtensionsWidget;
    QLineEdit* streamingNameWidget;
    QLineEdit* streamingUrlWidget;
    QListWidget* movieLibraryListWidget;
    QListWidget* streamingSitesListWidget;
    QPushButton* rotelEnableWidget;
    QLineEdit* rotelNetworkAddressWidget;
    QSpinBox* rotelNetworkPortWidget;
    QCheckBox* databaseMusicOverlayCheck;
    QCheckBox* databaseMovieOverlayCheck;
    QCheckBox* databaseCutOffCheck;
    QDateEdit* databaseCutOffDate;
    std::pair<QString,QUrl> streamingSitesDefault;
};

#endif
