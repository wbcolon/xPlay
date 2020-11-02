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

#include <QWidget>
#include <QString>

class xPlayerSliderWidget:public QWidget {
    Q_OBJECT

public:
    xPlayerSliderWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerSliderWidget() = default;

    /**
     * Return the state of hour mode.
     *
     * @return true if hours are enables in the scale, false otherwise.
     */
    bool hourScale();
    /**
     * Return the maxnumber of sections of the scale.
     * @return number of sections as integer.
     */
    int scaleSections();
    /**
     * Clear the state of the slider widget.
     */
    virtual void clear() = 0;

signals:
    void seek(qint64 position);

public slots:
    /**
     * Determine the format used for the track slider.
     *
     * @param hourScale decide whether to include hours in the time format.
     */
    virtual void useHourScale(bool hourScale);
    /**
     * Set the maximum number of scale sections (therefore labels) allowed.
     *
     * @param scaleSections number of sections.
     */
    virtual void useScaleSections(int scaleSections);
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    virtual void trackLength(qint64 length) = 0;
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    virtual void trackPlayed(qint64 played) = 0;

protected:
    /**
     * Convert milliseconds to a format string.
     *
     * @param ms time value in milliseconds.
     * @return format string "mm:ss.hh" or "hh:mm:ss.hh".
     */
    QString millisecondsToLabel(qint64 ms);
    /**
     * Find a scale layout.
     *
     * @param length current length of the for the track slider.
     * @return the divider used for the track slider.
     */
    int determineScaleDivider(int length);

    bool showHours;
    int maxScaleSections;
};

#endif
