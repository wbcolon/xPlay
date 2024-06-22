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

#include "xPlayerArtistInfo.h"
#include "xPlayerDatabase.h"
#include "xPlayerConfiguration.h"
#include "xPlayerUI.h"

#include <QComboBox>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineHistory>
#include <QWebEngineCookieStore>
#include <QToolButton>

xPlayerArtistInfo::xPlayerArtistInfo(QWidget* parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        artistName() {
    auto layout = new xPlayerLayout(this);
    layout->setSpacing(xPlayerLayout::NoSpace);
    // Setup URL bar with bookmark selection.
    urlEdit = new QLineEdit(this);
    urlEditAction = new QAction(QIcon(":images/xplay-empty-star.svg"), "Bookmark", urlEdit);
    urlEditAction->setCheckable(true);
    urlEdit->addAction(urlEditAction, QLineEdit::LeadingPosition);
    urlView = new QWebEngineView(this);
    // Configure QWebEngine.
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    urlView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    urlView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    urlView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // Navigation buttons and zoom factor box.
    homeButton = new QPushButton(QIcon(":images/xplay-home.svg"), "", this);
    homeButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    homeButton->setToolTip(tr("Home"));
    auto backButton = new QPushButton(QIcon(":images/xplay-left-arrow.svg"), "", this);
    backButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    backButton->setToolTip(tr("Back"));
    auto fwdButton = new QPushButton(QIcon(":images/xplay-right-arrow.svg"), "", this);
    fwdButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    fwdButton->setToolTip(tr("Forward"));
    auto reloadButton = new QPushButton(QIcon(":/images/xplay-refresh.svg"), "", this);
    reloadButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    reloadButton->setToolTip(tr("Reload"));
    auto closeButton = new QPushButton(QIcon(":images/xplay-close-window.svg"), "", this);
    closeButton->setIconSize(QSize(xPlayer::IconSize, xPlayer::IconSize));
    zoomBox = new QComboBox(this);
    for (const auto percent : xPlayerConfiguration::getWebsiteZoomFactors()) {
        zoomBox->addItem(QString("%1%").arg(percent));
    }
    // Setup default zoom factor.
    auto zoomFactorIndex = xPlayerConfiguration::configuration()->getWebsiteZoomFactorIndex();
    zoomBox->setCurrentIndex(zoomFactorIndex);
    zoomFactor = xPlayerConfiguration::getWebsiteZoomFactors()[zoomFactorIndex] / 100.0;
    // Layout widget.
    layout->addWidget(backButton, 0, 0);
    layout->addWidget(homeButton, 0, 1);
    layout->addWidget(reloadButton, 0, 2);
    layout->addWidget(fwdButton, 0, 3);
    layout->addWidget(urlEdit, 0, 4, 1, 6);
    layout->setColumnStretch(4, 2);
    layout->addColumnSpacer(11, xPlayerLayout::SmallSpace);
    layout->addWidget(zoomBox, 0, 12);
    layout->addColumnSpacer(13, xPlayerLayout::SmallSpace);
    layout->addWidget(closeButton, 0, 14);
    layout->addRowSpacer(1, xPlayerLayout::SmallSpace);
    layout->addWidget(urlView, 2, 0, 10, 15);
    setLayout(layout);
    // Connect navigation buttons.
    connect(homeButton, &QPushButton::pressed, [=]() { show(artistName); } );
    connect(backButton, &QPushButton::pressed, urlView, &QWebEngineView::back);
    connect(fwdButton, &QPushButton::pressed, urlView, &QWebEngineView::forward);
    connect(reloadButton, &QPushButton::pressed, urlView, &QWebEngineView::reload);
    connect(closeButton, &QPushButton::pressed, this, &xPlayerArtistInfo::close);
    connect(zoomBox, SIGNAL(currentIndexChanged(int)), this, SLOT(zoomFactorChanged(int)));
    // Connect url view.
    connect(urlView, &QWebEngineView::urlChanged, this, &xPlayerArtistInfo::urlChanged);
    connect(urlView, &QWebEngineView::loadFinished, this, &xPlayerArtistInfo::urlLoadFinished);
    connect(urlEdit, &QLineEdit::returnPressed, this, &xPlayerArtistInfo::urlUpdated);
    connect(urlEditAction, &QAction::toggled, this, &xPlayerArtistInfo::bookmarkToggled);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedWebsiteZoomFactor,
            this, &xPlayerArtistInfo::updatedZoomFactor);
    // Set user agent to firefox.
    urlView->page()->profile()->setHttpUserAgent(xPlayerConfiguration::configuration()->getWebsiteUserAgent());
    urlView->page()->setAudioMuted(false);
    urlView->page()->setZoomFactor(zoomFactor);
}

void xPlayerArtistInfo::show(const QString& artist) {
    if (artist.isEmpty()) {
        return;
    }
    // Show blank page.
    urlView->load(QUrl("about:blank"));
    // Clear history before loading new artists page.
    urlView->page()->profile()->clearAllVisitedLinks();
    urlView->history()->clear();
    // Update artist name.
    artistName = artist;
    // Query database for artist info. Configure URL.
    auto artistUrl = xPlayerDatabase::database()->getArtistURL(artist);
    // Update the action (and icon) in the url line edit.
    disconnect(urlEditAction, &QAction::toggled, this, &xPlayerArtistInfo::bookmarkToggled);
    if (artistUrl.isEmpty()) {
        artistUrl = QString("https://en.wikipedia.org/w/index.php?search=%1").arg(artistName);
        urlEditAction->setChecked(false);
        urlEditAction->setIcon(QIcon(":images/xplay-empty-star.svg"));
    } else {
        urlEditAction->setChecked(true);
        urlEditAction->setIcon(QIcon(":images/xplay-star.svg"));
    }
    connect(urlEditAction, &QAction::toggled, this, &xPlayerArtistInfo::bookmarkToggled);
    // Load the site.
    urlView->load(QUrl(artistUrl));
    urlView->page()->setZoomFactor(zoomFactor);
}

void xPlayerArtistInfo::bookmarkToggled(bool enabled) {
    if (enabled) {
        urlEditAction->setIcon(QIcon(":images/xplay-star.svg"));
        xPlayerDatabase::database()->updateArtistURL(artistName, urlEdit->text());
    } else {
        urlEditAction->setIcon(QIcon(":images/xplay-empty-star.svg"));
        xPlayerDatabase::database()->removeArtistURL(artistName);
    }
}

void xPlayerArtistInfo::urlChanged(const QUrl& url) {
    urlEdit->setText(url.toString());
}

void xPlayerArtistInfo::urlUpdated() {
    // Load the site.
    urlView->load(QUrl(urlEdit->text()));
}

void xPlayerArtistInfo::urlLoadFinished(bool ok) {
    Q_UNUSED(ok)
    urlView->page()->setZoomFactor(zoomFactor);
}

void xPlayerArtistInfo::zoomFactorChanged(int index) {
    try {
        zoomFactor = xPlayerConfiguration::getWebsiteZoomFactors()[index] / 100.0;
        urlView->page()->setZoomFactor(zoomFactor);
    } catch (...) {
        // Ignore error.
    }
}

void xPlayerArtistInfo::updatedZoomFactor() {
    auto zoomFactorIndex = xPlayerConfiguration::configuration()->getWebsiteZoomFactorIndex();
    zoomBox->setCurrentIndex(zoomFactorIndex);
}