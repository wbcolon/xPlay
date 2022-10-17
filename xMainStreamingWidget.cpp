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
#include <QPushButton>
#include <QSplitter>
#include <QApplication>

xMainStreamingWidget::xMainStreamingWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        zoomFactor(1.0) {

    auto streamingLayout = new xPlayerLayout(this);
    // Splitter for sidebar and the central webview.
    auto sideBarWebViewSplitter = new QSplitter(this);
    sideBarWebViewSplitter->setOrientation(Qt::Horizontal);
    // Webview.
    streamingWebView = new QWebEngineView(sideBarWebViewSplitter);
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // Sidebar.
    auto sideBarWidget = new QWidget(sideBarWebViewSplitter);
    auto sideBarLayout = new xPlayerLayout(sideBarWidget);
    // Sites box.
    auto sitesBox = new QGroupBox(tr("Sites"), sideBarWidget);
    sitesBox->setFlat(xPlayer::UseFlatGroupBox);
    sitesCombo = new QComboBox(sitesBox);
    auto historyCheckBox = new QCheckBox(tr("History"), sitesBox);
    historyCheckBox->setChecked(true);
    auto cookiesCheckBox = new QCheckBox(tr("Cookies"), sitesBox);
    cookiesCheckBox->setChecked(true);
    auto cacheCheckBox = new QCheckBox(tr("Cache"), sitesBox);
    cacheCheckBox->setChecked(true);
    auto clearButton = new QPushButton(QIcon(":/images/xplay-clear-data.svg"), "", sitesBox);
    clearButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    clearButton->setToolTip(tr("Clear"));
    // Sites box layout.
    auto siteLayout = new xPlayerLayout(sitesBox);
    siteLayout->setSpacing(xPlayerLayout::NoSpace);
    siteLayout->addWidget(sitesCombo, 0, 0, 1, 2);
    siteLayout->addRowSpacer(1, xPlayerLayout::SmallSpace);
    siteLayout->addWidget(historyCheckBox, 2, 0, 1, 1);
    siteLayout->addWidget(cookiesCheckBox, 2, 1, 1, 1);
    siteLayout->addWidget(cacheCheckBox, 3, 0, 1, 2);
    siteLayout->addWidget(clearButton, 4, 0, 1, 2);
    // Control tab for player and Rotel amp controls
    auto controlTab = new QTabWidget(sideBarWidget);
    controlTab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    auto controlVolumeTab = new QWidget(controlTab);
    auto volumeControl = new xPlayerVolumeWidget(true, controlVolumeTab);
    volumeControl->setFixedWidth(xPlayer::ControlButtonWidgetWidth);
    // Control tab layout.
    auto controlVolumeLayout = new xPlayerLayout(controlVolumeTab);
    controlVolumeLayout->addRowSpacer(0, 2);
    controlVolumeLayout->addWidget(volumeControl, 1, 0, 1, 1);
    controlVolumeLayout->addRowStretcher(2);
    auto controlRotelTab = new xPlayerRotelWidget(controlTab, Qt::Vertical);
    controlTab->setTabPosition(QTabWidget::North);
    controlTab->addTab(controlVolumeTab, "xPlay");
    controlTab->addTab(controlRotelTab, "Rotel");
    // Add sites box and control tab to sidebar.
    sideBarWidget->setMaximumWidth(xPlayer::SidebarWidgetWidth);
    sideBarLayout->addRowSpacer(0, xPlayerLayout::MediumSpace);
    sideBarLayout->addWidget(sitesBox, 1, 0);
    sideBarLayout->addRowSpacer(2, xPlayerLayout::SmallSpace);
    sideBarLayout->addWidget(controlTab, 3, 0);
    sideBarLayout->addRowStretcher(4);
    // Add sidebar and webview to splitter.
    sideBarWebViewSplitter->addWidget(sideBarWidget);
    sideBarWebViewSplitter->addWidget(streamingWebView);
    sideBarWebViewSplitter->setCollapsible(0, true);
    sideBarWebViewSplitter->setStretchFactor(0, 0);
    sideBarWebViewSplitter->setCollapsible(1, false);
    sideBarWebViewSplitter->setStretchFactor(1, 1);
    // Navigation elements
    auto homeButton = new QPushButton(QIcon(":/images/xplay-home.svg"), "", this);
    homeButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    homeButton->setToolTip(tr("Home"));
    auto backButton = new QPushButton(QIcon(":/images/xplay-left-arrow.svg"), "", this);
    backButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    backButton->setToolTip(tr("Back"));
    auto fwdButton = new QPushButton(QIcon(":/images/xplay-right-arrow.svg"), "", this);
    fwdButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    fwdButton->setToolTip(tr("Forward"));
    auto reloadButton = new QPushButton(QIcon(":/images/xplay-refresh.svg"), "", this);
    reloadButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    reloadButton->setToolTip(tr("Reload"));
    streamingUrl = new QLineEdit(this);
    zoomBox = new QComboBox(this);
    for (const auto& factor : xPlayerConfiguration::getWebsiteZoomFactors()) {
        zoomBox->addItem(QString("%1%").arg(factor));
    }
    // Main streaming layout.
    streamingLayout->addWidget(backButton, 0, 0);
    streamingLayout->addWidget(homeButton, 0, 1);
    streamingLayout->addWidget(reloadButton, 0, 2);
    streamingLayout->addWidget(fwdButton, 0, 3);
    streamingLayout->addWidget(streamingUrl, 0, 4, 1, 44);
    streamingLayout->addWidget(zoomBox, 0, 48, 1, 2);
    streamingLayout->addWidget(sideBarWebViewSplitter, 1, 0, 25, 50);
    streamingLayout->addRowStretcher(13);
    // Connect Rotel amp widget configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelWidget, [=]() {
        controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    } );
    // Connect volume and rotel amp controls
    volumeControl->setVolume(xPlayerPulseAudioControls::controls()->getVolume());
    connect(volumeControl, &xPlayerVolumeWidget::volume, [](int vol) {
        xPlayerPulseAudioControls::controls()->setVolume(vol);
    });
    connect(volumeControl, &xPlayerVolumeWidget::muted, this, &xMainStreamingWidget::setMuted);
    // Connect the navigation controls to the QWebEngineView.
    connect(homeButton, &QPushButton::pressed, [=] () { streamingWebView->load(currentSite.second); } );
    connect(backButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::back);
    connect(fwdButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::forward);
    connect(reloadButton, &QPushButton::pressed, streamingWebView, &QWebEngineView::reload);
    connect(clearButton, &QPushButton::pressed, [=] () {
        clearData(historyCheckBox->isChecked(), cookiesCheckBox->isChecked(), cacheCheckBox->isChecked());
    } );
    connect(zoomBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateZoomFactor(int)));
    connect(streamingUrl, &QLineEdit::returnPressed, this, &xMainStreamingWidget::urlUpdated);
    connect(streamingWebView, &QWebEngineView::loadFinished, this, &xMainStreamingWidget::urlLoadFinished);
    connect(streamingWebView, &QWebEngineView::urlChanged, this, &xMainStreamingWidget::urlChanged);
    // Connect Combo Box.
    connect(sitesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentSites(int)));
    // Set zoom factor.
    updatedZoomFactor();
    // Set user agent to firefox.
    updatedUserAgent();
    streamingWebView->page()->setAudioMuted(false);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedWebsiteZoomFactor,
            this, &xMainStreamingWidget::updatedZoomFactor);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedWebsiteZoomFactor,
            this, &xMainStreamingWidget::updatedUserAgent);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedStreamingSites,
            this, &xMainStreamingWidget::updatedStreamingSites);
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedStreamingSitesDefault,
            this, &xMainStreamingWidget::updatedStreamingSitesDefault);
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

void xMainStreamingWidget::updatedStreamingSites() {
    setSites(xPlayerConfiguration::configuration()->getStreamingSites());
}

void xMainStreamingWidget::updatedStreamingSitesDefault() {
    setSitesDefault(xPlayerConfiguration::configuration()->getStreamingSitesDefault());
}

void xMainStreamingWidget::urlChanged(const QUrl& url) {
    streamingUrl->setText(url.toString());
}

void xMainStreamingWidget::urlUpdated() {
    // Load the site.
    streamingWebView->load(QUrl(streamingUrl->text()));
}

void xMainStreamingWidget::urlLoadFinished(bool ok) {
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

void xMainStreamingWidget::updatedZoomFactor() {
    auto zoomFactorIndex = xPlayerConfiguration::configuration()->getWebsiteZoomFactorIndex();
    zoomBox->setCurrentIndex(zoomFactorIndex);
}

void xMainStreamingWidget::updatedUserAgent() {
    streamingWebView->page()->profile()->setHttpUserAgent(xPlayerConfiguration::configuration()->getWebsiteUserAgent());
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