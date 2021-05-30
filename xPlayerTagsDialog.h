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

#ifndef __XPLAYERTAGSDIALOG_H__
#define __XPLAYERTAGSDIALOG_H__

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QListWidget>

class xPlayerTagsDialog:public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor. Setup the dialog for selecting the playlist.
     *
     * @param parent the pointer to the parent widget.
     * @param flags the window flags for the widget.
     */
    explicit xPlayerTagsDialog(const QStringList& tags, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    /**
     * Destructor.
     */
    ~xPlayerTagsDialog() override = default;

signals:
    /**
     * Signal to load the queue with tagged songs from the database.
     *
     * @param tag the tag used to identify the tagged songs.
     * @param extend extend the current queue if true, replace otherwise
     */
    void loadFromTag(const QString& tag, bool extend);
};

#endif
