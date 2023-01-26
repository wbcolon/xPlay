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

#include <QSlider>
#include <QString>
#include <QLabel>
#include <QLCDNumber>

#include <cmath>


class xPlayerSliderScaleWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerSliderScaleWidget(int offset, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerSliderScaleWidget() override = default;
    /**
     * Update the length label.
     *
     * @param length the length of the current track in milliseconds.
     */
    void setLength(qint64 length);
    /**
     * Set the maximum number of scale sections (therefore labels) allowed.
     *
     * @param scaleSections number of sections.
     */
    void useScaleSections(int scaleSections);
    /**
     * Determine the format used for the track slider.
     *
     * @param hourScale decide whether to include hours in the time format.
     */
    void useHourScale(bool hourScale);

protected:
    /**
     * Overload paint function. Add scale.
     */
    void paintEvent(QPaintEvent* event) override;

private:
    /**
     * Find a scale layout.
     *
     * @param length current length of the for the track slider.
     * @return the divider used for the track slider.
     */
    [[nodiscard]] qint64 determineScaleDivider(qint64 length) const;
    /**
     * Convert the value into a time output.
     *
     * @param value the time in ms.
     * @return the time output as string.
     */
    [[nodiscard]] QString scaleLabel(qint64 value) const;

    QSize labelSize;
    bool hourScale;
    int sliderX;
    int labelX;
    QRect labelBox;
    qint64 lengthValue;
    qint64 scaleDivider;
    qint64 maxScaleSections;
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
    void setLength(qint64 length);
    /**
     * Update the played time label.
     *
     * @param played the amount played of the current track in milliseconds.
     */
    void setPlayed(qint64 played);

signals:
    /**
     * Signal emitted whenever the slider is moved.
     *
     * @param position the new slider position.
     */
    void seek(qint64 position);

private:
    bool showHours;
    QSlider* slider;
    xPlayerSliderScaleWidget* scale;
    qint64 lengthValue;
    QLCDNumber* lengthLabel;
    QLCDNumber* playedLabel;
};

#endif
