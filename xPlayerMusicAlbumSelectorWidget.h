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

#ifndef __XPLAYERMUSICALBUMSELECTORWIDGET_H__
#define __XPLAYERMUSICALBUMSELECTORWIDGET_H__

#include <QWidget>
#include <QListWidget>
#include <QDateTimeEdit>

#include <map>
#include <set>

class xPlayerMusicAlbumSelectorWidget: public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMusicAlbumSelectorWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMusicAlbumSelectorWidget() override = default;

    void setSelectors(const QStringList& selectors);

signals:
    void updatedSelectors(const QStringList& match, const QStringList& notMatch);
    void updatedDatabaseSelectors(const std::map<QString,std::set<QString>>& databaseMap, bool databaseNotMatch);
    void clearDatabaseSelectors();
    void clearAllSelectors();

private slots:
    void selectorClicked(QListWidgetItem* selectorItem);

private:
    void updateSelectors();

    enum xSelectorStates { StateNotUsed, StateMatch, StateNotMatch, StateSpacer };
    QVector<xSelectorStates> selectorState;
    QListWidget* selectorListWidget;
    QDateTimeEdit* timestampWidget;

    int currentUserSelectorIndex;
    int currentDatabaseSelectorIndex;
    qint64 currentTimestamp;
    std::map<QString,std::set<QString>> currentArtistAlbumMap;
};

#endif
