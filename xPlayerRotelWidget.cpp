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

#include <QGridLayout>
#include <QList>

// Valid sources for the Rotel A14 or A12 amp
const QList<QString> Rotel_Sources {
        "cd", "coax1", "coax2", "opt1", "opt2", "aux1", "aux2", "tuner", "phono", "usb", "bluetooth", "pcusb"
};
// Limit the maximal volume.
const int Rotel_MaximumVolume = 60;
// Command strings in order to communicate to and from the Rotel A14 or A12 amp.
// Volume
const QString Rotel_RequestVolume = "amp:volume?";
const QString Rotel_ResponsePrefixVolume = "amp:volume=";
const QString Rotel_SetVolume = "amp:vol_%1!";
// Input source
const QString Rotel_RequestSource = "amp:source?";
const QString Rotel_ResponsePrefixSource = "amp:source=";
const QString Rotel_SetSource = "amp:%1!";
// Mute
const QString Rotel_RequestMute = "amp:mute?";
const QString Rotel_ResponsePrefixMute = "amp:mute=";
const QString Rotel_SetMute = "amp:mute_%1!";


xPlayerRotelControls* xPlayerRotelControls::rotelControls = nullptr;

xPlayerRotelControls::xPlayerRotelControls():
        QObject() {
    // Create a socket to communicate with the amp.
    rotelSocket = new QTcpSocket(this);
    QObject::connect(rotelSocket, &QTcpSocket::connected, this, &xPlayerRotelControls::controlsConnected);
    QObject::connect(rotelSocket, &QTcpSocket::disconnected, this, &xPlayerRotelControls::disconnected);
}

xPlayerRotelControls* xPlayerRotelControls::controls() {
    // Create and return singleton controls.
    if (rotelControls == nullptr) {
        rotelControls = new xPlayerRotelControls;
    }
    return rotelControls;
}

void xPlayerRotelControls::connect(const QString& address, int port, bool wait) {
    if ((address.isEmpty()) || (port <= 0)) {
        qWarning() << "Rotel: not valid network address given: " << address << ":" << port;
        return;
    }
    rotelSocket->connectToHost(address, port);
    if (wait) {
        rotelSocket->waitForConnected();
    }
}

void xPlayerRotelControls::setVolume(int vol) {
    auto adjVolume = std::clamp(vol, 0, Rotel_MaximumVolume);
    auto volumeResponse = sendCommand(Rotel_SetVolume.arg(adjVolume, 2, 10, QChar('0')));
    emit volume(adjVolume);
    qDebug() << "RotelControls: setVolume: " << volumeResponse;
}

void xPlayerRotelControls::setSource(const QString& src) {
    auto srcIndex = Rotel_Sources.indexOf(src);
    if (srcIndex >= 0) {
        auto srcResponse = sendCommand(Rotel_SetSource.arg(src));
        emit source(src);
        qDebug() << "RotelControls: setSource: " << srcResponse;
    }
}

void xPlayerRotelControls::setMute(bool m) {
    auto muteResponse = sendCommand(Rotel_SetMute.arg(m ? "on" : "off"));
    emit mute(m);
    qDebug() << "RotelControls: setMute: " << muteResponse;
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

QString xPlayerRotelControls::getSource() {
    auto srcString = sendCommand(Rotel_RequestSource);
    qDebug() << "RotelControls: getSource: " << srcString;
    if (!srcString.isEmpty()) {
        return srcString.remove(Rotel_ResponsePrefixSource).remove("$");
    } else {
        return "";
    }
}

bool xPlayerRotelControls::getMute() {
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
    emit mute(getMute());
    emit connected();
}

QString xPlayerRotelControls::sendCommand(const QString& command) {
    char readBuffer[65] = { 0 };
    qint64 readBytes = 0;
    rotelSocket->waitForConnected(500);
    if (rotelSocket->state() == QAbstractSocket::ConnectedState) {
        rotelSocket->write(command.toStdString().c_str());
        rotelSocket->waitForBytesWritten();
        rotelSocket->waitForReadyRead();
        readBytes = rotelSocket->read(readBuffer, 64);
    }
    return (readBytes > 0) ? QString(readBuffer) : QString();
}


xPlayerRotelWidget::xPlayerRotelWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags) {
    auto rotelLayout = new QGridLayout(this);
    // Create a volume knop.
    rotelVolume = new xPlayerVolumeWidgetX(this);
    // Create a combo box for the input sources and add valid sources.
    rotelSource = new QComboBox(this);
    for (const auto& source : Rotel_Sources) {
       rotelSource->addItem(source);
    }
    rotelSourceLabel = new QLabel(tr("Source"));
    rotelSourceLabel->setAlignment(Qt::AlignLeft);
    rotelMute = new QCheckBox(tr("Mute"));
    // Add the volume knob and the source input to the layout
    rotelLayout->addWidget(rotelVolume, 0, 0, 4, 4);
    rotelLayout->setColumnMinimumWidth(4, 20);
    rotelLayout->addWidget(rotelSourceLabel, 0, 5, 1, 4);
    rotelLayout->addWidget(rotelSource, 1, 5, 1, 4);
    rotelLayout->addWidget(rotelMute, 2, 5, 1, 4);
    // Connect the widgets to the amp commands.
    QObject::connect(rotelVolume, &xPlayerVolumeWidgetX::volume, this, &xPlayerRotelWidget::setVolume);
    QObject::connect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setSource(const QString&)));
    QObject::connect(rotelMute, &QCheckBox::clicked, this, &xPlayerRotelWidget::setMute);
    // Connect Rotel controls to Rotel widget.
    rotelControls = xPlayerRotelControls::controls();
    QObject::connect(rotelControls, &xPlayerRotelControls::volume, this, &xPlayerRotelWidget::updateVolume);
    QObject::connect(rotelControls, &xPlayerRotelControls::source, this, &xPlayerRotelWidget::updateSource);
    QObject::connect(rotelControls, &xPlayerRotelControls::mute, this, &xPlayerRotelWidget::updateMute);
    QObject::connect(rotelControls, &xPlayerRotelControls::connected, this, &xPlayerRotelWidget::connected);
    QObject::connect(rotelControls, &xPlayerRotelControls::disconnected, this, &xPlayerRotelWidget::disconnected);
    // Disable until connected.
    rotelVolume->setEnabled(false);
    rotelSource->setEnabled(false);
    rotelSourceLabel->setEnabled(false);
    rotelMute->setEnabled(false);
}

void xPlayerRotelWidget::updateVolume(int vol) {
    qDebug() << "xPlayerRotel: updateVolume: " << vol;
    QObject::disconnect(rotelVolume, &xPlayerVolumeWidgetX::volume, this, &xPlayerRotelWidget::setVolume);
    rotelVolume->setVolume(vol);
    QObject::connect(rotelVolume, &xPlayerVolumeWidgetX::volume, this, &xPlayerRotelWidget::setVolume);
}

void xPlayerRotelWidget::updateSource(const QString& source) {
    qDebug() << "xPlayerRotel: updateSource: " << source;
    QObject::disconnect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setSource(const QString&)));
    rotelSource->setCurrentIndex(Rotel_Sources.indexOf(source));
    QObject::connect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setSource(const QString&)));
}

void xPlayerRotelWidget::updateMute(bool mute) {
    qDebug() << "xPlayerRotel: updateMute: " << mute;
    QObject::disconnect(rotelMute, &QCheckBox::clicked, this, &xPlayerRotelWidget::setMute);
    rotelMute->setChecked(mute);
    QObject::connect(rotelMute, &QCheckBox::clicked, this, &xPlayerRotelWidget::setMute);
}

void xPlayerRotelWidget::connected() {
    qDebug() << "xPlayerRotel: connected";
    // Enable all widgets.
    rotelVolume->setEnabled(true);
    rotelSource->setEnabled(true);
    rotelSourceLabel->setEnabled(true);
    rotelMute->setEnabled(true);
}

void xPlayerRotelWidget::disconnected() {
    qDebug() << "xPlayerRotel: disconnected";
}

void xPlayerRotelWidget::setVolume(int vol) {
    rotelControls->setVolume(vol);
}

void xPlayerRotelWidget::setSource(const QString& src) {
    rotelControls->setSource(src);
}

void xPlayerRotelWidget::setMute(bool m) {
    rotelControls->setMute(m);
}

