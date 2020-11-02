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
#ifndef __XPLAYERVOLUMEWIDGET_H__
#define __XPLAYERVOLUMEWIDGET_H__

#include <QWidget>

class xPlayerVolumeWidget:public QWidget {
    Q_OBJECT

public:
    xPlayerVolumeWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerVolumeWidget() = default;
    /**
     * Retrieve the current volume of displayed in the UI.
     *
     * @return the volume as integer in between 0 and 100.
     */
    int getVolume();

signals:
    /**
     * Sinal an update of the volume displayed.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    void volume(int vol);

public slots:
    /**
     * Set the volume displayed in the UI.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    virtual void setVolume(int vol) = 0;

protected:
    int currentVolume;
};

#endif
