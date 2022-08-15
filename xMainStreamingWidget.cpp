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
#include "xPlayerPulseAudioControls.h"
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
        QWidget(parent, flags),
        zoomFactor(1.0) {
    auto streamingLayout = new xPlayerLayout(this);
    streamingWebView = new QWebEngineView(this);
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // Add a control tab for player and rotel amp controls
    auto controlTab = new QTabWidget(this);
    controlTab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto controlVolumeTab = new QWidget(controlTab);
    auto volumeControl = new xPlayerVolumeWidget(true, controlVolumeTab);
    volumeControl->setFixedWidth(xPlayer::ControlButtonWidgetWidth);
    auto controlVolumeLayout = new xPlayerLayout();
    controlVolumeLayout->addRowSpacer(0, 2);
    controlVolumeLayout->addWidget(volumeControl, 1, 0, 1, 1);
    controlVolumeLayout->addRowStretcher(2);
    controlVolumeTab->setLayout(controlVolumeLayout);
    auto controlRotelTab = new xPlayerRotelWidget(controlTab, Qt::Vertical);
    controlTab->setTabPosition(QTabWidget::North);
    controlTab->addTab(controlVolumeTab, "xPlay");
    controlTab->addTab(controlRotelTab, "Rotel");
    // Connect volume and rotel amp controls
    volumeControl->setVolume(xPlayerPulseAudioControls::controls()->getVolume());
    connect(volumeControl, &xPlayerVolumeWidget::volume, [](int vol) {
        xPlayerPulseAudioControls::controls()->setVolume(vol);
    });
    connect(volumeControl, &xPlayerVolumeWidget::muted, this, &xMainStreamingWidget::setMuted);
    // Control box.
    auto controlBox = new QGroupBox("Navigation", this);
    controlBox->setFlat(xPlayer::UseFlatGroupBox);
    auto homeButton = new QPushButton(QIcon(":/images/xplay-home.svg"), "", controlBox);
    homeButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    homeButton->setToolTip(tr("Home"));
    auto backButton = new QPushButton(QIcon(":/images/xplay-left-arrow.svg"), "", controlBox);
    backButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    backButton->setToolTip(tr("Back"));
    auto fwdButton = new QPushButton(QIcon(":/images/xplay-right-arrow.svg"), "", controlBox);
    fwdButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    fwdButton->setToolTip(tr("Forward"));
    auto reloadButton = new QPushButton(QIcon(":/images/xplay-refresh.svg"), "", controlBox);
    reloadButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    reloadButton->setToolTip(tr("Reload"));
    auto zoomBox = new QComboBox(controlBox);
    for (const auto& factor : xPlayerConfiguration::getWebsiteZoomFactors()) {
        zoomBox->addItem(QString("%1%").arg(factor));
    }
    // Layout.
    auto controlLayout = new xPlayerLayout();
    controlLayout->setSpacing(xPlayerLayout::NoSpace);
    controlLayout->addWidget(backButton, 0, 0, 1, 1);
    controlLayout->addWidget(homeButton, 0, 1, 1, 1);
    controlLayout->addWidget(reloadButton, 0, 2, 1, 1);
    controlLayout->addWidget(fwdButton, 0, 3, 1, 1);
    controlLayout->addRowSpacer(1, xPlayerLayout::SmallSpace);
    controlLayout->addWidget(zoomBox, 2, 0, 1, 4);
    controlBox->setLayout(controlLayout);
    // Sites box.
    auto sitesBox = new QGroupBox(tr("Sites"), this);
    sitesBox->setFlat(xPlayer::UseFlatGroupBox);
    sitesCombo = new QComboBox(sitesBox);
    auto historyCheckBox = new QCheckBox(tr("History"), sitesBox);
    historyCheckBox->setChecked(true);
    auto cookiesCheckBox = new QCheckBox(tr("Cookies"), sitesBox);
    cookiesCheckBox->setChecked(true);
    auto cacheCheckBox = new QCheckBox(tr("Cache"), sitesBox);
    cacheCheckBox->setChecked(true);
    auto clearButton = new QPushButton(QIcon(":/images/xplay-clear-data.svg"), "", controlBox);
    clearButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    clearButton->setToolTip(tr("Clear"));
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
    streamingLayout->addWidget(controlBox, 3, 21, 2, 1);
    streamingLayout->addRowSpacer(5, xPlayerLayout::SmallSpace);
    streamingLayout->addWidget(controlTab, 6, 21, 7, 1);
    streamingLayout->addRowStretcher(14);
    // Connect Rotel amp widget configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelWidget, [=]() {
        controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    } );
    // Connect the navigation controls to the QWebEngineView.
    connect(homeButton, &QPushButton::pressed, [=] () { streamingWebView->load(currentSite.second); } );
    connect(backButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::back);
    connect(fwdButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::forward);
    connect(reloadButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::reload);
    connect(clearButton, &QPushButton::pressed, [=] () {
        clearData(historyCheckBox->isChecked(), cookiesCheckBox->isChecked(), cacheCheckBox->isChecked());
    } );
    connect(zoomBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateZoomFactor(int)));
    connect(streamingWebView, &QWebEngineView::loadFinished, this, &xMainStreamingWidget::currentSiteLoadFinished);
    // Connect Combo Box.
    connect(sitesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentSites(int)));
    // Set user agent to firefox.
    streamingWebView->page()->profile()->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64; rv:75.0) Gecko/20100101 Firefox/75.0");
    streamingWebView->page()->setAudioMuted(false);
    zoomBox->setCurrentIndex(xPlayerConfiguration::configuration()->getWebsiteZoomFactorIndex());
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
    streamingWebView->page()->setAudioMuted(mute);
    xPlayerPulseAudioControls::controls()->setMuted(mute);
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

void xMainStreamingWidget::currentSiteLoadFinished(bool ok) {
    Q_UNUSED(ok)
    streamingWebView->page()->setZoomFactor(zoomFactor);
}

void xMainStreamingWidget::updateZoomFactor(int index) {
    try {
        auto zoomFactors = xPlayerConfiguration::getWebsiteZoomFactors();
        zoomFactor = zoomFactors[index] / 100.0;
    } catch (...) {
        // Revert to default factor.
        zoomFactor = 1.0;
    }
    streamingWebView->page()->setZoomFactor(zoomFactor);
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