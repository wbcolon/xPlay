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
#include "xPlayerRotelControls.h"
#include "xPlayerConfiguration.h"

#include <QList>
#include <QTimer>

xPlayerRotelControls* xPlayerRotelControls::rotelControls = nullptr;

xPlayerRotelControls::xPlayerRotelControls():
        QObject(),
        rotelNetworkPort(0),
        rotelNetworkAddress(),
        rotelMutex() {
    // Setup timer for reconnect
    rotelNetworkReconnect = new QTimer(this);
    rotelNetworkReconnect->setSingleShot(true);
    QObject::connect(rotelNetworkReconnect, &QTimer::timeout, this, &xPlayerRotelControls::controlsCheckConnection);
    // Create a socket to communicate with the amp.
    rotelSocket = new QTcpSocket(this);
    rotelSocket->setSocketOption(QTcpSocket::LowDelayOption, true);
    rotelSocket->setSocketOption(QTcpSocket::KeepAliveOption, true);
    QObject::connect(rotelSocket, &QTcpSocket::connected, this, &xPlayerRotelControls::controlsConnected);
    QObject::connect(rotelSocket, &QTcpSocket::disconnected, this, &xPlayerRotelControls::controlsDisconnected);
}

xPlayerRotelControls* xPlayerRotelControls::controls() {
    // Create and return singleton controls.
    if (rotelControls == nullptr) {
        rotelControls = new xPlayerRotelControls;
    }
    return rotelControls;
}

void xPlayerRotelControls::connect(const QString& address, int port, bool wait) {
    // Do we have a valid address.
    if ((address.isEmpty()) || (port <= 0)) {
        qWarning() << "Rotel: not valid network address given: " << address << ":" << port;
        return;
    }
    // Save current network address and port.
    rotelNetworkPort = port;
    rotelNetworkAddress = address;
    // We do not connect if the widget is disabled.
    if (!xPlayerConfiguration::configuration()->rotelWidget()) {
        qInfo() << "Rotel: not connecting, widget is disabled.";
        return;
    }
    rotelSocket->connectToHost(rotelNetworkAddress, rotelNetworkPort);
    if (wait) {
        rotelSocket->waitForConnected();
    } else {
        // Check after 60 seconds if the Rotel amp is online.
        rotelNetworkReconnect->start(60000);
    }
}

void xPlayerRotelControls::disconnect() {
    rotelNetworkReconnect->stop();
    rotelSocket->disconnectFromHost();
}

void xPlayerRotelControls::setVolume(int vol) {
    auto adjVolume = std::clamp(vol, 0, Rotel::MaximumVolume);
    auto volumeResponse = sendCommand(Rotel::SetVolume.arg(adjVolume, 2, 10, QChar('0')));
    emit volume(adjVolume);
    qDebug() << "RotelControls: setVolume: " << volumeResponse;
}

auto signString = [] (int value) {
    if (value < 0) {
        return QString("-");
    } else if (value > 0) {
        return QString("+");
    } else {
        return QString("0");
    }
};

void xPlayerRotelControls::setBass(int b) {
    auto adjBass = std::clamp(b, -10, 10);
    auto bassResponse = sendCommand(Rotel::SetBass.arg(signString(adjBass)).arg(std::abs(adjBass), 2, 10, QChar('0')));
    emit bass(adjBass);
    qDebug() << "RotelControls: setBass: " << bassResponse;
}

void xPlayerRotelControls::setTreble(int t) {
    auto adjTreble = std::clamp(t, -10, 10);
    auto trebleResponse = sendCommand(Rotel::SetTreble.arg(signString(adjTreble)).arg(std::abs(adjTreble), 2, 10, QChar('0')));
    emit treble(adjTreble);
    qDebug() << "RotelControls: setTreble: " << trebleResponse;
}

void xPlayerRotelControls::setBalance(int b) {
    auto adjBalance = std::clamp(b, -15, 15);
    QString adjBalanceString("000");
    // Lower-case 'l' and 'r' required even though document specified 'L' and 'R'.
    if (adjBalance < 0) {
        adjBalanceString = QString("l%1").arg(-adjBalance, 2, 10, QChar('0'));
    } else if (adjBalance > 0) {
        adjBalanceString = QString("r%1").arg(adjBalance, 2, 10, QChar('0'));
    }
    auto balanceResponse = sendCommand(Rotel::SetBalance.arg(adjBalanceString));
    emit balance(adjBalance);
    qDebug() << "RotelControls: setBalance: " << balanceResponse;
}

void xPlayerRotelControls::setSource(const QString& src) {
    // Check for special "power off" command.
    if (src == Rotel::Source_PowerOff) {
        powerOff();
        return;
    }
    auto srcIndex = Rotel::Sources.indexOf(src);
    if (srcIndex >= 0) {
        // The command for setting pc usb must be "pcusb" even though the return message is "pc_usb"
        // Therefore we need to remove any "_" in the set source command.
        auto srcResponse = sendCommand(Rotel::SetSource.arg(src).remove("_"));
        emit source(src);
        qDebug() << "RotelControls: setSource: " << srcResponse;
    }
}

void xPlayerRotelControls::setMuted(bool m) {
    auto muteResponse = sendCommand(Rotel::SetMute.arg(m ? "on" : "off"));
    emit muted(m);
    qDebug() << "RotelControls: setMute: " << muteResponse;
}

void xPlayerRotelControls::status() {
    emit volume(getVolume());
    emit source(getSource());
    emit muted(isMuted());
    emit bass(getBass());
    emit treble(getTreble());
    emit balance(getBalance());
}

void xPlayerRotelControls::powerOff() {
    auto powerOffResponse = sendCommand(Rotel::PowerOff);
    emit disconnected();
    qDebug() << "RotelControls: powerOff: " << powerOffResponse;
}

int xPlayerRotelControls::getVolume() {
    QString volumeResponse = sendCommand(Rotel::RequestVolume);
    qDebug() << "RotelControls: getVolume: " << volumeResponse;
    if (!volumeResponse.isEmpty()) {
        return volumeResponse.remove(Rotel::ResponsePrefixVolume).remove("$").toInt();
    } else {
        return -1;
    }
}

int xPlayerRotelControls::getBass() {
    QString bassResponse = sendCommand(Rotel::RequestBass);
    qDebug() << "RotelControls: getBass: " << bassResponse;
    if (!bassResponse.isEmpty()) {
        return bassResponse.remove(Rotel::ResponsePrefixBass).remove("$").toInt();
    } else {
        return -1;
    }
}

int xPlayerRotelControls::getTreble() {
    QString trebleResponse = sendCommand(Rotel::RequestTreble);
    qDebug() << "RotelControls: getTreble: " << trebleResponse;
    if (!trebleResponse.isEmpty()) {
        return trebleResponse.remove(Rotel::ResponsePrefixTreble).remove("$").toInt();
    } else {
        return -1;
    }
}

int xPlayerRotelControls::getBalance() {
    QString balanceResponse = sendCommand(Rotel::RequestBalance);
    qDebug() << "RotelControls: getBalance: " << balanceResponse;
    if (!balanceResponse.isEmpty()) {
        return balanceResponse.remove(Rotel::ResponsePrefixBalance).remove("$").remove("R").replace("L","-").toInt();
    } else {
        return 0;
    }
}


QString xPlayerRotelControls::getSource() {
    auto srcString = sendCommand(Rotel::RequestSource);
    qDebug() << "RotelControls: getSource: " << srcString;
    if (!srcString.isEmpty()) {
        return srcString.remove(Rotel::ResponsePrefixSource).remove("$");
    } else {
        return "";
    }
}

bool xPlayerRotelControls::isMuted() {
    auto muteString = sendCommand(Rotel::RequestMute);
    qDebug() << "RotelControls: getMute: " << muteString;
    if (!muteString.isEmpty()) {
        return muteString.remove(Rotel::ResponsePrefixMute).startsWith("on");
    } else {
        return false;
    }
}

QString xPlayerRotelControls::cleanupReplyMessage(const QString& message) {
    qDebug() << "RotelControls: cleanupReplayMessage (before): " << message;
    auto replyMessage = message;
    while (replyMessage.startsWith("amp:freq")) {
        // Check if we have additional commands after the amp:freq
        if (replyMessage.indexOf("$") > 0) {
            replyMessage = replyMessage.remove(0, replyMessage.indexOf("$")+1);
        } else {
            // Last command is also an amp:freq info.
            replyMessage.clear();
        }
    }
    qDebug() << "RotelControls: cleanupReplayMessage (after): " << replyMessage;
    return replyMessage;
}

void xPlayerRotelControls::controlsConnected() {
    // Update UI if connected.
    status();
    emit connected();
}

void xPlayerRotelControls::controlsDisconnected() {
    // Update UI if disconnected.
    rotelSocket->disconnectFromHost();
    rotelSocket->abort();
    emit disconnected();
    // Check every 60 seconds if the Rotel amp is online.
    QTimer::singleShot(60000, this, &xPlayerRotelControls::controlsCheckConnection);
}

void xPlayerRotelControls::controlsCheckConnection() {
    // Return if we are already connected.
    if (rotelSocket->state() == QTcpSocket::ConnectedState) {
        return;
    }
    // Try to connect again.
    rotelSocket->connectToHost(rotelNetworkAddress, rotelNetworkPort);
    if (!rotelSocket->waitForConnected(1000)) {
        // Still not connected? Then try again in 60 seconds.
        rotelNetworkReconnect->start(60000);
    }
}

QString xPlayerRotelControls::sendCommand(const QString& command) {
    // We do ignore commands if the corresponding widget is disabled.
    if (!xPlayerConfiguration::configuration()->rotelWidget()) {
        qInfo() << "xPlayerRotelControls::sendcommand: ignore command, widget disabled.";
        return {};
    }
    QMutexLocker locker(&rotelMutex);
    char readBuffer[65] = { 0 };
    qint64 readBytes = 0;
    rotelSocket->waitForConnected(500);
    if (rotelSocket->state() == QTcpSocket::ConnectedState) {
        rotelSocket->write(command.toStdString().c_str());
        if (!rotelSocket->waitForBytesWritten(500)) {
            qCritical() << "xPlayerRotelControls::sendCommand: unable to send command. Disconnecting.";
            controlsDisconnected();
            return {};
        }
        if (!rotelSocket->waitForReadyRead(2000)) {
            qCritical() << "xPlayerRotelControls::sendCommand: unable to read reply. Disconnecting.";
            controlsDisconnected();
            return {};
        }
        readBytes = rotelSocket->read(readBuffer, 64);
    } else {
        qCritical() << "xPlayerRotelControls::sendCommand: state is not connected. Disconnecting.";
        controlsDisconnected();
    }
    return (readBytes > 0) ? cleanupReplyMessage(QString(readBuffer)) : QString{};
}