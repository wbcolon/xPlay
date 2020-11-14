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
    /**
     * Set the volume displayed in the UI.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    virtual void setVolume(int vol);
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    virtual void setMuted(bool mute);

protected:
    /**
     * Re-implement mouseDoubleClickEvent in order to implement mute functionality.
     *
     * @param event the event passed to the widget.
     */
    virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
    QwtKnob* volumeKnob;
};

#endif
