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

#ifndef __XPLAYERMUSICSEARCHWIDGET_H__
#define __XPLAYERMUSICSEARCHWIDGET_H__

#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>

class xPlayerMusicSearchWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerMusicSearchWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerMusicSearchWidget() override = default;
    /**
     * Retrieve the currently set match strings.
     *
     * @return a tuple of artist, album and track name match as string.
     */
    [[nodiscard]] std::tuple<QString,QString,QString> getMatch() const;

public slots:
    /**
     * Cleanup.
     *
     * Reset search category to album and clear search text.
     */
    void clear();

signals:
    /**
     * Signal emitted if clear button is clicked.
     */
    void clearFilter();
    /**
     * Signal emitted if search button is clicked.
     *
     * @param match the tuple of artist, album and track name match as string.
     */
    void updateFilter(const std::tuple<QString,QString,QString>& match);

private slots:
    /**
     * Called if search button is clicked. Emit updateFilter signal.
     */
    void searchClicked();
    /**
     * Called if the return key is pressed in the search text input.
     */
    void returnPressed();

private:
    QButtonGroup* searchCategoryGroup;
    QLineEdit* searchText;
    QRadioButton* searchArtistButton;
    QRadioButton* searchAlbumButton;
    QRadioButton* searchTrackButton;
    QPushButton* searchButton;
    QPushButton* clearButton;
    QString currentSearchText;
    int currentSearchCategory;
};


#endif
