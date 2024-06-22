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

#ifndef __XPLAYERARTISTINFO_H__
#define __XPLAYERARTISTINFO_H__

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QAction>
#include <QWebEngineView>

class xPlayerArtistInfo:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerArtistInfo(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerArtistInfo() override = default;

public slots:
    /**
     * Show the webpage for the given artist.
     *
     * @param artist the artist name as string.
     */
    void show(const QString& artist);

signals:
    /**
     * Signal emitted if close button is pressed.
     */
    void close();

private slots:
    /**
     * Called in the URL is being updated.
     *
     * @param url the new URL.
     */
    void urlChanged(const QUrl& url);
    /**
     * Called if return is pressed in the URL line edit.
     */
    void urlUpdated();
    /**
     * Called if the website was loaded.
     */
    void urlLoadFinished(bool ok);
    /**
     * Called if the line edit bookmark action is toggled.
     *
     * @param enabled true if enabled, false otherwise.
     */
    void bookmarkToggled(bool enabled);
    /**
     * Called if the zoom factor is selected.
     *
     * @param index the index of selected zoom factor.
     */
    void zoomFactorChanged(int index);
    /**
     * Called if the zoom factor was changed in the configuration.
     */
    void updatedZoomFactor();

private:
    QLineEdit* urlEdit;
    QAction* urlEditAction;
    QWebEngineView* urlView;
    QPushButton* homeButton;
    QComboBox* zoomBox;
    QString artistName;
    double zoomFactor;
};


#endif
