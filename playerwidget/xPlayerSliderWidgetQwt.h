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
#ifndef __XPLAYERSLIDERWIDGETQWT_H__
#define __XPLAYERSLIDERWIDGETQWT_H__

#include "xPlayerSliderWidget.h"

#include <qwt/qwt_date_scale_draw.h>
#include <qwt/qwt_slider.h>
#include <QLabel>
#include <cmath>

/*
 * Helper class in order to adjust the scale labels (only QWT)
 */
class xPlayerWidgetScaleDraw:public QwtScaleDraw {
public:
    xPlayerWidgetScaleDraw() = default;
    ~xPlayerWidgetScaleDraw() = default;

    void useHourScale(bool hours) {
        showHours = hours;
    }

    virtual QwtText label(double value) const {
        if (showHours) {
            return QwtText(QString("%1:%2:%3").arg(static_cast<int>(round(value)/3600000)).
                    arg(static_cast<int>(round(value)/60000)%60, 2, 10, QChar('0')).
                    arg((static_cast<int>(round(value))/1000)%60, 2, 10, QChar('0')));
        } else {
            return QwtText(QString("%1:%2").arg(static_cast<int>(round(value)/60000)).
                    arg((static_cast<int>(round(value))/1000)%60, 2, 10, QChar('0')));
        }
    }
private:
    bool showHours;
};

class xPlayerSliderWidgetQwt:public xPlayerSliderWidget {
    Q_OBJECT

public:
    xPlayerSliderWidgetQwt(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerSliderWidgetQwt() = default;

    /**
     * Clear the state of the slider widget.
     */
    virtual void clear();

public slots:
    virtual void useHourScale(bool hours) override;
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    virtual void trackLength(qint64 length);
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    virtual void trackPlayed(qint64 length);

private:
    QwtSlider* trackSlider;
    xPlayerWidgetScaleDraw* scaleDraw;
    QLabel* trackLengthLabel;
    QLabel* trackPlayedLabel;
};

#endif
