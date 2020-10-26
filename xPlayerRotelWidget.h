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

class xPlayerRotelWidget:public QWidget {
    Q_OBJECT

public:
    xPlayerRotelWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerRotelWidget() = default;

    void connect(const QString& address, int port, bool wait=false);

private slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError e);

    void setVolume(int vol);
    void setSource(const QString& source);
    void setMute(bool mute);

private:
    int getVolume();
    QString getSource();
    bool getMute();

    QString sendCommand(const QString& command);

    QTcpSocket* rotelSocket;
    xPlayerVolumeWidget* rotelVolume;
    QComboBox* rotelSource;
    QLabel* rotelSourceLabel;
    QCheckBox* rotelMute;
};

#endif
