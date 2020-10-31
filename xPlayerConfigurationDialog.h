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
    void loadSettings();

private slots:
    void saveSettings();

    void openMusicLibraryDirectory();
    void openMovieLibraryDirectory();

    void selectMovieLibrary(QListWidgetItem* item);
    void movieLibraryAdd();
    void movieLibraryRemove();

private:
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
