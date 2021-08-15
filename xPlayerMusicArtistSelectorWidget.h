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

#ifndef __XPLAYERMUSICARTISTSELECTORWIDGET_H__
#define __XPLAYERMUSICARTISTSELECTORWIDGET_H__

#include <QWidget>
#include <QListWidget>
#include <QCheckBox>

#include <set>


class xPlayerMusicArtistSelectorWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMusicArtistSelectorWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMusicArtistSelectorWidget() override = default;
    /**
     * Update the list of selectors (and add special "none" and "random").
     *
     * @param selectors a set of characters (as QString) used for filtering artists.
     */
    void updateSelectors(const std::set<QString>& selectors);
    /**
     * Remove all selectors.
     */
    void clear();

signals:
    /**
     * Signal emitted if a selector in the list widget has been selected.
     *
     * @param text the current selector as string.
     */
    void selector(const QString& text);
    /**
     * Signal emitted if a selector in the list widget has been double-clicked.
     *
     * @param text the currently double-clicked selector as string.
     */
    void selectorDoubleClicked(const QString& text);
    /**
     * Signal emitted if the sorting latest checkbox state has changed.
     *
     * @param enabled sort after the latest write time if true, after name otherwise.
     */
    void sortingLatest(bool enabled);

private slots:
    /**
     * Triggered if a selector in the list widget is selected.
     *
     * Function will emit a "selector" signal with the currently selected selector.
     *
     * @param selectorIndex the index of the selected selector.
     */
    void selectArtistSelector(int selectorIndex);
    /**
     * Triggered if a selector in the list widget is double-clicked.
     *
     * Function will emit a "selector" signal if "random" has been double-clicked. Otherwise
     * a "selectorDoubleClicked" signal is emitted with the currently selected selector.
     *
     * @param selectorItem pointer to the list widget item that has been double-clicked.
     */
    void doubleClickedArtistSelector(QListWidgetItem* selectorItem);

private:
    QListWidget* selectorList;
    QCheckBox* sortingLatestBox;
};


#endif
