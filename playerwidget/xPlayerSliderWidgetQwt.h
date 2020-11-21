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
    xPlayerWidgetScaleDraw():
            QwtScaleDraw(),
            showHours(false) { }
    ~xPlayerWidgetScaleDraw() override = default;

    void useHourScale(bool hours) {
        showHours = hours;
    }

    [[nodiscard]] QwtText label(double value) const override {
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
    explicit xPlayerSliderWidgetQwt(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerSliderWidgetQwt() override = default;

    /**
     * Clear the state of the slider widget.
     */
    void clear() override;

public slots:
    void useHourScale(bool hours) override;
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    void trackLength(qint64 length) override;
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    void trackPlayed(qint64 length) override;

private:
    QwtSlider* trackSlider;
    xPlayerWidgetScaleDraw* scaleDraw;
    QLabel* trackLengthLabel;
    QLabel* trackPlayedLabel;
};

#endif
