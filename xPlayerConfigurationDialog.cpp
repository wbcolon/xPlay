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

#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRegularExpression>
#include <QDebug>

xPlayerConfigurationDialog::xPlayerConfigurationDialog(QWidget* parent, Qt::WindowFlags flags):
        QDialog(parent, flags) {
    auto configurationLayout = new QGridLayout(this);
    auto rotelBox = new QGroupBox(tr("Rotel Configuration"), this);
    auto databaseBox = new QGroupBox(tr("Database Configuration"), this);
    auto musicLibraryBox = new QGroupBox(tr("Music Library Configuration"), this);
    auto movieLibraryBox = new QGroupBox(tr("Movie Library Configuration"), this);
    auto streamingSitesBox = new QGroupBox(tr("Streaming Sites Configuration"), this);
    auto configurationButtons = new QDialogButtonBox(Qt::Horizontal, this);
    configurationButtons->addButton(QDialogButtonBox::Save);
    configurationButtons->addButton(QDialogButtonBox::Reset);
    configurationButtons->addButton(QDialogButtonBox::Cancel);

    // Setup movie library setup, with tags, directories and extensions.
    auto movieLibraryLayout = new QGridLayout();
    auto movieLibraryTagLabel = new QLabel(tr("Tag"), movieLibraryBox);
    movieLibraryTagWidget = new QLineEdit(movieLibraryBox);
    auto movieLibraryDirectoryLabel = new QLabel(tr("Directory"), movieLibraryBox);
    movieLibraryDirectoryWidget = new QLineEdit(movieLibraryBox);
    auto movieLibraryDirectoryOpenButton = new QPushButton(tr("..."), movieLibraryBox);
    auto movieLibraryButtons = new QDialogButtonBox(Qt::Horizontal, movieLibraryBox);
    movieLibraryButtons->addButton(QDialogButtonBox::Apply);
    movieLibraryButtons->addButton(QDialogButtonBox::Discard);
    movieLibraryListWidget = new QListWidget(movieLibraryBox);
    movieLibraryListWidget->setSortingEnabled(true);
    auto movieLibraryExtensionsLabel = new QLabel(tr("Extensions"), movieLibraryBox);
    movieLibraryExtensionsWidget = new QLineEdit(movieLibraryBox);
    movieLibraryLayout->addWidget(movieLibraryTagLabel, 0, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryTagWidget, 1, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryDirectoryLabel, 2, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryDirectoryWidget, 3, 0, 1, 4);
    movieLibraryLayout->addWidget(movieLibraryDirectoryOpenButton, 3, 4);
    movieLibraryLayout->addWidget(movieLibraryListWidget, 4, 0, 3, 5);
    movieLibraryLayout->addWidget(movieLibraryButtons, 7, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryExtensionsLabel, 8, 0, 1, 5);
    movieLibraryLayout->addWidget(movieLibraryExtensionsWidget, 9, 0, 1, 5);
    movieLibraryBox->setLayout(movieLibraryLayout);
    // Setup music library with directory and extensions.
    auto musicLibraryLayout = new QGridLayout();
    auto musicLibraryDirectoryLabel = new QLabel(tr("Directory"), musicLibraryBox);
    musicLibraryDirectoryWidget = new QLineEdit(musicLibraryBox);
    auto musicLibraryDirectoryOpenButton = new QPushButton(tr("..."), musicLibraryBox);
    auto musicLibraryExtensionsLabel = new QLabel(tr("Extensions"), musicLibraryBox);
    musicLibraryExtensionsWidget = new QLineEdit(musicLibraryBox);
    auto musicLibraryAlbumSelectorsLabel = new QLabel(tr("Album Selectors"), musicLibraryBox);
    musicLibraryAlbumSelectorsWidget = new QLineEdit(musicLibraryBox);
    musicLibraryLayout->addWidget(musicLibraryDirectoryLabel, 0, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryDirectoryWidget, 1, 0, 1, 4);
    musicLibraryLayout->addWidget(musicLibraryDirectoryOpenButton, 1, 4);
    musicLibraryLayout->addWidget(musicLibraryExtensionsLabel, 2, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryExtensionsWidget, 3, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryAlbumSelectorsLabel, 4, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryAlbumSelectorsWidget, 5, 0, 1, 5);
    musicLibraryBox->setLayout(musicLibraryLayout);
    // Setup rotel amp with network address and port.
    auto rotelLayout = new QGridLayout();
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
    // Setup streaming sites with URL and short name.
    auto streamingSitesLayout = new QGridLayout();
    auto streamingNameLabel = new QLabel(tr("Name"), streamingSitesBox);
    streamingNameWidget = new QLineEdit(streamingSitesBox);
    auto streamingUrlLabel = new QLabel(tr("Url"), streamingSitesBox);
    streamingUrlWidget = new QLineEdit(streamingSitesBox);
    auto streamingSitesButtons = new QDialogButtonBox(Qt::Horizontal, streamingSitesBox);
    streamingSitesButtons->addButton(QDialogButtonBox::Apply);
    streamingSitesButtons->addButton(QDialogButtonBox::Discard);
    auto streamingSitesDefaultButton = streamingSitesButtons->addButton(tr("Default"), QDialogButtonBox::ResetRole);
    streamingSitesListWidget = new QListWidget(movieLibraryBox);
    streamingSitesListWidget->setSortingEnabled(true);
    streamingSitesLayout->addWidget(streamingNameLabel, 0, 0, 1, 2);
    streamingSitesLayout->addWidget(streamingNameWidget, 1, 0, 1, 2);
    streamingSitesLayout->addWidget(streamingUrlLabel, 1, 2, 1, 3);
    streamingSitesLayout->addWidget(streamingUrlWidget, 1, 2, 1, 3);
    streamingSitesLayout->addWidget(streamingSitesListWidget, 2, 0, 3, 5);
    streamingSitesLayout->addWidget(streamingSitesButtons, 5, 0, 1, 5);
    streamingSitesBox->setLayout(streamingSitesLayout);
    // Setup database configuration.
    auto databaseLayout = new QGridLayout();
    databaseMusicOverlayCheck = new QCheckBox(tr("Enable database overlay for music library"),databaseBox);
    databaseMovieOverlayCheck = new QCheckBox(tr("Enable database overlay for movie library"),databaseBox);
    databaseCutOffDate = new QDateEdit(databaseBox);
    databaseCutOffDate->setDisplayFormat("dd MMMM yyyy");
    databaseCutOffCheck = new QCheckBox(tr("Use cut-off date"), databaseBox);
    databaseLayout->addWidget(databaseMusicOverlayCheck, 0, 0, 1, 2);
    databaseLayout->addWidget(databaseMovieOverlayCheck, 1, 0, 1, 2);
    databaseLayout->addWidget(databaseCutOffCheck, 2, 0, 1, 1);
    databaseLayout->addWidget(databaseCutOffDate, 2, 1, 1, 1);
    databaseBox->setLayout(databaseLayout);
    // Configuration layout.
    configurationLayout->addWidget(musicLibraryBox, 0, 0, 2, 4);
    configurationLayout->addWidget(movieLibraryBox, 2, 0, 6, 4);
    configurationLayout->addWidget(streamingSitesBox, 0, 4, 4, 4);
    configurationLayout->addWidget(databaseBox, 4, 4, 2, 4);
    configurationLayout->addWidget(rotelBox, 6, 4, 2, 4);
    configurationLayout->setRowMinimumHeight(8, 50);
    configurationLayout->setRowStretch(8, 0);
    configurationLayout->addWidget(configurationButtons, 9, 4, 1, 4);
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
    // Connect database check boxes.
    connect(databaseCutOffCheck, &QCheckBox::clicked, databaseCutOffDate, &QDateEdit::setEnabled);
    // Connect dialog buttons.
    connect(configurationButtons->button(QDialogButtonBox::Save), &QPushButton::pressed, this, &xPlayerConfigurationDialog::saveSettings);
    connect(configurationButtons->button(QDialogButtonBox::Reset), &QPushButton::pressed, this, &xPlayerConfigurationDialog::loadSettings);
    connect(configurationButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &QDialog::reject);
    // Load and resize.
    loadSettings();
    setMinimumWidth(static_cast<int>(sizeHint().height()*1.7));
    setMinimumHeight(sizeHint().height());
}

void xPlayerConfigurationDialog::loadSettings() {
    auto musicLibraryDirectory = xPlayerConfiguration::configuration()->getMusicLibraryDirectory();
    auto musicLibraryExtensions = xPlayerConfiguration::configuration()->getMusicLibraryExtensions();
    auto musicLibraryAlbumSelectors = xPlayerConfiguration::configuration()->getMusicLibraryAlbumSelectors();
    auto rotelWidget = xPlayerConfiguration::configuration()->rotelWidget();
    auto [rotelNetworkAddress, rotelNetworkPort] = xPlayerConfiguration::configuration()->getRotelNetworkAddress();
    auto movieLibraryTagAndDirectory = xPlayerConfiguration::configuration()->getMovieLibraryTagAndDirectory();
    auto movieLibraryExtensions = xPlayerConfiguration::configuration()->getMovieLibraryExtensions();
    auto databaseCutOff = xPlayerConfiguration::configuration()->getDatabaseCutOff();
    auto streamingSites = xPlayerConfiguration::configuration()->getStreamingSites();
    streamingSitesDefault = xPlayerConfiguration::configuration()->getStreamingSitesDefault();
    // Update the configuration dialog UI.
    musicLibraryDirectoryWidget->setText(musicLibraryDirectory);
    musicLibraryExtensionsWidget->setText(musicLibraryExtensions);
    musicLibraryAlbumSelectorsWidget->setText(musicLibraryAlbumSelectors);
    rotelNetworkAddressWidget->setText(rotelNetworkAddress);
    rotelNetworkPortWidget->setValue(rotelNetworkPort);
    if (!rotelWidget) {
        rotelNetworkAddressWidget->setEnabled(false);
        rotelNetworkPortWidget->setEnabled(false);
        rotelEnableWidget->setText(tr("enable"));
    }
    databaseMusicOverlayCheck->setChecked(xPlayerConfiguration::configuration()->getDatabaseMusicOverlay());
    databaseMovieOverlayCheck->setChecked(xPlayerConfiguration::configuration()->getDatabaseMovieOverlay());
    if (databaseCutOff) {
        databaseCutOffCheck->setChecked(true);
        databaseCutOffDate->setDate(QDateTime::fromMSecsSinceEpoch(databaseCutOff).date());
    } else {
        databaseCutOffCheck->setChecked(false);
        databaseCutOffDate->setEnabled(false);
    }
    movieLibraryExtensionsWidget->setText(movieLibraryExtensions);
    movieLibraryListWidget->clear();
    if (movieLibraryTagAndDirectory.count() > 0) {
        for (const auto& entry : movieLibraryTagAndDirectory) {
            auto splitEntry = splitMovieLibraryEntry(entry);
            movieLibraryListWidget->addItem(QString("(%1) - %2").arg(splitEntry.first).arg(splitEntry.second));
        }
    }
    streamingSitesListWidget->clear();
    if (!streamingSites.isEmpty()) {
        for (const auto& entry : streamingSites) {
            streamingSitesListWidget->addItem(QString("(%1) - %2").arg(entry.first).arg(entry.second.toString()));
        }
    }
    updateStreamingSitesDefault();
}

void xPlayerConfigurationDialog::saveSettings() {
    // Read setting entries.
    auto musicLibraryDirectory = musicLibraryDirectoryWidget->text();
    auto musicLibraryExtensions = musicLibraryExtensionsWidget->text();
    auto musicLibraryAlbumSelectors = musicLibraryAlbumSelectorsWidget->text();
    auto rotelNetworkAddress = rotelNetworkAddressWidget->text();
    auto rotelNetworkPort = rotelNetworkPortWidget->value();
    auto movieLibraryExtensions = movieLibraryExtensionsWidget->text();
    quint64 databaseCutOff = 0;
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
    qDebug() << "xPlayerConfigurationDialog: save: rotelEnableWidget: " << rotelEnableWidget->isEnabled();
    qDebug() << "xPlayerConfigurationDialog: save: rotelNetworkAddress: " << rotelNetworkAddress;
    qDebug() << "xPlayerConfigurationDialog: save: rotelNetworkPort: " << rotelNetworkPort;
    qDebug() << "xPlayerConfigurationDialog: save: movieLibraryDirectory: " << movieLibraryTagAndDirectory;
    qDebug() << "xPlayerConfigurationDialog: save: movieLibraryExtensions: " << movieLibraryExtensions;
    qDebug() << "xPlayerConfigurationDialog: save: streamingSites: " << streamingSites;
    qDebug() << "xPlayerConfigurationDialog: save: streamingSitesDefault: " << streamingSitesDefault;
    qDebug() << "xPlayerConfigurationDialog: save: databaseCutOff: " << databaseCutOff;
    qDebug() << "xPlayerConfigurationDialog: save: databaseMusicOverlay: " << databaseMusicOverlayCheck->isChecked();
    qDebug() << "xPlayerConfigurationDialog: save: databaseMovieOverlay: " << databaseMovieOverlayCheck->isChecked();
    // Save settings.
    xPlayerConfiguration::configuration()->setMusicLibraryDirectory(musicLibraryDirectory);
    xPlayerConfiguration::configuration()->setMusicLibraryExtensions(musicLibraryExtensions);
    xPlayerConfiguration::configuration()->setMusicLibraryAlbumSelectors(musicLibraryAlbumSelectors);
    xPlayerConfiguration::configuration()->setRotelWidget(rotelNetworkAddressWidget->isEnabled());
    xPlayerConfiguration::configuration()->setRotelNetworkAddress(rotelNetworkAddress, rotelNetworkPort);
    xPlayerConfiguration::configuration()->setMovieLibraryTagAndDirectory(movieLibraryTagAndDirectory);
    xPlayerConfiguration::configuration()->setMovieLibraryExtensions(movieLibraryExtensions);
    xPlayerConfiguration::configuration()->setStreamingSites(streamingSites);
    xPlayerConfiguration::configuration()->setStreamingSitesDefault(streamingSitesDefault);
    xPlayerConfiguration::configuration()->setDatabaseCutOff(databaseCutOff);
    xPlayerConfiguration::configuration()->setDatabaseMusicOverlay(databaseMusicOverlayCheck->isChecked());
    xPlayerConfiguration::configuration()->setDatabaseMovieOverlay(databaseMovieOverlayCheck->isChecked());
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
            movieLibraryListWidget->addItem(QString("(%1) - %2").arg(currentTag).arg(currentDirectory));
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
            streamingSitesListWidget->addItem(QString("(%1) - %2").arg(currentName).arg(currentUrl));
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
    auto streamingSitesDefaultString = QString("(%1) - %2").arg(streamingSitesDefault.first).arg(streamingSitesDefault.second.toString());
    for (int i = 0; i < streamingSitesListWidget->count(); ++i) {
        auto streamingSitesItem = streamingSitesListWidget->item(i);
        if (streamingSitesItem->text() == streamingSitesDefaultString) {
            streamingSitesItem->setIcon(QIcon(":/images/xplay-play.svg"));
        } else {
            streamingSitesItem->setIcon(QIcon());
        }
    }
}

bool xPlayerConfigurationDialog::isEntryInListWidget(QListWidget* list, const QString& first, const QString& second) {
    auto entry = QString("(%1) - %2").arg(first).arg(second);
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
