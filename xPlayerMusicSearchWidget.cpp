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

#include "xPlayerMusicSearchWidget.h"
#include "xPlayerUI.h"

#include <QApplication>
#include <QRadioButton>

xPlayerMusicSearchWidget::xPlayerMusicSearchWidget(QWidget* parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        currentSearchText(),
        currentSearchCategory(-1) {
    auto layout = new xPlayerLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    searchArtistButton = new QRadioButton(tr("Artist"), this);
    searchArtistButton->setChecked(true);
    searchAlbumButton = new QRadioButton(tr("Album"), this);
    searchTrackButton = new QRadioButton(tr("Track"), this);
    searchCategoryGroup = new QButtonGroup(this);
    searchCategoryGroup->addButton(searchArtistButton, 1);
    searchCategoryGroup->addButton(searchAlbumButton, 2);
    searchCategoryGroup->addButton(searchTrackButton, 3);
    searchCategoryGroup->setExclusive(true);
    searchButton = new QPushButton(tr("Search"), this);
    clearButton = new QPushButton(tr("Clear"), this);
    searchText = new QLineEdit(this);
    layout->addWidget(searchText, 0, 0, 1, 6);
    layout->addColumnSpacer(6, xPlayerLayout::HugeSpace);
    layout->addWidget(searchArtistButton, 0, 7);
    layout->addWidget(searchAlbumButton, 0, 8);
    layout->addWidget(searchTrackButton, 0, 9);
    layout->addColumnSpacer(10, xPlayerLayout::HugeSpace);
    layout->addWidget(searchButton, 0, 11);
    layout->addColumnSpacer(12, xPlayerLayout::SmallSpace);
    layout->addWidget(clearButton, 0, 13);
    layout->addColumnSpacer(14, xPlayerLayout::SmallSpace);
    setLayout(layout);
    setFixedHeight(static_cast<int>(QFontMetrics(QApplication::font()).height()*xPlayer::SelectorHeightFontFactor));
    connect(searchButton, &QPushButton::pressed, this, &xPlayerMusicSearchWidget::searchClicked);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerMusicSearchWidget::clear);
    connect(searchText, &QLineEdit::returnPressed, this, &xPlayerMusicSearchWidget::returnPressed);
}

std::tuple<QString,QString,QString> xPlayerMusicSearchWidget::getMatch() const {
    switch (currentSearchCategory) {
        case 1: return std::make_tuple(currentSearchText, QString(), QString());
        case 2: return std::make_tuple(QString(), currentSearchText, QString());
        case 3: return std::make_tuple(QString(), QString(), currentSearchText);
        default: return std::make_tuple(QString(), QString(), QString());
    }
}

void xPlayerMusicSearchWidget::clear() {
    searchText->clear();
    searchArtistButton->setChecked(true);
    currentSearchCategory = -1;
    currentSearchText.clear();
    emit clearFilter();
}

void xPlayerMusicSearchWidget::searchClicked() {
    currentSearchText = searchText->text();
    currentSearchCategory = searchCategoryGroup->checkedId();
    if (currentSearchCategory != -1) {
        emit updateFilter(getMatch());
    } else {
        clear();
    }
}

void xPlayerMusicSearchWidget::returnPressed() {
    currentSearchText = searchText->text();
    currentSearchCategory = searchCategoryGroup->checkedId();
    if (currentSearchText.isEmpty()) {
        emit clearFilter();
    } else {
        searchClicked();
    }
}
