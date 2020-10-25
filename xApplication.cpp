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
#include <QMenuBar>
#include <QFileDialog>
#include <QInputDialog>

#include "xApplication.h"
#include "xPlayConfig.h"

const QString ApplicationName = "xPlay";
const QString OrganisationName = "wbcolon";

xApplication::xApplication(QWidget* parent, Qt::WindowFlags flags):
        QMainWindow(parent, flags) {
    // Setup music library.
    musicLibrary = new xMusicLibrary(this);
    musicPlayer = new xMusicPlayerX(this);
    // Setup player and main widget
    mainWidget = new xMainWidget(musicPlayer, this);
    // Use main widget as central application widget.
    setCentralWidget(mainWidget);
    // Connect music library with main widget.
    // Commands for the music library.
    connect(mainWidget, &xMainWidget::scanForArtist, musicLibrary, &xMusicLibrary::scanForArtist);
    connect(mainWidget, &xMainWidget::scanForArtistAndAlbum, musicLibrary, &xMusicLibrary::scanForArtistAndAlbum);
    // Results back to the main widget.
    connect(musicLibrary, &xMusicLibrary::scannedArtists, mainWidget, &xMainWidget::scannedArtists);
    connect(musicLibrary, &xMusicLibrary::scannedAlbums, mainWidget, &xMainWidget::scannedAlbums);
    connect(musicLibrary, &xMusicLibrary::scannedTracks, mainWidget, &xMainWidget::scannedTracks);
    // Do not main widget to music player here. It is done in the main widget

    // Read Settings
    settings = new QSettings(OrganisationName, ApplicationName);
    setMusicLibraryDirectory(settings->value("xPlay/MusicLibraryDirectory", "").toString());
    auto rotelAddress = settings->value("xPlay/RotelNetworkAddress", "").toString();
    auto rotelPort = settings->value("xPlay/RotelNetworkPort", "").toInt();
    mainWidget->connectRotel(rotelAddress, rotelPort);
    // Create Application menus.
    createMenus();
}
void xApplication::setMusicLibraryDirectory(const QString& directory) {
    musicLibraryDirectory = directory;
    musicLibrary->setBaseDirectory(std::filesystem::path(musicLibraryDirectory.toStdString()));
    musicPlayer->setBaseDirectory(musicLibraryDirectory);
    qInfo() << "Update music library path to " << musicLibraryDirectory;
}

void xApplication::createMenus() {
    auto fileMenuSelectAction = new QAction("&Select Music Library", this);
    auto fileMenuRotelAction = new QAction("&Configure Rotel", this);
    auto fileMenuExitAction = new QAction(tr("&Exit"), this);

    connect(fileMenuSelectAction, &QAction::triggered, this, &xApplication::selectMusicLibrary);
    connect(fileMenuRotelAction, &QAction::triggered, this, &xApplication::configureRotelAmp);
    connect(fileMenuExitAction, &QAction::triggered, this, &xApplication::close);

    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(fileMenuSelectAction);
    fileMenu->addAction(fileMenuRotelAction);
    fileMenu->addSeparator();
    fileMenu->addAction(fileMenuExitAction);
}

void xApplication::selectMusicLibrary() {
    QString newMusicLibraryDirectory =
            QFileDialog::getExistingDirectory(this, tr("Open Music Library"), musicLibraryDirectory,
                                              QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    if (!newMusicLibraryDirectory.isEmpty()) {
        settings->setValue("xPlay/MusicLibraryDirectory", newMusicLibraryDirectory);
        setMusicLibraryDirectory(newMusicLibraryDirectory);
    }
}

void xApplication::configureRotelAmp() {
    bool okPressed = false;
    auto oldAddress = settings->value("xPlay/RotelNetworkAddress", "").toString();
    auto oldPort = settings->value("xPlay/RotelNetworkPort", "").toInt();
    QString newConnection =
            QInputDialog::getText(this, tr("Configure Rotel"), tr("Rotel network address (address:port)"),
                                  QLineEdit::Normal, QString("%1:%2").arg(oldAddress).arg(oldPort), &okPressed);
    if (okPressed) {
        auto newConnectionSplit = newConnection.split(":");
        auto newAddress = newConnectionSplit.value(0);
        auto newPort = newConnectionSplit.value(1).toInt();
        settings->setValue("xPlay/RotelNetworkAddress", newAddress);
        settings->setValue("xPlay/RotelNetworkPort", newPort);
        qDebug() << "Rotel Connection: " << newAddress << ":" << newPort;
        mainWidget->connectRotel(newAddress, newPort);
    }
}
