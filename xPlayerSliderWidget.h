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
#ifndef __XPLAYERSLIDERWIDGET_H__
#define __XPLAYERSLIDERWIDGET_H__

#include <qwt/qwt_date_scale_draw.h>
#include <qwt/qwt_slider.h>

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QLCDNumber>

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

class xPlayerSliderWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerSliderWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerSliderWidget() override = default;
    /**
     * Return the state of hour mode.
     *
     * @return true if hours are enables in the scale, false otherwise.
     */
    [[nodiscard]] bool hourScale() const;
    /**
     * Return the maxnumber of sections of the scale.
     * @return number of sections as integer.
     */
    [[nodiscard]] int scaleSections() const;
    /**
     * Clear the state of the slider widget.
     */
    void clear();

public slots:
    /**
     * Determine the format used for the track slider.
     *
     * @param hourScale decide whether to include hours in the time format.
     */
    void useHourScale(bool hourScale);
    /**
     * Set the maximum number of scale sections (therefore labels) allowed.
     *
     * @param scaleSections number of sections.
     */
    void useScaleSections(int scaleSections);
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    void trackLength(qint64 length);
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    void trackPlayed(qint64 played);

signals:
    /**
     * Signal emitted whenever the slider is moved.
     *
     * @param position the new slider position.
     */
    void seek(qint64 position);

private:
    /**
     * Find a scale layout.
     *
     * @param length current length of the for the track slider.
     * @return the divider used for the track slider.
     */
    [[nodiscard]] int determineScaleDivider(int length) const;

    bool showHours;
    int maxScaleSections;
    QwtSlider* trackSlider;
    xPlayerWidgetScaleDraw* scaleDraw;
    QLCDNumber* trackLengthLabel;
    QLCDNumber* trackPlayedLabel;
};

#endif
