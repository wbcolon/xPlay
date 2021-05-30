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

#include "xPlayerTagsDialog.h"
#include "xPlayerUI.h"

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

xPlayerTagsDialog::xPlayerTagsDialog(const QStringList& tags, QWidget* parent, Qt::WindowFlags flags):
        QDialog(parent, flags) {
    auto tagsLayout = new xPlayerLayout();
    auto tagsList = new QListWidget(this);
    auto tagsListLabel = new QLabel(tr("Tags"), this);
    tagsListLabel->setAlignment(Qt::AlignLeft);
    auto tagsButtons = new QDialogButtonBox(this);
    tagsButtons->addButton(QDialogButtonBox::Open);
    tagsButtons->addButton(QDialogButtonBox::Apply);
    tagsButtons->addButton(QDialogButtonBox::Cancel);
    tagsLayout->addWidget(tagsListLabel, 0, 0, 1, 4);
    tagsLayout->addWidget(tagsList, 1, 0, 3, 4);
    tagsLayout->addRowSpacer(4, xPlayerLayout::LargeSpace);
    tagsLayout->addWidget(tagsButtons, 5, 0, 1, 4);
    setLayout(tagsLayout);
    tagsList->addItems(tags);
    tagsList->setCurrentRow(0);
    // Disable open and save button until line edit is not empty.
    tagsButtons->button(QDialogButtonBox::Open)->setText(tr("Load"));
    tagsButtons->button(QDialogButtonBox::Apply)->setText(tr("Extend"));
    // Connect buttons to tags actions and close afterwards.
    connect(tagsButtons->button(QDialogButtonBox::Open), &QPushButton::pressed,
            [=]() { emit loadFromTag(tagsList->currentItem()->text(), false); this->accept(); });
    connect(tagsButtons->button(QDialogButtonBox::Apply), &QPushButton::pressed,
            [=]() { emit loadFromTag(tagsList->currentItem()->text(), true); this->accept(); });
    connect(tagsButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &xPlayerTagsDialog::close);
    // Set the minimum width as 3/2 of its height.
    setMinimumWidth(sizeHint().height()*3/2); // NOLINT
}