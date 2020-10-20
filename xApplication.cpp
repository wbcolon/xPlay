#include <QMenuBar>

#include "xApplication.h"
#include "xMusicPlayerQt.h"
#include "xMusicPlayerPhonon.h"

xApplication::xApplication(QWidget* parent, Qt::WindowFlags flags):
        QMainWindow(parent, flags) {
    // Setup music library.
    musicLibrary = new xMusicLibrary(this);
    musicPlayer = new xMusicPlayerPhonon(this);
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
    // Create Application menus.
    createMenus();
}

void xApplication::setMusicLibraryDirectory(const QString& directory) {
    musicLibraryDirectory=directory;
    musicLibrary->setBaseDirectory(std::filesystem::path(musicLibraryDirectory.toStdString()));
    musicPlayer->setBaseDirectory(directory);
}

void xApplication::createMenus() {
    auto fileMenuExitAction = new QAction(tr("&Exit"), this);
    connect(fileMenuExitAction, &QAction::triggered, this, &xApplication::close);

    auto fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(fileMenuExitAction);
}
