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

#ifndef __XPLAYERBALANCEWIDGETQWT_H__
#define __XPLAYERBALANCEWIDGETQWT_H__

#include "xPlayerBalanceWidget.h"
#include <qwt/qwt_slider.h>

class xPlayerBalanceWidgetQwt:public xPlayerBalanceWidget {
    Q_OBJECT

public:
    explicit xPlayerBalanceWidgetQwt(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerBalanceWidgetQwt() override = default;

protected:
    /**
     * Update the slider and labels widget on balance changes.
     *
     * @param value the new balance as integer.
     */
    void updateSlider(int value) override;
    /**
     * Update the slider and labels widget on range changes.
     *
     * @param value the new range value as integer.
     */
    void updateSliderRange(int value) override;

    QwtSlider* balanceSlider;
};

#endif
