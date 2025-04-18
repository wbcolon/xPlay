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

#ifndef __XPLAYERUI_H__
#define __XPLAYERUI_H__

#include <QGridLayout>

namespace xPlayer {

/*
 * UI configuration values
 */
// Widget configuration
const bool UseFlatGroupBox = true;
const double SelectorHeightFontFactor = 1.6;
const int SliderWidgetSliderOffset = 32;
const int VolumeWidgetWidth = 188;
const int VolumeWidgetHeight = 132;
const int VolumeWidgetLCDWidth = 40;
const int VolumeWidgetLCDHeight = 28;
const int ControlButtonWidgetWidth = 200;
const int SidebarWidgetWidth = 300;
// Main window configuration
const int ArtistListMinimumWidth = 350;
const int AlbumListMinimumWidth = 550;
const int TracksListMinimumWidth = 400;
const int QueueListMinimumWidth = 400;
const int DialogMinimumWidth = 400;
const int QueueCriticalNumberEntries = 50000;
// Icon sizes
const int IconSize = 24;
const int LargeIconSize = 32;
// Visualization configuration
const int VisualizationNoDrop = 0;
const int VisualizationDropRate = 3;
// Player configuration
const qint64 MusicTickDelta = 500;
const qint64 MusicTickDeltaIgnore = 250;
const qint64 MusicForwardRewindDelta = 5000;
const qint64 MovieTickDelta = 1000;
const qint64 MovieTickDeltaIgnore = 500;
const qint64 MovieFinishDelta = 1500;
const qint64 MovieForwardRewindDelta = 60000;

/**
 * Convert the time given to human-readable time stamp.
 *
 * @param ms the given time in ms.
 * @param showHours show hours if true, ms instead otherwise.
 * @return the human-readable time (hour:min:sec) or (min:sec.ms)as string.
 */
QString millisecondsToTimeFormat(qint64 ms, bool showHours);
/**
 * Convert the time given to short human-readable time stamp.
 *
 * @param ms the given time in ms.
 * @return the human-readable time (hour:min:sec) or (min:sec) as string.
 */
QString millisecondsToShortTimeFormat(qint64 ms);

}

class xPlayerLayout:public QGridLayout {
public:
    static const int NoSpace = 0;
    static const int TinySpace = 4;
    static const int SmallSpace = 8;
    static const int MediumSpace = 16;
    static const int LargeSpace = 24;
    static const int HugeSpace = 32;
    static const int SeparatorSpace = 48;

    xPlayerLayout();
    explicit xPlayerLayout(QWidget* parent);
    ~xPlayerLayout() override = default;

    void addRowSpacer(int row, int space);
    void addRowStretcher(int row);
    void addColumnSpacer(int column, int space);
    void addColumnStretcher(int column);
};


#endif
