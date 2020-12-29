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

#ifndef __XPLAYERSELECTORWIDGET_H__
#define __XPLAYERSELECTORWIDGET_H__

#include <QListWidget>

class xPlayerSelectorWidget:public QListWidget {
    Q_OBJECT

public:
    explicit xPlayerSelectorWidget(QWidget* parent=nullptr);
    ~xPlayerSelectorWidget() override = default;

    void setSelectors(const QStringList& selectors);

signals:
    void updatedSelectors(const QStringList& match, const QStringList& notMatch);

private slots:
    void selectorClicked(QListWidgetItem* selectorItem);

private:
    void updateSelectors();

    enum xSelectorStates { StateNotUsed, StateMatch, StateNotMatch, StateSpacer };
    QVector<xSelectorStates> selectorState;
};

#endif
