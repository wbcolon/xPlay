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

#ifndef __XPLAYERPLAYLISTDIALOG_H__
#define __XPLAYERPLAYLISTDIALOG_H__

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QListWidget>

class xPlayerPlaylistDialog:public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor. Setup the dialog for selecting the playlist.
     *
     * @param playlistNames the list of playlist names in the database.
     * @param parent the pointer to the parent widget.
     * @param flags the window flags for the widget.
     */
    explicit xPlayerPlaylistDialog(const QStringList& playlistNames, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    /**
     * Destructor.
     */
    ~xPlayerPlaylistDialog() override = default;

signals:
    /**
     * Signal to load a playlist from the database.
     *
     * @param name the name of the playlist in the database.
     */
    void openPlaylist(const QString& name);
    /**
     * Signal to same the current queue as playlist to the database.
     *
     * @param name the name for the (new) playlist in the database.
     */
    void savePlaylist(const QString& name);
    /**
     * Signal to same the current queue as playlist to the database.
     *
     * @param name the name for the (new) playlist in the database.
     */
    void removePlaylist(const QString& name);

private slots:
    /**
     * Called upon the selecting a name from the playlist.
     *
     * @param index the index of the playlist as integer.
     */
    void playlistSelected(int index);
    /**
     * Called upon a change of the input for the playlist name.
     *
     * @param text the current text of the input.
     */
    void playlistInputChanged(const QString& text);
    /**
     * Called if remove button is called.
     */
    void removePlaylistItem();

private:
    QListWidget* playlistList;
    QLineEdit* playlistInput;
    QDialogButtonBox* playlistButtons;
};

#endif
