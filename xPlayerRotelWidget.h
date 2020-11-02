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
#ifndef __XPLAYERROTELWIDGET_H__
#define __XPLAYERROTELWIDGET_H__

#include "xPlayerVolumeWidget.h"

#include <QComboBox>
#include <QCheckBox>
#include <QTcpSocket>
#include <QLabel>
#include <QWidget>
#include <QString>

class xPlayerRotelControls:public QObject {
    Q_OBJECT

public:
    /**
     * Return the Rotel amp controls.
     *
     * @return pointer to a singleton of the rotel amp controls.
     */
    static xPlayerRotelControls* controls();
    /**
     * Connect controls via network to the Rotel amp.
     *
     * @param address the address of the Rotel amp as string (IP or DNS name).
     * @param port the port the controls connect to as integer.
     * @param wait if true then wait until the socket is connected.
     */
    void connect(const QString& address, int port, bool wait=false);
    /**
     * Send set volume command to the Rotel amp.
     *
     * @param vol the new volume as integer.
     */
    void setVolume(int vol);
    /**
     * Send set source command to the Rotel amp.
     *
     * @param src the new source as string.
     */
    void setSource(const QString& src);
    /**
     * Send set mute on/off command to the Rotel amp.
     *
     * @param m enable mute if true, disable mute otherwise.
     */
    void setMute(bool m);
    /**
     * Send get volume command to the Rotel amp and return retrieved volume.
     *
     * @return the current volume of the Rotel amp.
     */
    int getVolume();
    /**
     * Send get source command to the Rotel amp and return retrieved source.
     *
     * @return the current source of the Rotel amp as string.
     */
    QString getSource();
    /**
     * Send get mute command to the Rotel amp and return retrieved mute state.
     *
     * @return true if the Rotel amp is in mute state, false otherwise.
     */
    bool getMute();

signals:
    /**
     * Signal emitted if controls are connected to the Rotel amp.
     */
    void connected();
    /**
     * Signal emitted if controls loose connection to the Rotel amp.
     */
    void disconnected();
    /**
     * Signal emitted if volume is updated by a setVolume command.
     *
     * @param vol the new volume as integer.
     */
    void volume(int vol);
    /**
     * Signal emitted if source is updated by a setSource command.
     *
     * @param src the new source as string.
     */
    void source(const QString& src);
    /**
     * Signal emitted if the mute state is updated by a setMute command.
     *
     * @param m the new mute state as boolean.
     */
    void mute(bool m);

private:
    xPlayerRotelControls();
    ~xPlayerRotelControls() = default;
    /**
     * Called if controls are connected to the Rotel amp.
     *
     * After the controls are connected, the initial state of the
     * Rotel amp is retrieved and the corresponding signals are
     * emitted.
     */
    void controlsConnected();
    /**
     * Send command to the Rotel amp and retrieve the result.
     *
     * @param command the command send to the Rotel amp.
     * @return the response send back by the Rotel amp.
     */
    QString sendCommand(const QString& command);

    QTcpSocket* rotelSocket;
    static xPlayerRotelControls* rotelControls;
};


class xPlayerRotelWidget:public QWidget {
    Q_OBJECT

public:
    xPlayerRotelWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerRotelWidget() = default;

private slots:
    /**
     * Enable all widget elements of controls are connected.
     */
    void connected();
    /**
     * Disable all widget elements if controls loose connection.
     */
    void disconnected();
    /**
     * Update only the volume knobs position.
     *
     * @param vol the new volume as integer.
     */
    void updateVolume(int vol);
    /**
     * Update only the combo box containing the sources.
     *
     * @param source the new source as string.
     */
    void updateSource(const QString& source);
    /**
     * Update only the checkbox for the mute state.
     *
     * @param mute true if mute is enabled, false otherwise.
     */
    void updateMute(bool mute);
    /**
     * Set the volume of the Rotel amp using Rotel controls.
     *
     * @param vol the new volume as integer.
     */
    void setVolume(int vol);
    /**
     * Set the source of the Rotel amp using Rotel controls.
     *
     * @param source the new source as string.
     */
    void setSource(const QString& source);
    /**
     * Set the mute state of the Rotel amp using Rotel controls.
     *
     * @param mute true if mute will be enabled, false if it will be disabled.
     */
    void setMute(bool mute);

private:
    xPlayerVolumeWidget* rotelVolume;
    QComboBox* rotelSource;
    QLabel* rotelSourceLabel;
    QCheckBox* rotelMute;
    xPlayerRotelControls* rotelControls;
};

#endif
