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

#ifndef __XPLAY_XPLAYERDBUS_H__
#define __XPLAY_XPLAYERDBUS_H__

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QtDBus>

class xPlayerDBus:public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.wbcolon.xPlayer")
    Q_CLASSINFO("D-Bus Introspection",
                "<interface name=\"org.wbcolon.xPlayer\">\n"
                "    <method name=\"playPause\"/>\n"
                "    <method name=\"stop\"/>\n"
                "    <method name=\"prev\"/>\n"
                "    <method name=\"next\"/>\n"
                "    <method name=\"jump\">\n"
                "      <arg direction=\"in\" type=\"x\" name=\"delta\"/>\n"
                "    </method>\n"
                "    <method name=\"fullWindow\"/>\n"
                "    <method name=\"scaleAndCrop\"/>\n"
                "    <method name=\"mute\"/>\n"
                "    <method name=\"changeVolume\">\n"
                "      <arg direction=\"in\" type=\"i\" name=\"delta\"/>\n"
                "    </method>\n"
                "    <method name=\"selectView\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"view\"/>\n"
                "    </method>\n"
                "    <method name=\"muteRotel\"/>\n"
                "    <method name=\"changeRotelVolume\">\n"
                "      <arg direction=\"in\" type=\"i\" name=\"delta\"/>\n"
                "    </method>\n"
                "    <method name=\"selectRotelSource\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"view\"/>\n"
                "    </method>\n"
                "</interface>\n"
    )

public:
    xPlayerDBus(QObject* obj);
    ~xPlayerDBus() = default;

public Q_SLOTS:
    /**
     * Play/pause toggle for the music or movie view.
     */
    void playPause();
    /**
     * Stop playback for the music or movie view.
     */
    void stop();
    /**
     * Play previous file in the music view.
     */
    void prev();
    /**
     * Play next file in the music view.
     */
    void next();
    /**
     * Jump relativ within the currently played movie.
     *
     * @param delta the delta to the current position in ms.
     */
    void jump(qint64 delta);
    /**
     * Toggle the full window mode in movie view.
     */
    void fullWindow();
    /**
     * Toggle the scale and crop mode in the movie view.
     */
    void scaleAndCrop();
    /**
     * Toggle mute of the playback in the current view.
     */
    void mute();
    /**
     * Change the volume in the music or movie view.
     *
     * @param delta the delta to the current volume.
     */
    void changeVolume(int delta);
    /**
     * Select the current view.
     *
     * @param view the new view as string.
     */
    void selectView(const QString& view);
    /**
     * Toggle mute for the connected Rotel amp.
     */
    void muteRotel();
    /**
     * Change the volume for the connected Rotel amp.
     *
     * @param delta the delta to the current volume.
     */
    void changeRotelVolume(int delta);
    /**
     * Select a new source for the connected Rotel amp.
     *
     * @param source the new source as string.
     */
    void selectRotelSource(const QString& source);

signals:
    /**
     * Emitted if playPause dbus function is called.
     */
    void dbus_playPause();
    /**
     * Emitted if stop dbus function is called.
     */
    void dbus_stop();
    /**
     * Emitted if prev dbus function is called.
     */
    void dbus_prev();
    /**
     * Emitted if next dbus function is called.
     */
    void dbus_next();
    /**
     * Emitted if jump dbus function is called.
     *
     * @param delta the delta for the current position in ms.
     */
    void dbus_jump(qint64 delta);
    /**
     * Emitted if fullWindow dbus function is called.
     */
    void dbus_fullWindow();
    /**
     * Emitted if scaleAndCrop dbus function is called.
     */
    void dbus_scaleAndCrop();
    /**
     * Emitted if mute dbus function is called.
     */
    void dbus_mute();
    /**
     * Emitted if changeVolume dbus function is called.
     *
     * @param delta the delta to the current volume.
     */
    void dbus_changeVolume(int delta);
    /**
     * Emitted if the selectView dbus function is called.
     *
     * @param view the new view as string.
     */
    void dbus_selectView(const QString& view);
    /**
     * Emitted if the muteRoel dbus function is called.
     */
    void dbus_muteRotel();
    /**
     * Emitted if the changeRotelVolume dbus function is called.
     *
     * @param delta the delta to the current volume.
     */
    void dbus_changeRotelVolume(int delta);
    /**
     * Emitted if the selectRotelSource dbus function is called.
     *
     * @param source the new source as string.
     */
    void dbus_selectRotelSource(const QString& source);
};

#endif
