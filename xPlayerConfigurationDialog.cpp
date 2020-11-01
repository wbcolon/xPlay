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
#include <QPushButton>
#include <QLabel>
#include <QRegularExpression>
#include <QDebug>

xPlayerConfigurationDialog::xPlayerConfigurationDialog(QWidget* parent, Qt::WindowFlags flags):
        QDialog(parent, flags) {

    auto configurationLayout = new QGridLayout(this);
    auto rotelBox = new QGroupBox(tr("Rotel Configuration"), this);
    auto musicLibraryBox = new QGroupBox(tr("Music Library Configuration"), this);
    auto movieLibraryBox = new QGroupBox(tr("Movie Library Configuration"), this);
    auto configurationButtons = new QDialogButtonBox(Qt::Horizontal, this);
    configurationButtons->addButton(QDialogButtonBox::Save);
    configurationButtons->addButton(QDialogButtonBox::Reset);
    configurationButtons->addButton(QDialogButtonBox::Cancel);

    // Setup movie library setup, with tags, directories and extensions.
    auto movieLibraryLayout = new QGridLayout(movieLibraryBox);
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
    // Setup music library with directory and extensions.
    auto musicLibraryLayout = new QGridLayout(musicLibraryBox);
    auto musicLibraryDirectoryLabel = new QLabel(tr("Directory"), musicLibraryBox);
    musicLibraryDirectoryWidget = new QLineEdit(musicLibraryBox);
    auto musicLibraryDirectoryOpenButton = new QPushButton(tr("..."), musicLibraryBox);
    auto musicLibraryExtensionsLabel = new QLabel(tr("Extensions"), musicLibraryBox);
    musicLibraryExtensionsWidget = new QLineEdit(musicLibraryBox);
    musicLibraryLayout->addWidget(musicLibraryDirectoryLabel, 0, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryDirectoryWidget, 1, 0, 1, 4);
    musicLibraryLayout->addWidget(musicLibraryDirectoryOpenButton, 1, 4);
    musicLibraryLayout->addWidget(musicLibraryExtensionsLabel, 2, 0, 1, 5);
    musicLibraryLayout->addWidget(musicLibraryExtensionsWidget, 3, 0, 1, 5);
    // Setup rotel amp with network address and port.
    auto rotelLayout = new QGridLayout(rotelBox);
    auto rotelNetworkAddressLabel = new QLabel(tr("Network Address"), rotelBox);
    auto rotelNetworkPortLabel = new QLabel(tr("Port"), rotelBox);
    rotelNetworkAddressWidget = new QLineEdit(rotelBox);
    rotelNetworkPortWidget = new QSpinBox(rotelBox);
    rotelNetworkPortWidget->setRange(0, 10000);
    rotelLayout->addWidget(rotelNetworkAddressLabel, 0, 0);
    rotelLayout->addWidget(rotelNetworkAddressWidget, 1, 0);
    rotelLayout->addWidget(rotelNetworkPortLabel, 2, 0);
    rotelLayout->addWidget(rotelNetworkPortWidget, 3, 0);
    // Configuration layout.
    configurationLayout->addWidget(musicLibraryBox, 0, 0, 2, 4);
    configurationLayout->addWidget(rotelBox, 2, 0, 2, 4);
    configurationLayout->addWidget(movieLibraryBox, 0, 4, 4, 4);
    configurationLayout->setRowMinimumHeight(4, 50);
    configurationLayout->setRowStretch(4, 0);
    configurationLayout->addWidget(configurationButtons, 5, 4, 1, 4);
    // Connect dialog buttons.
    connect(musicLibraryDirectoryOpenButton, &QPushButton::pressed, this, &xPlayerConfigurationDialog::openMusicLibraryDirectory);
    connect(movieLibraryDirectoryOpenButton, &QPushButton::pressed, this, &xPlayerConfigurationDialog::openMovieLibraryDirectory);
    // Connect movie library.
    connect(movieLibraryButtons->button(QDialogButtonBox::Apply), &QPushButton::pressed, this, &xPlayerConfigurationDialog::movieLibraryAdd);
    connect(movieLibraryButtons->button(QDialogButtonBox::Discard), &QPushButton::pressed, this, &xPlayerConfigurationDialog::movieLibraryRemove);
    connect(movieLibraryListWidget, &QListWidget::currentItemChanged, this, &xPlayerConfigurationDialog::selectMovieLibrary);
    // Connect dialog buttons.
    connect(configurationButtons->button(QDialogButtonBox::Save), &QPushButton::pressed, this, &xPlayerConfigurationDialog::saveSettings);
    connect(configurationButtons->button(QDialogButtonBox::Reset), &QPushButton::pressed, this, &xPlayerConfigurationDialog::loadSettings);
    connect(configurationButtons->button(QDialogButtonBox::Cancel), &QPushButton::pressed, this, &QDialog::reject);
    // Load and resize.
    loadSettings();
    setMinimumWidth(sizeHint().height()*2);
    setMinimumHeight(sizeHint().height());
}

void xPlayerConfigurationDialog::loadSettings() {
    auto musicLibraryDirectory = xPlayerConfiguration::configuration()->getMusicLibraryDirectory();
    auto musicLibraryExtensions = xPlayerConfiguration::configuration()->getMusicLibraryExtensions();
    auto [rotelNetworkAddress, rotelNetworkPort] = xPlayerConfiguration::configuration()->getRotelNetworkAddress();
    auto movieLibraryTagAndDirectory = xPlayerConfiguration::configuration()->getMovieLibraryTagAndDirectory();
    auto movieLibraryExtensions = xPlayerConfiguration::configuration()->getMovieLibraryExtensions();
    // Update the configuration dialog UI.
    musicLibraryDirectoryWidget->setText(musicLibraryDirectory);
    musicLibraryExtensionsWidget->setText(musicLibraryExtensions);
    rotelNetworkAddressWidget->setText(rotelNetworkAddress);
    rotelNetworkPortWidget->setValue(rotelNetworkPort);
    movieLibraryExtensionsWidget->setText(movieLibraryExtensions);
    movieLibraryListWidget->clear();
    if (movieLibraryTagAndDirectory.count() > 0) {
        for (const auto& entry : movieLibraryTagAndDirectory) {
            auto splitEntry = splitMovieLibraryEntry(entry);
            movieLibraryListWidget->addItem(QString("(%1) - %2").arg(splitEntry.first).arg(splitEntry.second));
        }
    }
}

void xPlayerConfigurationDialog::saveSettings() {
    // Read setting entries.
    auto musicLibraryDirectory = musicLibraryDirectoryWidget->text();
    auto musicLibraryExtensions = musicLibraryExtensionsWidget->text();
    auto rotelNetworkAddress = rotelNetworkAddressWidget->text();
    auto rotelNetworkPort = rotelNetworkPortWidget->value();
    auto movieLibraryExtensions = movieLibraryExtensionsWidget->text();
    QStringList movieLibraryTagAndDirectory;
    if (movieLibraryListWidget->count() > 0) {
        for (int i = 0; i < movieLibraryListWidget->count(); ++i) {
            movieLibraryTagAndDirectory.push_back(movieLibraryListWidget->item(i)->text());
        }
    }
    // Debug output        movieLibraryTagWidget->clear();.
    qDebug() << "xPlayerConfigurationDialog: save: musicLibraryDirectory: " << musicLibraryDirectory;
    qDebug() << "xPlayerConfigurationDialog: save: musicLibraryExtensions: " << musicLibraryExtensions;
    qDebug() << "xPlayerConfigurationDialog: save: rotelNetworkAddress: " << rotelNetworkAddress;
    qDebug() << "xPlayerConfigurationDialog: save: rotelNetworkPort: " << rotelNetworkPort;
    qDebug() << "xPlayerConfigurationDialog: save: movieLibraryDirectory: " << movieLibraryTagAndDirectory;
    qDebug() << "xPlayerConfigurationDialog: save: movieLibraryExtensions: " << movieLibraryExtensions;
    // Save settings.
    xPlayerConfiguration::configuration()->setMusicLibraryDirectory(musicLibraryDirectory);
    xPlayerConfiguration::configuration()->setMusicLibraryExtensions(musicLibraryExtensions);
    xPlayerConfiguration::configuration()->setRotelNetworkAddress(rotelNetworkAddress, rotelNetworkPort);
    xPlayerConfiguration::configuration()->setMovieLibraryTagAndDirectory(movieLibraryTagAndDirectory);
    xPlayerConfiguration::configuration()->setMovieLibraryExtensions(movieLibraryExtensions);
    // End dialog.
    accept();
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
        movieLibraryListWidget->addItem(QString("(%1) - %2").arg(currentTag).arg(currentDirectory));
    }
}

void xPlayerConfigurationDialog::movieLibraryRemove() {
    auto currentIndex = movieLibraryListWidget->currentRow();
    if ((currentIndex >= 0) && (currentIndex < movieLibraryListWidget->count())) {
        movieLibraryListWidget->takeItem(currentIndex);
    }
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
