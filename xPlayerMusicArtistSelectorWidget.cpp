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

#include "xPlayerMusicArtistSelectorWidget.h"
#include "xPlayerUI.h"

#include <QApplication>

xPlayerMusicArtistSelectorWidget::xPlayerMusicArtistSelectorWidget(QWidget* parent, Qt::WindowFlags flags):
        QWidget(parent, flags) {
    setContentsMargins(0, 0, 0, 0);
    auto layout = new xPlayerLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    selectorList = new QListWidget(this);
    selectorList->setViewMode(QListView::IconMode);
    selectorList->setWrapping(false);
    sortingLatestBox = new QCheckBox(tr("Sorting Latest"), this);
    layout->addWidget(sortingLatestBox, 0, 11);
    layout->addColumnSpacer(12, xPlayerLayout::SmallSpace);
    layout->addWidget(selectorList, 0, 0, 1, 10);
    layout->addColumnSpacer(10, xPlayerLayout::MediumSpace);
    setLayout(layout);
    setFixedHeight(static_cast<int>(QFontMetrics(QApplication::font()).height()*xPlayerSelectorHeightFontFactor));

    connect(selectorList, &QListWidget::currentRowChanged, this, &xPlayerMusicArtistSelectorWidget::selectArtistSelector);
    connect(selectorList, &QListWidget::itemDoubleClicked, this, &xPlayerMusicArtistSelectorWidget::doubleClickedArtistSelector);
    connect(sortingLatestBox, &QCheckBox::clicked, this, &xPlayerMusicArtistSelectorWidget::sortingLatest);
}

void xPlayerMusicArtistSelectorWidget::updateSelectors(const std::set<QString>& selectors) {
    // Update artist selectors list widget.
    selectorList->clear();
    selectorList->addItem(tr("none"));
    selectorList->addItem(tr("random"));
    for (const auto& as : selectors) {
        selectorList->addItem(as);
    }
}

void xPlayerMusicArtistSelectorWidget::clear() {
    selectorList->clear();
}

void xPlayerMusicArtistSelectorWidget::selectArtistSelector(int selectorIndex) {
    // Check if index is valid.
    if ((selectorIndex >= 0) && (selectorIndex < selectorList->count())) {
        // Call with stored list in order to update artist filtering.
        emit selector(selectorList->currentItem()->text());
    }
}

void xPlayerMusicArtistSelectorWidget::doubleClickedArtistSelector(QListWidgetItem* selectorItem) {
    // Currently unused
    auto selectorIndex = selectorList->row(selectorItem);
    if ((selectorIndex >= 0) && (selectorIndex < selectorList->count())) {
        // Do not queue artist selector if random is selected, just randomize again.
        if (selectorItem->text().compare("random", Qt::CaseInsensitive) == 0) {
            // Disable sorting mode on random.
            sortingLatestBox->setChecked(false);
            emit selector("random");
            return;
        }
        // Other selectors will be queued.
        emit selectorDoubleClicked(selectorList->currentItem()->text());
    }
}
