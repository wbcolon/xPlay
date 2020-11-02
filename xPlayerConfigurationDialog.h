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
#include <QSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QSettings>

class xPlayerConfigurationDialog:public QDialog {
    Q_OBJECT

public:
    xPlayerConfigurationDialog(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerConfigurationDialog() = default;

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

private:
    /**
     * Split an entry from the movie library list into tag and directory.
     *
     * @param entry the string containing tag and directory.
     * @return a pair of tag and directory as string.
     */
    std::pair<QString,QString> splitMovieLibraryEntry(const QString& entry);

    QLineEdit* musicLibraryDirectoryWidget;
    QLineEdit* musicLibraryExtensionsWidget;
    QLineEdit* movieLibraryTagWidget;
    QLineEdit* movieLibraryDirectoryWidget;
    QLineEdit* movieLibraryExtensionsWidget;
    QListWidget* movieLibraryListWidget;
    QLineEdit* rotelNetworkAddressWidget;
    QSpinBox* rotelNetworkPortWidget;
};

#endif
