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

#include "xPlayerMusicAlbumSelectorWidget.h"
#include "xPlayerDatabase.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"

#include <QApplication>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

const QVector<QColor> xPlayerMusicAlbumSelectorWidget_StateColors {QColor(Qt::white), QColor(Qt::green), QColor(Qt::red) }; // NOLINT

xPlayerMusicAlbumSelectorWidget::xPlayerMusicAlbumSelectorWidget(QWidget* parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        currentUserSelectorIndex(0),
        currentDatabaseSelectorIndex(-1),
        currentTimestamp(0),
        currentArtistAlbumMap() {
    // Setup the list widget.
    setContentsMargins(0, 0, 0, 0);
    auto layout = new xPlayerLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    selectorListWidget = new QListWidget(this);
    selectorListWidget->setViewMode(QListWidget::IconMode);
    selectorListWidget->setSelectionMode(QListWidget::NoSelection);
    auto timestampLabel = new QLabel(tr("Timestamp"), this);
    timestampLabel->setAlignment(Qt::AlignRight);
    timestampWidget = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    timestampWidget->setCalendarPopup(true);
    timestampWidget->setDisplayFormat("dd MMMM yyyy - hh:mm:ss");
    auto nowTimestampButton = new QPushButton(tr("Now"), this);
    auto beginTimestampButton = new QPushButton(tr("Begin"), this);
    layout->addWidget(selectorListWidget, 0, 0, 1, 6);
    layout->addColumnSpacer(6, xPlayerLayout::MediumSpace);
    layout->addWidget(timestampLabel, 0, 7);
    layout->addColumnSpacer(8, xPlayerLayout::SmallSpace);
    layout->addWidget(timestampWidget, 0, 9, 1, 1);
    layout->addColumnSpacer(10, xPlayerLayout::MediumSpace);
    layout->addWidget(nowTimestampButton, 0, 11, 1, 1);
    layout->addColumnSpacer(12, xPlayerLayout::SmallSpace);
    layout->addWidget(beginTimestampButton, 0, 13, 1, 1);
    layout->addColumnSpacer(14, xPlayerLayout::SmallSpace);
    setLayout(layout);
    setFixedHeight(static_cast<int>(QFontMetrics(QApplication::font()).height()*xPlayerSelectorHeightFontFactor));
    // Connect signals.
    connect(selectorListWidget, &QListWidget::itemDoubleClicked, this, &xPlayerMusicAlbumSelectorWidget::selectorClicked);
    connect(nowTimestampButton, &QPushButton::pressed, [this]() { timestampWidget->setDateTime(QDateTime::currentDateTime()); });
    connect(beginTimestampButton, &QPushButton::pressed, [this]() { timestampWidget->setDateTime(QDateTime(QDate(2020, 1, 1), QTime(0, 0))); });
}

void xPlayerMusicAlbumSelectorWidget::setSelectors(const QStringList& selectors) {
    selectorListWidget->clear();
    selectorState.clear();
    // Predefined selectors "none" and "database" including spacer.
    selectorListWidget->addItem(tr("none"));
    selectorState.push_back(xPlayerMusicAlbumSelectorWidget::StateNotUsed);
    selectorListWidget->addItem(QString());
    selectorState.push_back(xPlayerMusicAlbumSelectorWidget::StateSpacer);
    selectorListWidget->addItem(tr("database"));
    selectorState.push_back(xPlayerMusicAlbumSelectorWidget::StateNotUsed);
    // Database selector at index 2, and user defined selectors are starting at index 3.
    currentDatabaseSelectorIndex = 2;
    currentUserSelectorIndex = 3;
    for (const auto& selector : selectors) {
        // Add spacer item, marked with state < 0.
        selectorListWidget->addItem(QString());
        selectorState.push_back(xPlayerMusicAlbumSelectorWidget::StateSpacer);
        // Add selector.
        selectorListWidget->addItem(selector);
        selectorState.push_back(xPlayerMusicAlbumSelectorWidget::StateNotUsed);
    }
}

void xPlayerMusicAlbumSelectorWidget::selectorClicked(QListWidgetItem* selectorItem) {
    if (selectorListWidget->count() != selectorState.count()) {
        qCritical() << "xPlayerMusicAlbumSelectorWidget: item count does not match state count.";
        return;
    }
    auto index = selectorListWidget->row(selectorItem);
    if (index == 0) {
        // Reset all
        auto updateRequired = false;
        for (auto i = 0; i < selectorListWidget->count(); ++i) {
            if (selectorState[i] != xPlayerMusicAlbumSelectorWidget::StateSpacer) {
                selectorState[i] = xPlayerMusicAlbumSelectorWidget::StateNotUsed;
                // Update the background color
                selectorListWidget->item(i)->setBackgroundColor(xPlayerMusicAlbumSelectorWidget_StateColors[xPlayerMusicAlbumSelectorWidget::StateNotUsed]);
                // We need to check and update the selectors
                updateRequired = true;
            }
        }
        // Only send clear if we had to reset any selector.
        if (updateRequired) {
            currentArtistAlbumMap.clear();
            currentTimestamp = 0;
            emit clearAllSelectors();
        }
    } else if ((index > 0) && (index < selectorListWidget->count())) {
        auto state = selectorState[index];
        // Ignore spacer items.
        if (state >= 0) {
            // Move to the next state.
            state = static_cast<xSelectorStates>((state + 1) % xPlayerMusicAlbumSelectorWidget_StateColors.count());
            // Update the background color
            selectorItem->setBackgroundColor(xPlayerMusicAlbumSelectorWidget_StateColors[state]);
            // Save new state.
            selectorState[index] = state;
            // We need to check and update the selectors
            if (index != currentDatabaseSelectorIndex) {
                updateSelectors();
            } else {
                if (state != xSelectorStates::StateNotUsed) {
                    // Database selector has been modified.
                    currentTimestamp = timestampWidget->dateTime().toMSecsSinceEpoch();
                    // Always ask database as it may have been updated in between.
                    currentArtistAlbumMap = xPlayerDatabase::database()->getAllAlbums(currentTimestamp);
                    emit updatedDatabaseSelectors(currentArtistAlbumMap, state == xSelectorStates::StateNotMatch);
                } else {
                    currentArtistAlbumMap.clear();
                    currentTimestamp = 0;
                    emit clearDatabaseSelectors();
                }
            }
        }
    }
}

void xPlayerMusicAlbumSelectorWidget::updateSelectors() {
    QStringList match, notMatch;
    for (auto i = currentUserSelectorIndex; i < selectorListWidget->count(); ++i) {
        if (selectorState[i] == xPlayerMusicAlbumSelectorWidget::StateMatch) {
            match.push_back(selectorListWidget->item(i)->text());
        } else if (selectorState[i] == xPlayerMusicAlbumSelectorWidget::StateNotMatch) {
            notMatch.push_back(selectorListWidget->item(i)->text());
        }
    }
    emit updatedSelectors(match, notMatch);
}