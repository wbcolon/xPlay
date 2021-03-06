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

const bool xPlayerUseFlatGroupBox = true;
const double xPlayerSelectorHeightFontFactor = 1.6;
const int xPlayerVolumeWidgetWidth = 168;
const int xPlayerControlButtonWidgetWidth = 200;

const int xPlayerArtistListMinimumWidth = 350;
const int xPlayerAlbumListMinimumWidth = 550;
const int xPlayerTracksListMinimumWidth = 400;
const int xPlayerQueueListMinimumWidth = 400;

const int xPlayerIconSize = 24;

class xPlayerLayout:public QGridLayout {
public:
    static const int NoSpace = 0;
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
