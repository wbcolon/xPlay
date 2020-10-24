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
#ifndef __XPLAYERVOLUMEWIDGETX_H__
#define __XPLAYERVOLUMEWIDGETX_H__

#include "xPlayConfig.h"

#ifdef USE_QWT
#include "playerwidget/xPlayerVolumeWidgetQwt.h"
typedef xPlayerVolumeWidgetQwt xPlayerVolumeWidgetX;
#else
#include "playerwidget/xPlayerVolumeWidgetQt.h"
typedef xPlayerVolumeWidgetQt xPlayerVolumeWidgetX;
#endif
#endif

