#ifndef __XAPPLICATION_H__
#define __XAPPLICATION_H__

#include <QSettings>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "xMusicLibrary.h"
#include "xMusicPlayer.h"
#include "xMainWidget.h"

class xApplication:public QMainWindow {
public:
    xApplication(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xApplication() = default;

private slots:
    void openMusicLibrary();

private:
    void setMusicLibraryDirectory(const QString& directory);
    void createMenus();

    QSettings* settings;
    QString musicLibraryDirectory;
    xMusicLibrary* musicLibrary;
    xMusicPlayer* musicPlayer;
    xMainWidget* mainWidget;
};

#endif