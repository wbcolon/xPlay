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

#include "xPlayerPlaylistDialog.h"
#include "xPlayerUI.h"

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

xPlayerPlaylistDialog::xPlayerPlaylistDialog(const QStringList& playlistNames, QWidget* parent, Qt::WindowFlags flags):
        QDialog(parent, flags) {
    auto playlistLayout = new xPlayerLayout();
    playlistList = new QListWidget(this);
    playlistInput = new QLineEdit(this);
    auto playlistListLabel = new QLabel(tr("Playlists"), this);
    playlistListLabel->setAlignment(Qt::AlignLeft);
    auto playlistInputLabel = new QLabel(tr("Name"), this);
    playlistInputLabel->setAlignment(Qt::AlignLeft);
    playlistButtons = new QDialogButtonBox(this);
    playlistButtons->addButton(QDialogButtonBox::Open);
    playlistButtons->addButton(QDialogButtonBox::Save);
    playlistButtons->addButton(QDialogButtonBox::Discard);
    playlistButtons->addButton(QDialogButtonBox::Cancel);
    playlistLayout->addWidget(playlistListLabel, 0, 0, 1, 4);
    playlistLayout->addWidget(playlistList, 1, 0, 3, 4);
    playlistLayout->addRowSpacer(4, xPlayerLayout::LargeSpace);
    playlistLayout->addWidget(playlistInputLabel, 5, 0, 1, 4);
    playlistLayout->addWidget(playlistInput, 6, 0, 1, 4);
    playlistLayout->addRowSpacer(7, xPlayerLayout::LargeSpace);
    playlistLayout->addWidget(playlistButtons, 8, 0, 1, 4);
    setLayout(playlistLayout);
    playlistList->addItems(playlistNames);
    // Disable open and save button until line edit is not empty.
    playlistButtons->button(QDialogButtonBox::Open)->setText(tr("Load"));
    playlistButtons->button(QDialogButtonBox::Open)->setEnabled(false);
    playlistButtons->button(QDialogButtonBox::Save)->setEnabled(false);
    playlistButtons->button(QDialogButtonBox::Discard)->setText(tr("Remove"));
    playlistButtons->button(QDialogButtonBox::Discard)->setEnabled(false);
    // Connect list widget and line edit signals. Buttons are enabled based on their values.
    connect(playlistList, &QListWidget::currentRowChanged, this, &xPlayerPlaylistDialog::playlistSelected);
    connect(playlistInput, &QLineEdit::textChanged, this, &xPlayerPlaylistDialog::playlistInputChanged);
    // Connect buttons to playlist actions and close afterwards.
    connect(playlistButtons->button(QDialogButtonBox::Open), &QPushButton::pressed,
            [=]() { emit openPlaylist(playlistInput->text()); this->accept(); });
    connect(playlistButtons->button(QDialogButtonBox::Save), &QPushButton::pressed,
            [=]() { emit savePlaylist(playlistInput->text()); this->accept(); });
    connect(playlistButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed,
            this, &xPlayerPlaylistDialog::removePlaylistItem);
    connect(playlistButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &xPlayerPlaylistDialog::close);
    // Set the minimum width as 3/2 of its height.
    setMinimumWidth(sizeHint().height()*3/2);
}

void xPlayerPlaylistDialog::playlistSelected(int index) {
    if ((index >= 0) && (index < playlistList->count())) {
        // Update the line edit with the selected playlist name.
        playlistInput->setText(playlistList->item(index)->text());
    }
}

void xPlayerPlaylistDialog::playlistInputChanged(const QString& text) {
    // Make sure that the buttons are properly enabled/disabled
    // The "Save" button is enabled if the input is non-empty.
    playlistButtons->button(QDialogButtonBox::Save)->setDisabled(text.isEmpty());
    if (!text.isEmpty()) {
        // The "Load" button is only enabled if the text matches an entry in the list.
        for (auto i = 0; i < playlistList->count(); ++i) {
           if (text == playlistList->item(i)->text()) {
               playlistButtons->button(QDialogButtonBox::Open)->setEnabled(true);
               playlistButtons->button(QDialogButtonBox::Discard)->setEnabled(true);
               return;
           }
        }
        playlistButtons->button(QDialogButtonBox::Open)->setEnabled(false);
        playlistButtons->button(QDialogButtonBox::Discard)->setEnabled(false);
    } else {
        playlistButtons->button(QDialogButtonBox::Open)->setEnabled(false);
    }
}

void xPlayerPlaylistDialog::removePlaylistItem() {
    auto name = playlistInput->text();
    if (!name.isEmpty()) {
        // The "Load" button is only enabled if the text matches an entry in the list.
        for (auto i = 0; i < playlistList->count(); ++i) {
            if (name == playlistList->item(i)->text()) {
                playlistList->takeItem(i);
                emit removePlaylist(name);
                break;
            }
        }
    }
}