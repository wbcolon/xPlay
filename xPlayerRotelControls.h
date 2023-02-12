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
#ifndef __XPLAYERROTELCONTROLS_H__
#define __XPLAYERROTELCONTROLS_H__

#include <QTcpSocket>
#include <QWidget>
#include <QString>

namespace Rotel {

// Valid sources for the Rotel A14 or A12 amp
const QList<QString> Sources { // NOLINT
        "cd", "coax1", "coax2", "opt1", "opt2", "aux1", "aux2", "tuner", "phono", "usb", "bluetooth", "pc_usb"
};
// Special source indicating power off command.
const QString Source_PowerOff = "power off"; // NOLINT
// Limit the maximal volume.
const int MaximumVolume = 60;
// Command strings in order to communicate to and from the Rotel A14 or A12 amp.
// Volume
const QString RequestVolume = "amp:volume?"; // NOLINT
const QString ResponsePrefixVolume = "amp:volume="; // NOLINT
const QString SetVolume = "amp:vol_%1!"; // NOLINT
// Bass
const QString RequestBass = "amp:bass?"; // NOLINT
const QString ResponsePrefixBass = "amp:bass="; // NOLINT
const QString SetBass = "amp:bass_%1%2!"; // NOLINT
// Volume
const QString RequestTreble = "amp:treble?"; // NOLINT
const QString ResponsePrefixTreble = "amp:treble="; // NOLINT
const QString SetTreble = "amp:treble_%1%2!"; // NOLINT
// Input source
const QString RequestSource = "amp:source?"; // NOLINT
const QString ResponsePrefixSource = "amp:source="; // NOLINT
const QString SetSource = "amp:%1!"; // NOLINT
// Mute
const QString RequestMute = "amp:mute?"; // NOLINT
const QString ResponsePrefixMute = "amp:mute="; // NOLINT
const QString SetMute = "amp:mute_%1!"; // NOLINT
// Mute
const QString RequestBalance = "amp:balance?"; // NOLINT
const QString ResponsePrefixBalance = "amp:balance="; // NOLINT
const QString SetBalance = "amp:balance_%1!"; // NOLINT
// Power off
const QString PowerOff = "amp:power_off!"; // NOLINT

}

// Allow test class to access everything.
class test_xPlayerRotelControls;

class xPlayerRotelControls:public QObject {
    Q_OBJECT

    friend class test_xPlayerRotelControls;

public:
    /**
     * Return the Rotel amp controls.
     *
     * @return pointer to a singleton of the rotel amp controls.
     */
    [[nodiscard]] static xPlayerRotelControls* controls();
    /**
     * Connect controls via network to the Rotel amp.
     *
     * @param address the address of the Rotel amp as string (IP or DNS name).
     * @param port the port the controls connect to as integer.
     * @param wait if true then wait until the socket is connected.
     */
    void connect(const QString& address, int port, bool wait=false);
    /**
     * Send get volume command to the Rotel amp and return retrieved volume.
     *
     * @return the current volume of the Rotel amp.
     */
    [[nodiscard]] int getVolume();
    /**
     * Send get bass command to the Rotel amp and return retrieved bass.
     *
     * @return the current bass of the Rotel amp.
     */
    [[nodiscard]] int getBass();
    /**
     * Send get treble command to the Rotel amp and return retrieved treble.
     *
     * @return the current treble of the Rotel amp.
     */
    [[nodiscard]] int getTreble();
    /**
     * Send get balance command to the Rotel amp and return retrieved treble.
     *
     * @return the current balance of the Rotel amp (< 0 to the left, > 0 to the right).
     */
    [[nodiscard]] int getBalance();
    /**
     * Send get source command to the Rotel amp and return retrieved source.
     *
     * @return the current source of the Rotel amp as string.
     */
    [[nodiscard]] QString getSource();
    /**
     * Send get mute command to the Rotel amp and return retrieved mute state.
     *
     * @return true if the Rotel amp is in mute state, false otherwise.
     */
    [[nodiscard]] bool isMuted();

public slots:
    /**
     * Send set volume command to the Rotel amp.
     *
     * @param vol the new volume as integer.
     */
    void setVolume(int vol);
    /**
     * Send set bass command to the Rotel amp.
     *
     * @param b the new bass as integer.
     */
    void setBass(int b);
    /**
     * Send set treble command to the Rotel amp.
     *
     * @param t the new treble as integer.
     */
    void setTreble(int t);
    /**
     * Send set balance command to the Rotel amp.
     *
     * @param b the new balance as integer (< 0 to the left, > 0 to the right)
     */
    void setBalance(int b);
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
    void setMuted(bool m);
    /**
     * Send command to power off the Rotel amp.
     */
    void powerOff();

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
     * Signal emitted if bass is updated by a setBass command.
     *
     * @param b the new volume as integer.
     */
    void bass(int b);
    /**
     * Signal emitted if treble is updated by a setTreble command.
     *
     * @param t the new volume as integer.
     */
    void treble(int t);
    /**
     * Signal emitted if balance is updated by a setBalance command.
     *
     * @param b the new balance as integer (< 0 to the left, > 0 to the right).
     */
    void balance(int b);
    /**
     * Signal emitted if source is updated by a setSource command.
     *
     * @param src the new source as string.
     */
    void source(const QString& src);
    /**
     * Signal emitted if the mute state is updated by a setMuted command.
     *
     * @param m the new mute state as boolean.
     */
    void muted(bool m);

private slots:
    /**
     * Called if controls are connected to the Rotel amp.
     *
     * After the controls are connected, the initial state of the
     * Rotel amp is retrieved and the corresponding signals are
     * emitted.
     */
    void controlsConnected();
    /**
     * Called if controls are disconnected from the Rotel amp.
     *
     * The disconnect can also be called if one of the sendCommands
     * to the Rotel amp fails. Try every 60 seconds to reconnect.
     */
    void controlsDisconnected();
    /**
     * Called every 60 seconds if Rotel amp is disconnected.
     *
     * Try to reconnect. If that fails try again in 60 seconds.
     */
    void controlsCheckConnection();
    /**
     * Cleanup any response commands that we do not support.
     *
     * @param message the raw reply message as string.
     * @return the cleaned reply message as string.
     */
    static QString cleanupReplyMessage(const QString& message);

private:
    /**
     * Constructor. Create socket and connect signals.
     */
    xPlayerRotelControls();
    /**
     * Destructor. Default.
     */
    ~xPlayerRotelControls() override = default;
    /**
     * Send command to the Rotel amp and retrieve the result.
     *
     * @param command the command send to the Rotel amp.
     * @return the response send back by the Rotel amp.
     */
    QString sendCommand(const QString& command);

    QTcpSocket* rotelSocket;
    int rotelNetworkPort;
    QString rotelNetworkAddress;
    static xPlayerRotelControls* rotelControls;
};

#endif