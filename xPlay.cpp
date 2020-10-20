#include <QApplication>

#include "xApplication.h"
#include "xMusicLibrary.h"

int main(int argc, char* argv[])
{
    QApplication playApp(argc, argv);
    xApplication app;
    app.show();
    return playApp.exec();
}
