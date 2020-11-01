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
    static xPlayerRotelControls* controls();

    void connect(const QString& address, int port, bool wait=false);

    void setVolume(int vol);
    void setSource(const QString& src);
    void setMute(bool m);

    int getVolume();
    QString getSource();
    bool getMute();

signals:
    void connected();
    void disconnected();

    void volume(int vol);
    void source(const QString& src);
    void mute(bool m);

private:
    xPlayerRotelControls();
    ~xPlayerRotelControls() = default;

    void controlsConnected();
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
    void connected();
    void disconnected();

    void updateVolume(int vol);
    void updateSource(const QString& source);
    void updateMute(bool mute);

    void setVolume(int vol);
    void setSource(const QString& source);
    void setMute(bool mute);

private:
    xPlayerVolumeWidget* rotelVolume;
    QComboBox* rotelSource;
    QLabel* rotelSourceLabel;
    QCheckBox* rotelMute;
    xPlayerRotelControls* rotelControls;
};

#endif
