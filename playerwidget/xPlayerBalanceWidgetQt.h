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

#ifndef __XPLAYERBALANCEWIDGETQT_H__
#define __XPLAYERBALANCEWIDGETQT_H__

#include "xPlayerBalanceWidget.h"
#include <QSlider>

class xPlayerBalanceWidgetQt:public xPlayerBalanceWidget {
    Q_OBJECT

public:
    explicit xPlayerBalanceWidgetQt(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerBalanceWidgetQt() override = default;

protected:
    void updateSlider(int value) override;
    void updateSliderRange(int value) override;

    QSlider* balanceSlider;
};

#endif
