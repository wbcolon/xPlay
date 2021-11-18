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
    /**
     * Setup the selectors displayed in in the selector widget.
     *
     * @param selectors a list strings of selectors.
     */
    void setSelectors(const QStringList& selectors);

public slots:
    /**
     * Clear all selectors.
     */
    void clear();

signals:
    /**
     * Signal emitted if one of the tag selectors has been modified.
     *
     * @param match a list of strings of matching selectors.
     * @param notMatch a list of strings of non-matching selectors.
     */
    void updatedSelectors(const QStringList& match, const QStringList& notMatch);
    /**
     * Signal emitted if the database selector has been updated.
     *
     * @param databaseMap the map of artist and sets of albums that match the database query.
     * @param databaseNotMatch invert the databaseMap if true in matching.
     */
    void updatedDatabaseSelectors(const std::map<QString,std::set<QString>>& databaseMap, bool databaseNotMatch);
    /**
     * Signal emitted if the database selectors are cleared.
     */
    void clearDatabaseSelectors();
    /**
     * Signal emitted if a clear of all selectors is performed.
     */
    void clearAllSelectors();

private slots:
    /**
     * Update selector if double-clicked.
     *
     * @param selectorItem pointer to the selector item clicked.
     */
    void selectorClicked(QListWidgetItem* selectorItem);

private:
    /**
     * Update selectors and emit corresponding signals.
     */
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
