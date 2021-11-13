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

#include "xPlayerConfigurationDialog.h"
#include "xPlayerUI.h"

#include <QGroupBox>
#include <QComboBox>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRegularExpression>
#include <QTabWidget>
#include <QDebug>

xPlayerConfigurationDialog::xPlayerConfigurationDialog(QWidget* parent, Qt::WindowFlags flags):
        QDialog(parent, flags) {
    auto configurationLayout = new xPlayerLayout(this);
    configurationLayout->setSpacing(xPlayerLayout::NoSpace);
    auto configurationTab = new QTabWidget(this);
    configurationTab->setTabPosition(QTabWidget::South);
    configurationTab->setFocusPolicy(Qt::NoFocus);
    auto musicLibraryTab = new QGroupBox(tr("Music Library Configuration"), configurationTab);
    musicLibraryTab->setFlat(xPlayer::UseFlatGroupBox);
    auto movieLibraryTab = new QGroupBox(tr("Movie Library Configuration"), configurationTab);
    movieLibraryTab->setFlat(xPlayer::UseFlatGroupBox);
    auto streamingSitesTab = new QGroupBox(tr("Streaming Sites Configuration"), configurationTab);
    streamingSitesTab->setFlat(xPlayer::UseFlatGroupBox);
    auto additionalTab = new QWidget(configurationTab);
    auto additionalLayout = new xPlayerLayout();
    auto databaseBox = new QGroupBox(tr("Database Configuration"), additionalTab);
    databaseBox->setFlat(xPlayer::UseFlatGroupBox);
    auto rotelBox = new QGroupBox(tr("Rotel Configuration"), additionalTab);
    rotelBox->setFlat(xPlayer::UseFlatGroupBox);
    auto websiteBox = new QGroupBox(tr("Website Configuration"), additionalTab);
    websiteBox->setFlat(xPlayer::UseFlatGroupBox);
    configurationTab->addTab(musicLibraryTab, tr("Music Library"));
    configurationTab->addTab(movieLibraryTab, tr("Movie Library"));
    configurationTab->addTab(streamingSitesTab, tr("Streaming Sites"));
    configurationTab->addTab(additionalTab, tr("Additional"));
    auto configurationButtons = new QDialogButtonBox(Qt::Horizontal, this);
    configurationButtons->addButton(QDialogButtonBox::Save);
    configurationButtons->addButton(QDialogButtonBox::Reset);
    configurationButtons->addButton(QDialogButtonBox::Cancel);
    configurationButtons->setFocusPolicy(Qt::NoFocus);
    // Setup movie library setup, with tags, directories and extensions.
    auto movieLibraryLayout = new xPlayerLayout();
    auto movieLibraryTagLabel = new QLabel(tr("Tag"), movieLibraryTab);
    movieLibraryTagWidget = new QLineEdit(movieLibraryTab);
    auto movieLibraryDirectoryLabel = new QLabel(tr("Directory"), movieLibraryTab);
    movieLibraryDirectoryWidget = new QLineEdit(movieLibraryTab);
    auto movieLibraryDirectoryOpenButton = new QPushButton(tr("..."), movieLibraryTab);
    auto movieLibraryButtons = new QDialogButtonBox(Qt::Horizontal, movieLibraryTab);
    movieLibraryButtons->addButton(QDialogButtonBox::Apply);
    movieLibraryButtons->addButton(QDialogButtonBox::Discard);
    movieLibraryListWidget = new QListWidget(movieLibraryTab);
    movieLibraryListWidget->setSortingEnabled(true);
    auto movieLibraryExtensionsLabel = new QLabel(tr("Extensions"), movieLibraryTab);
    movieLibraryExtensionsWidget = new QLineEdit(movieLibraryTab);
    auto movieDefaultAudioLanguageLabel = new QLabel(tr("Default Audio Channel Language"), movieLibraryTab);
    movieDefaultAudioLanguageWidget = new QComboBox(movieLibraryTab);
    movieDefaultAudioLanguageWidget->addItem(tr("movie default"));
    movieDefaultAudioLanguageWidget->addItems(xPlayerConfiguration::getMovieDefaultLanguages());
    auto movieDefaultSubtitleLanguageLabel = new QLabel(tr("Default Subtitle Language"), movieLibraryTab);
    movieDefaultSubtitleLanguageWidget = new QComboBox(movieLibraryTab);
    movieDefaultSubtitleLanguageWidget->addItem(tr("disable"));
    movieDefaultSubtitleLanguageWidget->addItems(xPlayerConfiguration::getMovieDefaultLanguages());
    movieLibraryLayout->addWidget(movieLibraryTagLabel, 0, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryTagWidget, 1, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryDirectoryLabel, 2, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryDirectoryWidget, 3, 0, 1, 4);
    movieLibraryLayout->addWidget(movieLibraryDirectoryOpenButton, 3, 4);
    movieLibraryLayout->addWidget(movieLibraryListWidget, 4, 0, 3, 5);
    movieLibraryLayout->addWidget(movieLibraryButtons, 7, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryExtensionsLabel, 8, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryExtensionsWidget, 9, 0, 1, 5);
    movieLibraryLayout->addRowSpacer(10, xPlayerLayout::LargeSpace);
    movieLibraryLayout->addWidget(movieDefaultAudioLanguageLabel, 11, 0, 1, 2);
    movieLibraryLayout->addWidget(movieDefaultAudioLanguageWidget, 12, 0, 1, 2);
    movieLibraryLayout->addWidget(movieDefaultSubtitleLanguageLabel, 11, 3, 1, 2);
    movieLibraryLayout->addWidget(movieDefaultSubtitleLanguageWidget, 12, 3, 1, 2);
    movieLibraryLayout->addRowStretcher(13);
    movieLibraryTab->setLayout(movieLibraryLayout);
    // Setup music library with directory and extensions.
    auto musicLibraryLayout = new xPlayerLayout();
    auto musicLibraryDirectoryLabel = new QLabel(tr("Directory"), musicLibraryTab);
    musicLibraryDirectoryWidget = new QLineEdit(musicLibraryTab);
    auto musicLibraryDirectoryOpenButton = new QPushButton(tr("..."), musicLibraryTab);
    auto musicLibraryExtensionsLabel = new QLabel(tr("Extensions"), musicLibraryTab);
    musicLibraryExtensionsWidget = new QLineEdit(musicLibraryTab);
    auto musicLibraryAlbumSelectorsLabel = new QLabel(tr("Album Selectors"), musicLibraryTab);
    musicLibraryAlbumSelectorsWidget = new QLineEdit(musicLibraryTab);
    auto musicLibraryTagsLabel = new QLabel(tr("Tags"), musicLibraryTab);
    musicLibraryTagsWidget = new QLineEdit(musicLibraryTab);
    musicLibraryLayout->addWidget(musicLibraryDirectoryLabel, 0, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryDirectoryWidget, 1, 0, 1, 4);
    musicLibraryLayout->addWidget(musicLibraryDirectoryOpenButton, 1, 4);
    musicLibraryLayout->addWidget(musicLibraryExtensionsLabel, 2, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryExtensionsWidget, 3, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryAlbumSelectorsLabel, 4, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryAlbumSelectorsWidget, 5, 0, 1, 5);
    musicLibraryLayout->addRowSpacer(6, xPlayerLayout::LargeSpace);
    musicLibraryLayout->addWidget(musicLibraryTagsLabel, 7, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryTagsWidget, 8, 0, 1, 5);
    musicLibraryLayout->addRowStretcher(9);
    musicLibraryTab->setLayout(musicLibraryLayout);
    // Setup streaming sites with URL and short name.
    auto streamingSitesLayout = new xPlayerLayout();
    auto streamingNameLabel = new QLabel(tr("Name"), streamingSitesTab);
    streamingNameWidget = new QLineEdit(streamingSitesTab);
    auto streamingUrlLabel = new QLabel(tr("Url"), streamingSitesTab);
    streamingUrlWidget = new QLineEdit(streamingSitesTab);
    auto streamingSitesButtons = new QDialogButtonBox(Qt::Horizontal, streamingSitesTab);
    streamingSitesButtons->addButton(QDialogButtonBox::Apply);
    streamingSitesButtons->addButton(QDialogButtonBox::Discard);
    auto streamingSitesDefaultButton = streamingSitesButtons->addButton(tr("Default"), QDialogButtonBox::ResetRole);
    streamingSitesListWidget = new QListWidget(streamingSitesTab);
    streamingSitesListWidget->setSortingEnabled(true);
    streamingSitesLayout->addWidget(streamingNameLabel, 0, 0, 1, 2);
    streamingSitesLayout->addWidget(streamingNameWidget, 1, 0, 1, 2);
    streamingSitesLayout->addWidget(streamingUrlLabel, 1, 2, 1, 3);
    streamingSitesLayout->addWidget(streamingUrlWidget, 1, 2, 1, 3);
    streamingSitesLayout->addWidget(streamingSitesListWidget, 2, 0, 3, 5);
    streamingSitesLayout->addWidget(streamingSitesButtons, 5, 0, 1, 5);
    streamingSitesLayout->setRowStretch(2, 2);
    streamingSitesTab->setLayout(streamingSitesLayout);
    // Setup rotel amp with network address and port.
    auto rotelLayout = new xPlayerLayout();
    auto rotelNetworkAddressLabel = new QLabel(tr("Network Address"), rotelBox);
    auto rotelNetworkPortLabel = new QLabel(tr("Port"), rotelBox);
    rotelNetworkAddressWidget = new QLineEdit(rotelBox);
    rotelNetworkPortWidget = new QSpinBox(rotelBox);
    rotelNetworkPortWidget->setRange(0, 10000);
    rotelEnableWidget = new QPushButton(tr("disable"), rotelBox);
    rotelLayout->addWidget(rotelNetworkAddressLabel, 0, 0, 1, 3);
    rotelLayout->addWidget(rotelNetworkAddressWidget, 1, 0, 1, 3);
    rotelLayout->addWidget(rotelNetworkPortLabel, 0, 3);
    rotelLayout->addWidget(rotelNetworkPortWidget, 1, 3);
    rotelLayout->addWidget(rotelEnableWidget, 1, 4, 1, 1);
    rotelBox->setLayout(rotelLayout);
    // Setup database configuration.
    auto databaseLayout = new xPlayerLayout();
    auto databaseDirectoryLabel = new QLabel(tr("Directory"), databaseBox);
    databaseDirectoryWidget = new QLineEdit(databaseBox);
    databaseDirectoryWidget->setReadOnly(true);
    auto databaseDirectoryButton = new QPushButton("...", databaseBox);
    databaseMusicOverlayCheck = new QCheckBox(tr("Enable database overlay for music library"),databaseBox);
    databaseMovieOverlayCheck = new QCheckBox(tr("Enable database overlay for movie library"),databaseBox);
    databaseCutOffDate = new QDateEdit(databaseBox);
    databaseCutOffDate->setDisplayFormat("dd MMMM yyyy");
    databaseCutOffCheck = new QCheckBox(tr("Use cut-off date"), databaseBox);
    databaseIgnoreUpdateErrorsCheck = new QCheckBox(tr("Ignore database update errors"), databaseBox);
    databaseLayout->addWidget(databaseDirectoryLabel, 0, 0, 1, 5);
    databaseLayout->addWidget(databaseDirectoryWidget, 1, 0, 1, 4);
    databaseLayout->addWidget(databaseDirectoryButton, 1, 4, 1, 1);
    databaseLayout->addRowSpacer(2, xPlayerLayout::MediumSpace);
    databaseLayout->addWidget(databaseMusicOverlayCheck, 3, 0, 1, 5);
    databaseLayout->addWidget(databaseMovieOverlayCheck, 4, 0, 1, 5);
    databaseLayout->addWidget(databaseCutOffCheck, 5, 0, 1, 3);
    databaseLayout->addWidget(databaseCutOffDate, 5, 3, 1, 2);
    databaseLayout->addWidget(databaseIgnoreUpdateErrorsCheck, 6, 0, 1, 5);
    databaseBox->setLayout(databaseLayout);
    // Setup website default zoom factor.
    auto websiteLayout = new xPlayerLayout();
    auto websiteZoomFactorLabel = new QLabel(tr("Zoom Factor"), websiteBox);
    websiteZoomFactors = new QComboBox(websiteBox);
    for (const auto& factor : xPlayerConfiguration::getWebsiteZoomFactors()) {
        websiteZoomFactors->addItem(QString("%1%").arg(factor));
    }
    websiteLayout->addWidget(websiteZoomFactorLabel, 0, 0, 1, 5);
    websiteLayout->addWidget(websiteZoomFactors, 1, 0);
    websiteBox->setLayout(websiteLayout);
    // Additional tab layout.
    additionalLayout->addWidget(databaseBox, 0, 0, 2, 4);
    additionalLayout->addRowSpacer(2, xPlayerLayout::HugeSpace);
    additionalLayout->addWidget(rotelBox, 3, 0, 1, 4);
    additionalLayout->addRowSpacer(4, xPlayerLayout::HugeSpace);
    additionalLayout->addWidget(websiteBox, 5, 0, 1, 4);
    additionalLayout->addRowStretcher(6);
    additionalTab->setLayout(additionalLayout);
    // Configuration layout.
    configurationLayout->addWidget(configurationTab, 0, 0, 4, 4);
    configurationLayout->addRowSpacer(4, xPlayerLayout::HugeSpace);
    configurationLayout->addWidget(configurationButtons, 5, 0, 1, 4);
    setLayout(configurationLayout);
    // Connect rotel button
    connect(rotelEnableWidget, &QPushButton::pressed, this, &xPlayerConfigurationDialog::toggleRotelWidget);
    // Connect dialog buttons.
    connect(musicLibraryDirectoryOpenButton, &QPushButton::pressed, this, &xPlayerConfigurationDialog::openMusicLibraryDirectory);
    connect(movieLibraryDirectoryOpenButton, &QPushButton::pressed, this, &xPlayerConfigurationDialog::openMovieLibraryDirectory);
    // Connect movie library.
    connect(movieLibraryButtons->button(QDialogButtonBox::Apply), &QPushButton::pressed, this, &xPlayerConfigurationDialog::movieLibraryAdd);
    connect(movieLibraryButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed, this, &xPlayerConfigurationDialog::movieLibraryRemove);
    connect(movieLibraryListWidget, &QListWidget::currentItemChanged, this, &xPlayerConfigurationDialog::selectMovieLibrary);
    // Connect streaming sites.
    connect(streamingSitesButtons->button(QDialogButtonBox::Apply), &QPushButton::pressed, this, &xPlayerConfigurationDialog::streamingSiteAdd);
    connect(streamingSitesButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed, this, &xPlayerConfigurationDialog::streamingSiteRemove);
    connect(streamingSitesDefaultButton, &QPushButton::pressed, this, &xPlayerConfigurationDialog::streamingSiteDefault);
    connect(streamingSitesListWidget, &QListWidget::currentItemChanged, this, &xPlayerConfigurationDialog::selectStreamingSite);
    // Connect database button and check box.
    connect(databaseDirectoryButton, &QPushButton::pressed, this, &xPlayerConfigurationDialog::openDatabaseDirectory);
    connect(databaseCutOffCheck, &QCheckBox::clicked, databaseCutOffDate, &QDateEdit::setEnabled);
    // Connect dialog buttons.
    connect(configurationButtons->button(QDialogButtonBox::Save), &QPushButton::pressed, this, &xPlayerConfigurationDialog::saveSettings);
    connect(configurationButtons->button(QDialogButtonBox::Reset), &QPushButton::pressed, this, &xPlayerConfigurationDialog::loadSettings);
    connect(configurationButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &QDialog::reject);
    // Load and resize.
    loadSettings();
    setMinimumWidth(static_cast<int>(sizeHint().height()*1.5)); // NOLINT
    setMinimumHeight(sizeHint().height()); // NOLINT
}

void xPlayerConfigurationDialog::loadSettings() {
    auto musicLibraryDirectory = xPlayerConfiguration::configuration()->getMusicLibraryDirectory();
    auto musicLibraryExtensions = xPlayerConfiguration::configuration()->getMusicLibraryExtensions();
    auto musicLibraryAlbumSelectors = xPlayerConfiguration::configuration()->getMusicLibraryAlbumSelectors();
    auto musicLibraryTags = xPlayerConfiguration::configuration()->getMusicLibraryTags();
    auto rotelWidget = xPlayerConfiguration::configuration()->rotelWidget();
    auto [rotelNetworkAddress, rotelNetworkPort] = xPlayerConfiguration::configuration()->getRotelNetworkAddress();
    auto movieLibraryTagAndDirectory = xPlayerConfiguration::configuration()->getMovieLibraryTagAndDirectory();
    auto movieLibraryExtensions = xPlayerConfiguration::configuration()->getMovieLibraryExtensions();
    auto movieDefaultAudioLanguage = xPlayerConfiguration::configuration()->getMovieDefaultAudioLanguage();
    auto movieDefaultSubtitleLanguage = xPlayerConfiguration::configuration()->getMovieDefaultSubtitleLanguage();
    auto databaseDirectory = xPlayerConfiguration::configuration()->getDatabaseDirectory();
    auto databaseCutOff = xPlayerConfiguration::configuration()->getDatabaseCutOff();
    auto streamingSites = xPlayerConfiguration::configuration()->getStreamingSites();
    streamingSitesDefault = xPlayerConfiguration::configuration()->getStreamingSitesDefault();
    auto zoomFactorIndex = xPlayerConfiguration::configuration()->getWebsiteZoomFactorIndex();
    // Update the configuration dialog UI.
    musicLibraryDirectoryWidget->setText(musicLibraryDirectory);
    musicLibraryExtensionsWidget->setText(musicLibraryExtensions);
    musicLibraryAlbumSelectorsWidget->setText(musicLibraryAlbumSelectors);
    musicLibraryTagsWidget->setText(musicLibraryTags.join(" "));
    rotelNetworkAddressWidget->setText(rotelNetworkAddress);
    rotelNetworkPortWidget->setValue(rotelNetworkPort);
    if (!rotelWidget) {
        rotelNetworkAddressWidget->setEnabled(false);
        rotelNetworkPortWidget->setEnabled(false);
        rotelEnableWidget->setText(tr("enable"));
    }
    databaseDirectoryWidget->setText(databaseDirectory);
    databaseMusicOverlayCheck->setChecked(xPlayerConfiguration::configuration()->getDatabaseMusicOverlay());
    databaseMovieOverlayCheck->setChecked(xPlayerConfiguration::configuration()->getDatabaseMovieOverlay());
    if (databaseCutOff) {
        databaseCutOffCheck->setChecked(true);
        databaseCutOffDate->setDate(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(databaseCutOff)).date());
    } else {
        databaseCutOffCheck->setChecked(false);
        databaseCutOffDate->setEnabled(false);
    }
    databaseIgnoreUpdateErrorsCheck->setChecked(xPlayerConfiguration::configuration()->getDatabaseIgnoreUpdateErrors());
    movieLibraryExtensionsWidget->setText(movieLibraryExtensions);
    movieLibraryListWidget->clear();
    if (movieLibraryTagAndDirectory.count() > 0) {
        for (const auto& entry : movieLibraryTagAndDirectory) {
            auto splitEntry = splitMovieLibraryEntry(entry);
            movieLibraryListWidget->addItem(QString("(%1) - %2").arg(splitEntry.first, splitEntry.second));
        }
    }
    if (movieDefaultAudioLanguage.isEmpty()) {
        movieDefaultAudioLanguageWidget->setCurrentIndex(0);
    } else {
        movieDefaultAudioLanguageWidget->setCurrentText(movieDefaultAudioLanguage);
    }
    if (movieDefaultSubtitleLanguage.isEmpty()) {
        movieDefaultSubtitleLanguageWidget->setCurrentIndex(0);
    } else {
        movieDefaultSubtitleLanguageWidget->setCurrentText(movieDefaultSubtitleLanguage);
    }
    streamingSitesListWidget->clear();
    if (!streamingSites.isEmpty()) {
        for (const auto& entry : streamingSites) {
            streamingSitesListWidget->addItem(QString("(%1) - %2").arg(entry.first, entry.second.toString()));
        }
    }
    updateStreamingSitesDefault();
    websiteZoomFactors->setCurrentIndex(zoomFactorIndex);
}

void xPlayerConfigurationDialog::saveSettings() {
    // Read setting entries.
    auto musicLibraryDirectory = musicLibraryDirectoryWidget->text();
    auto musicLibraryExtensions = musicLibraryExtensionsWidget->text();
    auto musicLibraryAlbumSelectors = musicLibraryAlbumSelectorsWidget->text();
    auto musicLibraryTags = musicLibraryTagsWidget->text().split(" ");
    auto rotelNetworkAddress = rotelNetworkAddressWidget->text();
    auto rotelNetworkPort = rotelNetworkPortWidget->value();
    auto movieLibraryExtensions = movieLibraryExtensionsWidget->text();
    auto movieDefaultAudioLanguage =
            movieDefaultAudioLanguageWidget->currentIndex() ? movieDefaultAudioLanguageWidget->currentText() : "";
    auto movieDefaultSubtitleLanguage =
            movieDefaultSubtitleLanguageWidget->currentIndex() ? movieDefaultSubtitleLanguageWidget->currentText() : "";
    auto databaseDirectory = databaseDirectoryWidget->text();
    qint64 databaseCutOff = 0;
    if (databaseCutOffCheck->isChecked()) {
        // startOfDay requires Qt 5.14 or higher.
        databaseCutOff = QDateTime(databaseCutOffDate->date()).toMSecsSinceEpoch();
    }
    QStringList movieLibraryTagAndDirectory;
    if (movieLibraryListWidget->count() > 0) {
        for (int i = 0; i < movieLibraryListWidget->count(); ++i) {
            movieLibraryTagAndDirectory.push_back(movieLibraryListWidget->item(i)->text());
        }
    }
    QList<std::pair<QString,QUrl>> streamingSites;
    if (streamingSitesListWidget->count() > 0) {
        for (int i = 0; i < streamingSitesListWidget->count(); ++i) {
            auto streamingEntry = streamingSitesListWidget->item(i)->text();
            if (!streamingEntry.isEmpty()) {
                streamingSites.push_back(splitStreamingSiteEntry(streamingEntry));
            }
        }
    }
    // Debug output
    qDebug() << "xPlayerConfigurationDialog: save: musicLibraryDirectory: " << musicLibraryDirectory;
    qDebug() << "xPlayerConfigurationDialog: save: musicLibraryExtensions: " << musicLibraryExtensions;
    qDebug() << "xPlayerConfigurationDialog: save: musicLibraryAlbumSelectors: " << musicLibraryAlbumSelectors;
    qDebug() << "xPlayerConfigurationDialog: save: musicLibraryTags: " << musicLibraryTags;
    qDebug() << "xPlayerConfigurationDialog: save: rotelEnableWidget: " << rotelEnableWidget->isEnabled();
    qDebug() << "xPlayerConfigurationDialog: save: rotelNetworkAddress: " << rotelNetworkAddress;
    qDebug() << "xPlayerConfigurationDialog: save: rotelNetworkPort: " << rotelNetworkPort;
    qDebug() << "xPlayerConfigurationDialog: save: movieLibraryDirectory: " << movieLibraryTagAndDirectory;
    qDebug() << "xPlayerConfigurationDialog: save: movieLibraryExtensions: " << movieLibraryExtensions;
    qDebug() << "xPlayerConfigurationDialog: save: movieDefaultAudioLanguage: " << movieDefaultAudioLanguage;
    qDebug() << "xPlayerConfigurationDialog: save: movieDefaultSubtitleLanguage: " << movieDefaultSubtitleLanguage;
    qDebug() << "xPlayerConfigurationDialog: save: streamingSites: " << streamingSites;
    qDebug() << "xPlayerConfigurationDialog: save: streamingSitesDefault: " << streamingSitesDefault;
    qDebug() << "xPlayerConfigurationDialog: save: databaseDirectory: " << databaseDirectory;
    qDebug() << "xPlayerConfigurationDialog: save: databaseCutOff: " << databaseCutOff;
    qDebug() << "xPlayerConfigurationDialog: save: databaseMusicOverlay: " << databaseMusicOverlayCheck->isChecked();
    qDebug() << "xPlayerConfigurationDialog: save: databaseMovieOverlay: " << databaseMovieOverlayCheck->isChecked();
    qDebug() << "xPlayerConfigurationDialog: save: databaseIgnoreUpdateErrors: " << databaseIgnoreUpdateErrorsCheck->isChecked();
    qDebug() << "xPlayerConfigurationDialog: save: websiteZoomFactor: " << websiteZoomFactors->currentIndex();
    // Save settings.
    xPlayerConfiguration::configuration()->setMusicLibraryDirectory(musicLibraryDirectory);
    xPlayerConfiguration::configuration()->setMusicLibraryExtensions(musicLibraryExtensions);
    xPlayerConfiguration::configuration()->setMusicLibraryAlbumSelectors(musicLibraryAlbumSelectors);
    xPlayerConfiguration::configuration()->setMusicLibraryTags(musicLibraryTags);
    xPlayerConfiguration::configuration()->setRotelWidget(rotelNetworkAddressWidget->isEnabled());
    xPlayerConfiguration::configuration()->setRotelNetworkAddress(rotelNetworkAddress, rotelNetworkPort);
    xPlayerConfiguration::configuration()->setMovieLibraryTagAndDirectory(movieLibraryTagAndDirectory);
    xPlayerConfiguration::configuration()->setMovieLibraryExtensions(movieLibraryExtensions);
    xPlayerConfiguration::configuration()->setMovieDefaultAudioLanguage(movieDefaultAudioLanguage);
    xPlayerConfiguration::configuration()->setMovieDefaultSubtitleLanguage(movieDefaultSubtitleLanguage);
    xPlayerConfiguration::configuration()->setStreamingSites(streamingSites);
    xPlayerConfiguration::configuration()->setStreamingSitesDefault(streamingSitesDefault);
    xPlayerConfiguration::configuration()->setDatabaseDirectory(databaseDirectory);
    xPlayerConfiguration::configuration()->setDatabaseCutOff(databaseCutOff);
    xPlayerConfiguration::configuration()->setDatabaseMusicOverlay(databaseMusicOverlayCheck->isChecked());
    xPlayerConfiguration::configuration()->setDatabaseMovieOverlay(databaseMovieOverlayCheck->isChecked());
    xPlayerConfiguration::configuration()->setDatabaseIgnoreUpdateErrors(databaseIgnoreUpdateErrorsCheck->isChecked());
    xPlayerConfiguration::configuration()->setWebsiteZoomFactorIndex(websiteZoomFactors->currentIndex());
    // End dialog.
    accept();
}

void xPlayerConfigurationDialog::toggleRotelWidget() {
    if (rotelNetworkAddressWidget->isEnabled()) {
        rotelNetworkAddressWidget->setEnabled(false);
        rotelNetworkPortWidget->setEnabled(false);
        rotelEnableWidget->setText(tr("enable"));
    } else {
        rotelNetworkAddressWidget->setEnabled(true);
        rotelNetworkPortWidget->setEnabled(true);
        rotelEnableWidget->setText(tr("disable"));
    }
}

void xPlayerConfigurationDialog::selectMovieLibrary(QListWidgetItem* item) {
    if (item) {
        auto entry = splitMovieLibraryEntry(item->text());
        movieLibraryTagWidget->setText(entry.first);
        movieLibraryDirectoryWidget->setText(entry.second);
    } else {
        movieLibraryTagWidget->clear();
        movieLibraryDirectoryWidget->clear();
    }
}

void xPlayerConfigurationDialog::openMusicLibraryDirectory() {
    QString musicLibraryDirectory =
            QFileDialog::getExistingDirectory(this, tr("Open Music Library"), musicLibraryDirectoryWidget->text(),
                                              QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if (!musicLibraryDirectory.isEmpty()) {
        musicLibraryDirectoryWidget->setText(musicLibraryDirectory);
    }
}

void xPlayerConfigurationDialog::openMovieLibraryDirectory() {
    QString newMovieLibraryDirectory =
            QFileDialog::getExistingDirectory(this, tr("Open Movie Library"), movieLibraryDirectoryWidget->text(),
                                              QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if (!newMovieLibraryDirectory.isEmpty()) {
        movieLibraryDirectoryWidget->setText(newMovieLibraryDirectory);
    }
}

void xPlayerConfigurationDialog::movieLibraryAdd() {
    auto currentTag = movieLibraryTagWidget->text();
    auto currentDirectory = movieLibraryDirectoryWidget->text();
    if ((!currentTag.isEmpty()) && (!currentDirectory.isEmpty())) {
        if (!xPlayerConfigurationDialog::isEntryInListWidget(movieLibraryListWidget, currentTag, currentDirectory)) {
            movieLibraryListWidget->addItem(QString("(%1) - %2").arg(currentTag, currentDirectory));
        }
    }
}

void xPlayerConfigurationDialog::movieLibraryRemove() {
    auto currentIndex = movieLibraryListWidget->currentRow();
    if ((currentIndex >= 0) && (currentIndex < movieLibraryListWidget->count())) {
        movieLibraryListWidget->takeItem(currentIndex);
    }
}

void xPlayerConfigurationDialog::selectStreamingSite(QListWidgetItem *item) {
    if (item) {
        auto entry = splitStreamingSiteEntry(item->text());
        streamingNameWidget->setText(entry.first);
        streamingUrlWidget->setText(entry.second.toString());
    } else {
        streamingNameWidget->clear();
        streamingUrlWidget->clear();
    }
}

void xPlayerConfigurationDialog::streamingSiteAdd() {
    auto currentName = streamingNameWidget->text();
    auto currentUrl = streamingUrlWidget->text();
    if ((!currentName.isEmpty()) && (!currentUrl.isEmpty())) {
        if (!xPlayerConfigurationDialog::isEntryInListWidget(streamingSitesListWidget, currentName, currentUrl)) {
            streamingSitesListWidget->addItem(QString("(%1) - %2").arg(currentName, currentUrl));
        }
        updateStreamingSitesDefault();
    }
}

void xPlayerConfigurationDialog::streamingSiteRemove() {
    auto currentIndex = streamingSitesListWidget->currentRow();
    if ((currentIndex >= 0) && (currentIndex < streamingSitesListWidget->count())) {
        streamingSitesListWidget->takeItem(currentIndex);
        updateStreamingSitesDefault();
        // Reset stream site default if current default is no longer in the list widget.
        if (!xPlayerConfigurationDialog::isEntryInListWidget(streamingSitesListWidget, streamingSitesDefault.first,
                                                            streamingSitesDefault.second.toString())) {
            streamingSitesDefault = std::make_pair("", QUrl(""));
        }
    }
}

void xPlayerConfigurationDialog::streamingSiteDefault() {
    auto currentIndex = streamingSitesListWidget->currentRow();
    if ((currentIndex >= 0) && (currentIndex < streamingSitesListWidget->count())) {
        streamingSitesDefault = splitStreamingSiteEntry(streamingSitesListWidget->item(currentIndex)->text());
        updateStreamingSitesDefault();
    }
}
void xPlayerConfigurationDialog::updateStreamingSitesDefault() {
    // Streaming sites list widget is sorting. Therefore we now have to update the default.
    auto streamingSitesDefaultString = QString("(%1) - %2").arg(streamingSitesDefault.first, streamingSitesDefault.second.toString());
    for (int i = 0; i < streamingSitesListWidget->count(); ++i) {
        auto streamingSitesItem = streamingSitesListWidget->item(i);
        if (streamingSitesItem->text() == streamingSitesDefaultString) {
            streamingSitesItem->setIcon(QIcon(":/images/xplay-play.svg"));
        } else {
            streamingSitesItem->setIcon(QIcon());
        }
    }
}

void xPlayerConfigurationDialog::openDatabaseDirectory() {
    QString databaseDirectory =
            QFileDialog::getExistingDirectory(this, tr("Open Database Directory"), databaseDirectoryWidget->text(),
                                              QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if (!databaseDirectory.isEmpty()) {
        databaseDirectoryWidget->setText(databaseDirectory);
    }
}

bool xPlayerConfigurationDialog::isEntryInListWidget(QListWidget* list, const QString& first, const QString& second) {
    auto entry = QString("(%1) - %2").arg(first, second);
    for (int i = 0; i < list->count(); ++i) {
        if (list->item(i)->text() == entry) {
            return true;
        }
    }
    return false;
}

std::pair<QString,QString> xPlayerConfigurationDialog::splitMovieLibraryEntry(const QString& entry) {
    QRegularExpression regExp("\\((?<tag>.*)\\) - (?<directory>.*)");
    QRegularExpressionMatch regExpMatch = regExp.match(entry);
    if (regExpMatch.hasMatch()) {
        return std::make_pair(regExpMatch.captured("tag"), regExpMatch.captured("directory"));
    } else {
        return std::make_pair("","");
    }
}

std::pair<QString,QUrl> xPlayerConfigurationDialog::splitStreamingSiteEntry(const QString& entry) {
    QRegularExpression regExp("\\((?<name>.*)\\) - (?<url>.*)");
    QRegularExpressionMatch regExpMatch = regExp.match(entry);
    if (regExpMatch.hasMatch()) {
        return std::make_pair(regExpMatch.captured("name"), QUrl(regExpMatch.captured("url")));
    } else {
        return std::make_pair("",QUrl(""));
    }
}
