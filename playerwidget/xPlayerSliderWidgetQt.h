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
#ifndef __XPLAYERSLIDERWIDGETQT_H__
#define __XPLAYERSLIDERWIDGETQT_H__

#include "xPlayerSliderWidget.h"

#include <QSlider>
#include <QLabel>

class xPlayerSliderWidgetQt:public xPlayerSliderWidget {
    Q_OBJECT

public:
    explicit xPlayerSliderWidgetQt(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerSliderWidgetQt() override = default;

    /**
     * Clear the state of the slider widget.
     */
    void clear() override;

public slots:
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
    void trackPlayed(qint64 played) override;

private:
    QSlider* trackSlider;
    QLabel* trackLengthLabel;
    QLabel* trackPlayedLabel;
};

#endif
