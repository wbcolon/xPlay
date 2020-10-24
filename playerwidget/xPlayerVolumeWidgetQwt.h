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
#ifndef __XPLAYERVOLUMEWIDGETQWT_H__
#define __XPLAYERVOLUMEWIDGETQWT_H__

#include "xPlayerVolumeWidget.h"

#include <qwt/qwt_knob.h>

class xPlayerVolumeWidgetQwt:public xPlayerVolumeWidget {
    Q_OBJECT

public:
    xPlayerVolumeWidgetQwt(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerVolumeWidgetQwt() = default;

public slots:
    virtual void setVolume(int vol);

private:
    QwtKnob* volumeKnob;
};

#endif
