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

#include <QApplication>
#include "xApplication.h"

bool handleCommandLine(QApplication& playApp, QCommandLineParser& playAppParser) {
    // Commandline handling.
    playAppParser.addOptions( {
            { {"p", "playpause"},
              QApplication::translate("xPlay", "Toggle play/pause in the current view.") },
            { {"s", "stop"},
              QApplication::translate("xPlay", "Stop the playback in current view.") },
            { {"P", "prev"},
              QApplication::translate("xPlay", "Play the previous file in the music view.") },
            { {"N", "next"},
              QApplication::translate("xPlay", "Play the next file in the music view.") },
            { {"j", "jump"},
              QApplication::translate("xPlay", "Jump relative (in ms) within the movie file."),
              QApplication::translate("xPlay", "delta") },
            { {"m", "mute"},
              QApplication::translate("xPlay", "Toggle mute the audio in the current view.") },
            { {"c", "changevolume"},
              QApplication::translate("xPlay", "Change the volume for the current view."),
              QApplication::translate("xPlay", "delta") },
            { {"F", "fullwindow"},
              QApplication::translate("xPlay", "Toggle the full window mode in the movie view.") },
            { {"e", "selectview"},
                    QApplication::translate("xPlay", R"(Select a view ("music", "movie" or "streaming").)"),
                    QApplication::translate("xPlay", "view") },
            { {"M", "muterotel"},
                    QApplication::translate("xPlay", "Toggle mute the audio for the Rotel amp.") },
            { {"C", "changerotelvolume"},
                    QApplication::translate("xPlay", "Change the volume of the Rotel amp."),
                    QApplication::translate("xPlay", "delta") },
            { {"E", "selectrotelsource"},
              QApplication::translate("xPlay", R"(Select source for Rotel amp ("aux1", "aux1", "opt1", ...).)"),
              QApplication::translate("xPlay", "source") },
            { {"x", "maximized"},
                    QApplication::translate("xPlay", "Start xplay in maximized window.") },
    } );
    playAppParser.addHelpOption();
    playAppParser.addVersionOption();
    playAppParser.process(playApp);
    if (!playAppParser.unknownOptionNames().isEmpty()) {
        // Show help if there are any unknown options.
        playAppParser.showVersion();
    }
    // Do we have any arguments.
    if (playAppParser.optionNames().isEmpty()) {
        return false;
    }
    // Exit with false for non-dbus arguments.
    if (playAppParser.isSet("maximized")) {
        return false;
    }
    // Prepare for DBus commands to be issued.
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCritical() << "Cannot connect to the D-Bus session bus.";
        return true;
    }
    QDBusInterface playInterface("org.wbcolon.xPlayer", "/", "", QDBusConnection::sessionBus());
    if (playInterface.isValid()) {
        if (playAppParser.isSet("changevolume")) {
            // Change volume.
            auto delta = playAppParser.value("changevolume").toInt();
            QDBusReply<void> reply = playInterface.call("changeVolume", delta);
            if (!reply.isValid()) {
                qCritical() << "Unable to run dbus command." << qPrintable(reply.error().message());
            }
            return true;
        } else if (playAppParser.isSet("changerotelvolume")) {
            // Change volume for Rotel amp.
            auto delta = playAppParser.value("changerotelvolume").toInt();
            QDBusReply<void> reply = playInterface.call("changeRotelVolume", delta);
            if (!reply.isValid()) {
                qCritical() << "Unable to run dbus command." << qPrintable(reply.error().message());
            }
            return true;
        } else if (playAppParser.isSet("jump")) {
            // Jump within the movie file.
            qint64 delta = static_cast<qint64>(playAppParser.value("jump").toLong());
            QDBusReply<void> reply = playInterface.call("jump", delta);
            if (!reply.isValid()) {
                qCritical() << "Unable to run dbus command." << qPrintable(reply.error().message());
            }
            return true;
        } else if (playAppParser.isSet("selectview")) {
            // Select view.
            auto view = playAppParser.value("selectview");
            QDBusReply<void> reply = playInterface.call("selectView", view);
            if (!reply.isValid()) {
                qCritical() << "Unable to run dbus command." << qPrintable(reply.error().message());
            }
            return true;
        } else if (playAppParser.isSet("selectrotelsource")) {
            // Select source for Rotel amp.
            auto source = playAppParser.value("selectrotelsource");
            QDBusReply<void> reply = playInterface.call("selectRotelSource", source);
            if (!reply.isValid()) {
                qCritical() << "Unable to run dbus command." << qPrintable(reply.error().message());
            }
            return true;
        } else {
            // All commands that do not have any arguments.
            QString command;
            if (playAppParser.isSet("playpause")) {
                command = "playPause";
            } else if (playAppParser.isSet("stop")) {
                command = "stop";
            } else if (playAppParser.isSet("prev")) {
                command = "prev";
            } else if (playAppParser.isSet("next")) {
                command = "next";
            } else if (playAppParser.isSet("mute")) {
                command = "mute";
            } else if (playAppParser.isSet("fullwindow")) {
                command = "fullWindow";
            } else if (playAppParser.isSet("muterotel")) {
                command = "muteRotel";
            } else {
                // Should not happen.
                qCritical() << "Unknown dbus command: " << playAppParser.optionNames();
                return true;
            }
            QDBusReply<void> reply = playInterface.call(command);
            if (!reply.isValid()) {
                qCritical() << "Unable to run dbus command: " << qPrintable(reply.error().message());
            }
        }
        return true;
    } else {
        // Do not start xPlay if it is not yet running and a dbus commandline was given.
        qCritical() << "Unable to connect to the dbus interface.";
        return true;
    }
}

int main(int argc, char* argv[])
{
    QApplication playApp(argc, argv);
    QApplication::setApplicationName("xPlay");
    QApplication::setApplicationVersion("0.14.1");
    QCommandLineParser playAppParser;
    if (handleCommandLine(playApp, playAppParser)) {
        // Exit if the command line was handled.
        return 0;
    }
    // Start xPlay.
    xApplication app;
    if (playAppParser.isSet("maximized")) {
        app.showMaximized();
    } else {
        app.resize(1920, 1080);
        app.show();
    }
    return playApp.exec(); // NOLINT
}
