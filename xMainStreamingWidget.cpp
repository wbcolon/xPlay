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

#include "xMainStreamingWidget.h"
#include "xPlayerRotelWidget.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"

#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineHistory>
#include <QWebEngineCookieStore>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QApplication>

xMainStreamingWidget::xMainStreamingWidget(QWidget *parent, Qt::WindowFlags flags):
    QWidget(parent, flags) {
    auto streamingLayout = new xPlayerLayout(this);
    streamingWebView = new QWebEngineView(this);
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // Rotel Controls.
    auto rotelBox = new QGroupBox(tr("Rotel"), this);
    rotelBox->setFlat(xPlayerUseFlatGroupBox);
    auto rotelWidget = new xPlayerRotelWidget(rotelBox, Qt::Vertical);
    auto rotelLayout = new QVBoxLayout();
    rotelLayout->addWidget(rotelWidget);
    rotelBox->setLayout(rotelLayout);
    rotelBox->setEnabled(xPlayerConfiguration::configuration()->rotelWidget());
    // Control box.
    auto controlBox = new QGroupBox("Navigation", this);
    controlBox->setFlat(xPlayerUseFlatGroupBox);
    auto homeButton = new QPushButton(tr("Home"), controlBox);
    auto backButton = new QPushButton(tr("Back"), controlBox);
    auto fwdButton = new QPushButton(tr("Fwd"), controlBox);
    auto reloadButton = new QPushButton(tr("Reload"), controlBox);
    muteAudio = new QCheckBox(tr("Mute Site"), controlBox);
    // Layout.
    auto controlLayout = new xPlayerLayout();
    controlLayout->setSpacing(xPlayerLayout::NoSpace);
    controlLayout->addWidget(homeButton, 0, 0, 1, 2);
    controlLayout->addWidget(backButton, 1, 0, 1, 1);
    controlLayout->addWidget(fwdButton, 1, 1, 1, 1);
    controlLayout->addWidget(reloadButton, 2, 0, 1, 2);
    controlLayout->addRowSpacer(3, xPlayerLayout::SmallSpace);
    controlLayout->addWidget(muteAudio, 4, 0, 1, 2);
    controlBox->setLayout(controlLayout);
    // Sites box.
    auto sitesBox = new QGroupBox(tr("Sites"), this);
    sitesBox->setFlat(xPlayerUseFlatGroupBox);
    sitesCombo = new QComboBox(sitesBox);
    auto historyCheckBox = new QCheckBox(tr("History"), sitesBox);
    historyCheckBox->setChecked(true);
    auto cookiesCheckBox = new QCheckBox(tr("Cookies"), sitesBox);
    cookiesCheckBox->setChecked(true);
    auto cacheCheckBox = new QCheckBox(tr("Cache"), sitesBox);
    cacheCheckBox->setChecked(true);
    auto clearButton = new QPushButton(tr("Clear"), sitesBox);
    // Layout.
    auto siteLayout = new xPlayerLayout();
    siteLayout->setSpacing(xPlayerLayout::NoSpace);
    siteLayout->addWidget(sitesCombo, 0, 0, 1, 2);
    siteLayout->addRowSpacer(1, xPlayerLayout::SmallSpace);
    siteLayout->addWidget(historyCheckBox, 2, 0, 1, 1);
    siteLayout->addWidget(cookiesCheckBox, 2, 1, 1, 1);
    siteLayout->addWidget(cacheCheckBox, 3, 0, 1, 2);
    siteLayout->addWidget(clearButton, 4, 0, 1, 2);
    sitesBox->setLayout(siteLayout);
    // Main streaming layout.
    streamingLayout->addWidget(streamingWebView, 0, 0, 25, 20);
    streamingLayout->addWidget(sitesBox, 0, 21, 2, 1);
    streamingLayout->addRowSpacer(2, xPlayerLayout::SmallSpace);
    streamingLayout->addWidget(controlBox, 3, 21, 4, 1);
    streamingLayout->addRowSpacer(7, xPlayerLayout::SmallSpace);
    streamingLayout->addWidget(rotelBox, 8, 21, 7, 1);
    streamingLayout->addRowSpacer(10, xPlayerLayout::SmallSpace);
    streamingLayout->addRowStretcher(16);
    // Connect Rotel amp widget configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelWidget, [=]() {
        rotelBox->setEnabled(xPlayerConfiguration::configuration()->rotelWidget());
    } );
    // Connect the navigation controls to the QWebEngineView.
    connect(homeButton, &QPushButton::pressed, [=] () { streamingWebView->load(currentSite.second); } );
    connect(backButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::back);
    connect(fwdButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::forward);
    connect(reloadButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::reload);
    connect(muteAudio, &QCheckBox::toggled, streamingWebView->page(), &QWebEnginePage::setAudioMuted);
    connect(clearButton, &QPushButton::pressed, [=] () {
        clearData(historyCheckBox->isChecked(), cookiesCheckBox->isChecked(), cacheCheckBox->isChecked());
    } );
    // Connect Combo Box.
    connect(sitesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentSites(int)));
    // Set user agent to firefox.
    streamingWebView->page()->profile()->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64; rv:75.0) Gecko/20100101 Firefox/75.0");
    streamingWebView->page()->setAudioMuted(false);
}

void xMainStreamingWidget::initializeView() {
    emit showWindowTitle(QApplication::applicationName());
    emit showMenuBar(true);
}

void xMainStreamingWidget::setSites(const QList<std::pair<QString,QUrl>>& sites) {
    streamingSites = sites;
    sitesCombo->clear();
    for (const auto& site : sites) {
        sitesCombo->addItem(site.first);
    }
    // Update for current streaming sites default.
    setSitesDefault(streamingSitesDefault);
}

void xMainStreamingWidget::setSitesDefault(const std::pair<QString, QUrl>& site) {
    streamingSitesDefault = site;
    // Check for streaming sites default. Update the combo box.
    for (int i = 0; i < streamingSites.size(); ++i) {
        if (streamingSitesDefault == streamingSites.at(i)) {
            sitesCombo->setCurrentIndex(i);
            updateCurrentSites(i);
        }
    }
}

const QList<std::pair<QString,QUrl>>& xMainStreamingWidget::getSites() const {
    return streamingSites;
}

const std::pair<QString,QUrl>& xMainStreamingWidget::getSitesDefault() const {
    return streamingSitesDefault;
}

void xMainStreamingWidget::setMuted(bool mute) {
    // Setting the state for the widget will trigger the signal that sets the audio state.
    muteAudio->setChecked(mute);
}

bool xMainStreamingWidget::isMuted() const {
    return streamingWebView->page()->isAudioMuted();
}

void xMainStreamingWidget::updateCurrentSites(int index) {
    if ((index >= 0) && (index < streamingSites.size())) {
        currentSite = streamingSites[index];
        if ((currentSite.second.isValid()) && (!currentSite.second.isEmpty())) {
            streamingWebView->load(currentSite.second);
        } else {
            // Output error if URL is not valid.
            qCritical() << "Url not valid: " << currentSite.second;
        }
    }
}

void xMainStreamingWidget::clearData(bool history, bool cookies, bool cache) {
    if (history) {
        qInfo() << "Clearing History...";
        streamingWebView->page()->profile()->clearAllVisitedLinks();
        streamingWebView->history()->clear();
    }
    if (cookies) {
        qInfo() << "Clearing Cookies...";
        streamingWebView->page()->profile()->cookieStore()->deleteAllCookies();
    }
    if (cache) {
        qInfo() << "Clearing Cache...";
        streamingWebView->page()->profile()->clearHttpCache();
    }
}