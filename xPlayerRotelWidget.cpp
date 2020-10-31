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

xPlayerRotelWidget::xPlayerRotelWidget(QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        rotelChild(nullptr),
        rotelParent(nullptr) {
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
    // Create a socket to communicate with the amp.
    rotelSocket = new QTcpSocket(this);
    QObject::connect(rotelSocket, &QTcpSocket::connected, this, &xPlayerRotelWidget::connected);
    QObject::connect(rotelSocket, &QTcpSocket::disconnected, this, &xPlayerRotelWidget::disconnected);
    // Disable until connected.
    rotelVolume->setEnabled(false);
    rotelSource->setEnabled(false);
    rotelSourceLabel->setEnabled(false);
    rotelMute->setEnabled(false);
}

void xPlayerRotelWidget::connect(const QString& address, int port, bool wait) {
    if ((address.isEmpty()) || (port <= 0)) {
        qWarning() << "Rotel: not valid network address given: " << address << ":" << port;
        return;
    }
    rotelSocket->connectToHost(address, port);
    if (wait) {
        rotelSocket->waitForConnected();
    }
}

void xPlayerRotelWidget::connectToParent(xPlayerRotelWidget* rotel) {
    // Connect with child widget. Disconnect with nullptr.
    qDebug() << "CONNECT_PARENT: " << rotel;
    if (rotelParent) {
        rotelParent->rotelChild = nullptr;
    }
    rotelParent = rotel;
    if (rotelParent) {
        rotelParent->rotelChild = this;
    }
}

void xPlayerRotelWidget::syncVolume(int vol) {
    qDebug() << "SYNC_VOLUME: " << vol << ", " << this;
    QObject::disconnect(rotelVolume, &xPlayerVolumeWidgetX::volume, this, &xPlayerRotelWidget::setVolume);
    rotelVolume->setVolume(vol);
    QObject::connect(rotelVolume, &xPlayerVolumeWidgetX::volume, this, &xPlayerRotelWidget::setVolume);
}

void xPlayerRotelWidget::syncSource(const QString& source) {
    qDebug() << "SYNC_SOURCE: " << source << ", " << this;
    QObject::disconnect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setSource(const QString&)));
    rotelSource->setCurrentIndex(Rotel_Sources.indexOf(source));
    QObject::connect(rotelSource, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setSource(const QString&)));
}

void xPlayerRotelWidget::syncMute(bool mute) {
    qDebug() << "SYNC_MUTE: " << mute << ", " << this;
    QObject::disconnect(rotelMute, &QCheckBox::clicked, this, &xPlayerRotelWidget::setMute);
    rotelMute->setChecked(mute);
    QObject::connect(rotelMute, &QCheckBox::clicked, this, &xPlayerRotelWidget::setMute);
}

void xPlayerRotelWidget::connected() {
    qDebug() << "Rotel: connected";
    // Enable all widgets.
    rotelVolume->setEnabled(true);
    rotelSource->setEnabled(true);
    rotelSourceLabel->setEnabled(true);
    rotelMute->setEnabled(true);
    // Enable all widgets for rotel child.
    qDebug() << "ROTEL: CHILD: " << rotelChild;
    if (rotelChild) {
        rotelChild->rotelVolume->setEnabled(true);
        rotelChild->rotelSource->setEnabled(true);
        rotelChild->rotelSourceLabel->setEnabled(true);
        rotelChild->rotelMute->setEnabled(true);
    }
    // Initialize.
    rotelVolume->setVolume(getVolume());
    auto sourceIndex = Rotel_Sources.indexOf(getSource());
    rotelSource->setCurrentIndex(sourceIndex);
    rotelMute->setChecked(getMute());

}

void xPlayerRotelWidget::disconnected() {
    qDebug() << "Rotel: disconnected";
}

void xPlayerRotelWidget::error(QAbstractSocket::SocketError e) {
    qDebug() << "Rotel: error: " << e;
}

auto volumeResponseToInt = [] (QString response) {
    return response.remove(Rotel_ResponsePrefixVolume).remove("$").toInt();
};

void xPlayerRotelWidget::setVolume(int v) {
    if (rotelParent) {
        rotelParent->setVolume(v);
    } else {
        auto adjVolume = std::clamp(v, 0, Rotel_MaximumVolume);
        auto volumeResponse = sendCommand(Rotel_SetVolume.arg(adjVolume, 2, 10, QChar('0')));
        qDebug() << "Rotel: setVolume: " << volumeResponse;
        if (rotelChild) {
            rotelChild->syncVolume(adjVolume);
        }
    }
}

void xPlayerRotelWidget::setSource(const QString& source) {
    if (rotelParent) {
        rotelParent->syncSource(source);
        rotelParent->setSource(source);
    } else {
        auto sourceIndex = Rotel_Sources.indexOf(source);
        if (sourceIndex >= 0) {
            auto sourceResponse = sendCommand(Rotel_SetSource.arg(source));
            qDebug() << "Rotel: setSource: " << sourceResponse;
            if (rotelChild) {
                rotelChild->syncSource(source);
            }
        }
    }
}

void xPlayerRotelWidget::setMute(bool mute) {
    if (rotelParent) {
        rotelParent->syncMute(mute);
        rotelParent->setMute(mute);
    } else {
        auto muteResponse = sendCommand(Rotel_SetMute.arg(mute ? "on" : "off"));
        qDebug() << "Rotel: setMute: " << muteResponse;
        if (rotelChild) {
            rotelChild->syncMute(mute);
        }
    }
}

int xPlayerRotelWidget::getVolume() {
    if (rotelParent) {
        return rotelParent->getVolume();
    } else {
        QString volumeResponse = sendCommand(Rotel_RequestVolume);
        qDebug() << "Rotel: getVolume: " << volumeResponse;
        if (!volumeResponse.isEmpty()) {
            return volumeResponseToInt(volumeResponse);
        } else {
            return -1;
        }
    }
}

QString xPlayerRotelWidget::getSource() {
    if (rotelParent) {
        return rotelParent->getSource();
    } else {
        auto srcString = sendCommand(Rotel_RequestSource);
        qDebug() << "Rotel: getSource: " << srcString;
        if (!srcString.isEmpty()) {
            return srcString.remove(Rotel_ResponsePrefixSource).remove("$");
        } else {
            return "";
        }
    }
}

bool xPlayerRotelWidget::getMute() {
    if (rotelParent) {
        return rotelParent->getMute();
    } else {
        auto muteString = sendCommand(Rotel_RequestMute);
        qDebug() << "Rotel: getMute: " << muteString;
        if (!muteString.isEmpty()) {
            return muteString.remove(Rotel_ResponsePrefixMute).startsWith("on");
        } else {
            return false;
        }
    }
}

QString xPlayerRotelWidget::sendCommand(const QString& command) {
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