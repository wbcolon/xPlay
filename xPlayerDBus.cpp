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

#include "xPlayerDBus.h"

xPlayerDBus::xPlayerDBus(QObject* obj):QDBusAbstractAdaptor(obj) {
    QDBusConnection::sessionBus().registerObject("/", obj);
    if (!QDBusConnection::sessionBus().registerService("org.wbcolon.xPlayer")) {
        qCritical() << "DBus error: " << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}

void xPlayerDBus::playPause() {
    emit dbus_playPause();
}

void xPlayerDBus::stop() {
    emit dbus_stop();
}

void xPlayerDBus::prev() {
    emit dbus_prev();
}

void xPlayerDBus::next() {
    emit dbus_next();
}

void xPlayerDBus::jump(qint64 delta) {
    emit dbus_jump(delta);
}

void xPlayerDBus::fullWindow() {
    emit dbus_fullWindow();
}

void xPlayerDBus::scaleAndCrop() {
    emit dbus_scaleAndCrop();
}

void xPlayerDBus::mute() {
    emit dbus_mute();
}
void xPlayerDBus::changeVolume(int delta) {
    emit dbus_changeVolume(delta);
}

void xPlayerDBus::selectView(const QString& view) {
    emit dbus_selectView(view);
}
void xPlayerDBus::muteRotel() {
    emit dbus_muteRotel();
}
void xPlayerDBus::changeRotelVolume(int delta) {
    emit dbus_changeRotelVolume(delta);
}

void xPlayerDBus::selectRotelSource(const QString& source) {
    emit dbus_selectRotelSource(source);
}
