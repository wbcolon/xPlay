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

#include "xPlayerSelectorWidget.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include <QApplication>
#include <QDebug>

const QVector<QColor> xPlayerSelectorWidget_StateColors { QColor(Qt::white), QColor(Qt::green), QColor(Qt::red) };

xPlayerSelectorWidget::xPlayerSelectorWidget(QWidget* parent):
        QListWidget(parent) {
    // Setup the list widget.
    setViewMode(QListWidget::IconMode);
    setSelectionMode(QListWidget::NoSelection);
    connect(this, &QListWidget::itemDoubleClicked, this, &xPlayerSelectorWidget::selectorClicked);
    setFixedHeight(static_cast<int>(QFontMetrics(QApplication::font()).height()*xPlayerSelectorHeightFontFactor));
}

void xPlayerSelectorWidget::setSelectors(const QStringList& selectors) {
    clear();
    selectorState.clear();
    addItem(tr("none"));
    selectorState.push_back(xPlayerSelectorWidget::StateNotUsed);
    for (const auto& selector : selectors) {
        // Add spacer item, marked with state < 0.
        addItem(QString());
        selectorState.push_back(xPlayerSelectorWidget::StateSpacer);
        // Add selector.
        addItem(selector);
        selectorState.push_back(xPlayerSelectorWidget::StateNotUsed);
    }
}

void xPlayerSelectorWidget::selectorClicked(QListWidgetItem* selectorItem) {
    if (count() != selectorState.count()) {
        qCritical() << "xPlayerSelectorWidget: item count does not match state count.";
        return;
    }
    auto index = row(selectorItem);
    if (index == 0) {
        // Reset all
        for (auto i = 0; i < count(); ++i) {
            if (selectorState[i] != xPlayerSelectorWidget::StateSpacer) {
                selectorState[i] = xPlayerSelectorWidget::StateNotUsed;
                // Update the background color
                item(i)->setBackgroundColor(xPlayerSelectorWidget_StateColors[xPlayerSelectorWidget::StateNotUsed]);
                // We need to check and update the selectors
                updateSelectors();
            }
        }
    } else if ((index > 0) && (index < count())) {
        auto state = selectorState[index];
        // Ignore spacer items.
        if (state >= 0) {
            // Move to the next state.
            state = static_cast<xSelectorStates>((state + 1) % xPlayerSelectorWidget_StateColors.count());
            // Update the background color
            selectorItem->setBackgroundColor(xPlayerSelectorWidget_StateColors[state]);
            // Save new state.
            selectorState[index] = state;
            // We need to check and update the selectors
            updateSelectors();
        }
    }
}

void xPlayerSelectorWidget::updateSelectors() {
    QStringList match, notMatch;
    for (auto i = 0; i < count(); ++i) {
        if (selectorState[i] == xPlayerSelectorWidget::StateMatch) {
            match.push_back(item(i)->text());
        } else if (selectorState[i] == xPlayerSelectorWidget::StateNotMatch) {
            notMatch.push_back(item(i)->text());
        }
    }
    emit updatedSelectors(match, notMatch);
}