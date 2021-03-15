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
#include "xPlayerRotelWidget.h"
#include "xPlayerVolumeWidgetX.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"

#include <QList>
#include <QTimer>
#include <QSpinBox>

// Valid sources for the Rotel A14 or A12 amp
const QList<QString> Rotel_Sources {
        "cd", "coax1", "coax2", "opt1", "opt2", "aux1", "aux2", "tuner", "phono", "usb", "bluetooth", "pc_usb"
};
// Special source indicating power off command.
const QString Rotel_Source_PowerOff = "power off";
// Limit the maximal volume.
const int Rotel_MaximumVolume = 60;
// Command strings in order to communicate to and from the Rotel A14 or A12 amp.
// Volume
const QString Rotel_RequestVolume = "amp:volume?";
const QString Rotel_ResponsePrefixVolume = "amp:volume=";
const QString Rotel_SetVolume = "amp:vol_%1!";
// Bass
const QString Rotel_RequestBass = "amp:bass?";
const QString Rotel_ResponsePrefixBass = "amp:bass=";
const QString Rotel_SetBass = "amp:bass_%1%2!";
// Volume
const QString Rotel_RequestTreble = "amp:treble?";
const QString Rotel_ResponsePrefixTreble = "amp:treble=";
const QString Rotel_SetTreble = "amp:treble_%1%2!";
// Input source
const QString Rotel_RequestSource = "amp:source?";
const QString Rotel_ResponsePrefixSource = "amp:source=";
const QString Rotel_SetSource = "amp:%1!";
// Mute
const QString Rotel_RequestMute = "amp:mute?";
const QString Rotel_ResponsePrefixMute = "amp:mute=";
const QString Rotel_SetMute = "amp:mute_%1!";
// Mute
const QString Rotel_RequestBalance = "amp:balance?";
const QString Rotel_ResponsePrefixBalance = "amp:balance=";
const QString Rotel_SetBalance = "amp:balance_%1!";
// Power off
const QString Rotel_PowerOff = "amp:power_off!";

xPlayerRotelControls* xPlayerRotelControls::rotelControls = nullptr;

xPlayerRotelControls::xPlayerRotelControls():
        QObject(),
        rotelNetworkPort(0),
        rotelNetworkAddress() {
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
        QTimer::singleShot(60000, this, &xPlayerRotelControls::controlsCheckConnection);
    }
}

void xPlayerRotelControls::setVolume(int vol) {
    auto adjVolume = std::clamp(vol, 0, Rotel_MaximumVolume);
    auto volumeResponse = sendCommand(Rotel_SetVolume.arg(adjVolume, 2, 10, QChar('0')));
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
    auto bassResponse = sendCommand(Rotel_SetBass.arg(signString(adjBass)).arg(std::abs(adjBass), 2, 10, QChar('0')));
    emit bass(adjBass);
    qDebug() << "RotelControls: setBass: " << bassResponse;
}

void xPlayerRotelControls::setTreble(int t) {
    auto adjTreble = std::clamp(t, -10, 10);
    auto trebleResponse = sendCommand(Rotel_SetTreble.arg(signString(adjTreble)).arg(std::abs(adjTreble), 2, 10, QChar('0')));
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
    auto balanceResponse = sendCommand(Rotel_SetBalance.arg(adjBalanceString));
    emit balance(adjBalance);
    qDebug() << "RotelControls: setBalance: " << balanceResponse;
}

void xPlayerRotelControls::setSource(const QString& src) {
    // Check for special "power off" command.
    if (src == Rotel_Source_PowerOff) {
        powerOff();
        return;
    }
    auto srcIndex = Rotel_Sources.indexOf(src);
    if (srcIndex >= 0) {
        auto srcResponse = sendCommand(Rotel_SetSource.arg(src));
        emit source(src);
        qDebug() << "RotelControls: setSource: " << srcResponse;
    }
}

void xPlayerRotelControls::setMuted(bool m) {
    auto muteResponse = sendCommand(Rotel_SetMute.arg(m ? "on" : "off"));
    emit muted(m);
    qDebug() << "RotelControls: setMute: " << muteResponse;
}

void xPlayerRotelControls::powerOff() {
    auto powerOffResponse = sendCommand(Rotel_PowerOff);
    emit disconnected();
    qDebug() << "RotelControls: powerOff: " << powerOffResponse;
}

int xPlayerRotelControls::getVolume() {
    QString volumeResponse = sendCommand(Rotel_RequestVolume);
    qDebug() << "RotelControls: getVolume: " << volumeResponse;
    if (!volumeResponse.isEmpty()) {
        return volumeResponse.remove(Rotel_ResponsePrefixVolume).remove("$").toInt();
    } else {
        return -1;
    }
}

int xPlayerRotelControls::getBass() {
    QString bassResponse = sendCommand(Rotel_RequestBass);
    qDebug() << "RotelControls: getBass: " << bassResponse;
    if (!bassResponse.isEmpty()) {
        return bassResponse.remove(Rotel_ResponsePrefixBass).remove("$").toInt();
    } else {
        return -1;
    }
}

int xPlayerRotelControls::getTreble() {
    QString trebleResponse = sendCommand(Rotel_RequestTreble);
    qDebug() << "RotelControls: getTreble: " << trebleResponse;
    if (!trebleResponse.isEmpty()) {
        return trebleResponse.remove(Rotel_ResponsePrefixTreble).remove("$").toInt();
    } else {
        return -1;
    }
}

int xPlayerRotelControls::getBalance() {
    QString balanceResponse = sendCommand(Rotel_RequestBalance);
    qDebug() << "RotelControls: getBalance: " << balanceResponse;
    if (!balanceResponse.isEmpty()) {
        return balanceResponse.remove(Rotel_ResponsePrefixBalance).remove("$").remove("R").replace("L","-").toInt();
    } else {
        return 0;
    }
}


QString xPlayerRotelControls::getSource() {
    auto srcString = sendCommand(Rotel_RequestSource);
    qDebug() << "RotelControls: getSource: " << srcString;
    if (!srcString.isEmpty()) {
        return srcString.remove(Rotel_ResponsePrefixSource).remove("$");
    } else {
        return "";
    }
}

bool xPlayerRotelControls::isMuted() {
    auto muteString = sendCommand(Rotel_RequestMute);
    qDebug() << "RotelControls: getMute: " << muteString;
    if (!muteString.isEmpty()) {
        return muteString.remove(Rotel_ResponsePrefixMute).startsWith("on");
    } else {
        return false;
    }
}

void xPlayerRotelControls::controlsConnected() {
    // Update UI if connected.
    emit volume(getVolume());
    emit source(getSource());
    emit muted(isMuted());
    emit bass(getBass());
    emit treble(getTreble());
    emit balance(getBalance());
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
        QTimer::singleShot(60000, this, &xPlayerRotelControls::controlsCheckConnection);
    }
}

QString xPlayerRotelControls::sendCommand(const QString& command) {
    // We do ignore commands if the corresponding widget is disabled.
    if (!xPlayerConfiguration::configuration()->rotelWidget()) {
        qInfo() << "xPlayerRotelControls::sendcommand: ignore command, widget disabled.";
        return QString();
    }
    char readBuffer[65] = { 0 };
    qint64 readBytes = 0;
    rotelSocket->waitForConnected(500);
    if (rotelSocket->state() == QTcpSocket::ConnectedState) {
        rotelSocket->write(command.toStdString().c_str());
        if (!rotelSocket->waitForBytesWritten(500)) {
            qCritical() << "xPlayerRotelControls::sendCommand: unable to send command. Disconnecting.";
            controlsDisconnected();
            return QString();
        }
        if (!rotelSocket->waitForReadyRead(2000)) {
            qCritical() << "xPlayerRotelControls::sendCommand: unable to read reply. Disconnecting.";
            controlsDisconnected();
            return QString();
        }
        readBytes = rotelSocket->read(readBuffer, 64);
    } else {
        qCritical() << "xPlayerRotelControls::sendCommand: state is not connected. Disconnecting.";
        controlsDisconnected();
    }
    return (readBytes > 0) ? QString(readBuffer) : QString();
}


xPlayerRotelWidget::xPlayerRotelWidget(QWidget *parent, Qt::Orientation orientation, Qt::WindowFlags flags):
        QWidget(parent, flags) {
    auto rotelLayout = new xPlayerLayout(this);
    rotelLayout->setSpacing(xPlayerLayout::NoSpace);
    // Create a volume knob.
    rotelVolume = new xPlayerVolumeWidgetX(this);
    // Create rotel control buttons widget.
    auto rotelControlButton = new QWidget(this);
    auto rotelControlButtonLayout = new xPlayerLayout();
    // Create a combo box for the input sources and add valid sources.
    rotelSource = new QComboBox(rotelControlButton);
    for (const auto& source : Rotel_Sources) {
        rotelSource->addItem(source);
    }
    // Insert separator and the special power off source.
    rotelSource->insertSeparator(rotelSource->count());
    rotelSource->addItem(Rotel_Source_PowerOff);
    rotelBass = new QSpinBox(rotelControlButton);
    rotelBass->setRange(-10, 10);
    rotelTreble = new QSpinBox(rotelControlButton);
    rotelTreble->setRange(-10, 10);
    rotelBassLabel = new QLabel(tr("Bass"), rotelControlButton);
    rotelBassLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    rotelTrebleLabel = new QLabel(tr("Treble"), rotelControlButton);
    rotelTrebleLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    rotelBalanceLabel = new QLabel(tr("Balance"), rotelControlButton);
    rotelBalanceLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    rotelBalance = new xPlayerBalanceWidgetX(rotelControlButton);
    rotelBalance->setRange(15);
    rotelControlButtonLayout->addRowStretcher(0);
    rotelControlButtonLayout->addWidget(rotelSource, 1, 0, 1, 2);
    rotelControlButtonLayout->addRowSpacer(2, xPlayerLayout::SmallSpace);
    rotelControlButtonLayout->addWidget(rotelBassLabel, 3, 0);
    rotelControlButtonLayout->addWidget(rotelTrebleLabel, 3, 1);
    rotelControlButtonLayout->addWidget(rotelBass, 4, 0);
    rotelControlButtonLayout->addWidget(rotelTreble, 4, 1);
    rotelControlButtonLayout->addRowSpacer(5, xPlayerLayout::SmallSpace);
    rotelControlButtonLayout->addWidget(rotelBalanceLabel, 6, 0, 1, 2);
    rotelControlButtonLayout->addWidget(rotelBalance, 7, 0, 1, 2);
    rotelControlButtonLayout->addRowStretcher(8);
    rotelControlButtonLayout->setContentsMargins(0, 0, 0, 0);
    rotelControlButton->setLayout(rotelControlButtonLayout);
    rotelControlButton->setContentsMargins(0, 0, 0, 0);
    rotelControlButton->setFixedWidth(xPlayerControlButtonWidgetWidth);
    // Add the volume knob, bass, treble and the source input to the layout
    if (orientation == Qt::Horizontal) {
        rotelLayout->addWidget(rotelVolume, 0, 0);
        rotelLayout->addColumnSpacer(1, xPlayerLayout::MediumSpace);
        rotelLayout->addWidget(rotelControlButton, 0, 2);
    } else {
        rotelLayout->setSpacing(xPlayerLayout::SmallSpace);
        rotelLayout->addWidget(rotelControlButton, 0, 0);
        rotelLayout->addRowSpacer(1, xPlayerLayout::MediumSpace);
        // Adjust width for vertical layout.
        rotelVolume->setFixedWidth(xPlayerControlButtonWidgetWidth);
        rotelLayout->addWidget(rotelVolume, 2, 0);
    }
    rotelControls = xPlayerRotelControls::controls();
    // Connect the widgets to the amp commands.
    QObject::connect(rotelVolume, &xPlayerVolumeWidgetX::volume, rotelControls, &xPlayerRotelControls::setVolume);
    QObject::connect(rotelVolume, &xPlayerVolumeWidgetX::muted, rotelControls, &xPlayerRotelControls::setMuted);
    QObject::connect(rotelBass, SIGNAL(valueChanged(int)), rotelControls, SLOT(setBass(int)));
    QObject::connect(rotelTreble, SIGNAL(valueChanged(int)), rotelControls, SLOT(setTreble(int)));
    QObject::connect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), rotelControls, SLOT(setSource(const QString&)));
    QObject::connect(rotelBalance, &xPlayerBalanceWidgetX::balance, rotelControls, &xPlayerRotelControls::setBalance);
    // Connect Rotel controls to Rotel widget.
    QObject::connect(rotelControls, &xPlayerRotelControls::volume, this, &xPlayerRotelWidget::updateVolume);
    QObject::connect(rotelControls, &xPlayerRotelControls::bass, this, &xPlayerRotelWidget::updateBass);
    QObject::connect(rotelControls, &xPlayerRotelControls::treble, this, &xPlayerRotelWidget::updateTreble);
    QObject::connect(rotelControls, &xPlayerRotelControls::balance, this, &xPlayerRotelWidget::updateBalance);
    QObject::connect(rotelControls, &xPlayerRotelControls::source, this, &xPlayerRotelWidget::updateSource);
    QObject::connect(rotelControls, &xPlayerRotelControls::muted, this, &xPlayerRotelWidget::updateMuted);
    QObject::connect(rotelControls, &xPlayerRotelControls::connected, this, &xPlayerRotelWidget::connected);
    QObject::connect(rotelControls, &xPlayerRotelControls::disconnected, this, &xPlayerRotelWidget::disconnected);
    // Disable until connected.
    disconnected();
}

void xPlayerRotelWidget::updateVolume(int vol) {
    qDebug() << "xPlayerRotel: updateVolume: " << vol;
    QObject::disconnect(rotelVolume, &xPlayerVolumeWidgetX::volume, rotelControls, &xPlayerRotelControls::setVolume);
    rotelVolume->setVolume(vol);
    QObject::connect(rotelVolume, &xPlayerVolumeWidgetX::volume, rotelControls, &xPlayerRotelControls::setVolume);
}

void xPlayerRotelWidget::updateBass(int b) {
    qDebug() << "xPlayerRotel: updateBass: " << b;
    QObject::disconnect(rotelBass, SIGNAL(valueChanged(int)), rotelControls, SLOT(setBass(int)));
    rotelBass->setValue(b);
    QObject::connect(rotelBass, SIGNAL(valueChanged(int)), rotelControls, SLOT(setBass(int)));
}

void xPlayerRotelWidget::updateTreble(int t) {
    qDebug() << "xPlayerRotel: updateTreble: " << t;
    QObject::disconnect(rotelTreble, SIGNAL(valueChanged(int)), rotelControls, SLOT(setTreble(int)));
    rotelTreble->setValue(t);
    QObject::connect(rotelTreble, SIGNAL(valueChanged(int)), rotelControls, SLOT(setTreble(int)));
}

void xPlayerRotelWidget::updateBalance(int b) {
    qDebug() << "xPlayerRotel: updateBalance: " << b;
    QObject::disconnect(rotelBalance, &xPlayerBalanceWidgetX::balance, rotelControls, &xPlayerRotelControls::setBalance);
    rotelBalance->setBalance(b);
    QObject::connect(rotelBalance, &xPlayerBalanceWidgetX::balance, rotelControls, &xPlayerRotelControls::setBalance);
}

void xPlayerRotelWidget::updateSource(const QString& source) {
    qDebug() << "xPlayerRotel: updateSource: " << source;
    QObject::disconnect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), rotelControls, SLOT(setSource(const QString&)));
    rotelSource->setCurrentIndex(Rotel_Sources.indexOf(source));
    QObject::connect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), rotelControls, SLOT(setSource(const QString&)));
}

void xPlayerRotelWidget::updateMuted(bool mute) {
    qDebug() << "xPlayerRotel: updateMute: " << mute;
    // No disconnect necessary. Own function does not issue signal.
    rotelVolume->setMuted(mute);
}

void xPlayerRotelWidget::connected() {
    qDebug() << "xPlayerRotel: connected";
    // Enable all widgets.
    rotelVolume->setEnabled(true);
    rotelSource->setEnabled(true);
    rotelTrebleLabel->setEnabled(true);
    rotelTreble->setEnabled(true);
    rotelBassLabel->setEnabled(true);
    rotelBass->setEnabled(true);
    rotelBalanceLabel->setEnabled(true);
    rotelBalance->setEnabled(true);
}

void xPlayerRotelWidget::disconnected() {
    qDebug() << "xPlayerRotel: disconnected";
    rotelVolume->setEnabled(false);
    rotelSource->setEnabled(false);
    rotelTrebleLabel->setEnabled(false);
    rotelTreble->setEnabled(false);
    rotelBassLabel->setEnabled(false);
    rotelBass->setEnabled(false);
    rotelBalanceLabel->setEnabled(false);
    rotelBalance->setEnabled(false);
}

