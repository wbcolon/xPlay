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

#ifndef __XMAINSTREAMINGWIDGET_H__
#define __XMAINSTREAMINGWIDGET_H__

#include <QWidget>
#include <QUrl>
#include <QList>
#include <QWebEngineView>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

class xMainStreamingWidget:public QWidget {
    Q_OBJECT

public:
    explicit xMainStreamingWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xMainStreamingWidget() override = default;
    /**
     * Perform initial commands required when switching to this view.
     */
    void initializeView();

    /**
     * Set the sites accessible within this widget.
     *
     * @param sites list of pairs of short name and URL.
     */
    void setSites(const QList<std::pair<QString,QUrl>>& sites);
    /**
     * Set the default streaming site to be loaded.
     *
     * @param site a pair of short name and URL.
     */
    void setSitesDefault(const std::pair<QString,QUrl>& site);
    /**
     * Retrieve current list of sites.
     *
     * @return list of pairs of short name and URL.
     */
    [[nodiscard]] const QList<std::pair<QString,QUrl>>& getSites() const;
    /**
     * Retrieve current streaming site default.
     *
     * @return a pairs of short name and URL.
     */
    [[nodiscard]] const std::pair<QString,QUrl>& getSitesDefault() const;
    /**
     * Return the mute state for webengine.
     *
     * @return true if music player is muted, false otherwise.
     */
    [[nodiscard]] bool isMuted() const;

public slots:
    /**
     * Set the mute mode for the webengine.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute);

signals:
    /**
     * Signal emitted to notify main widget to update the window title.
     *
     * @param title the new window title as string.
     */
    void showWindowTitle(const QString& title);
    /**
     * Signal emitted to notify main widget to show/hide the menu bar.
     *
     * @param menu show menu bar if true, hide otherwise.
     */
    void showMenuBar(bool menu);

private slots:
    /**
     * Update the current site based on the selected entry in the Combo Box.
     *
     * @param index the position of the entry currently selected.
     */
    void updateCurrentSites(int index);
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
     * Update the zoom factor based on the selected entry in the Combo Box.
     *
     * @param index the position of the entry currently selected.
     */
    void updateZoomFactor(int index);
    /**
     * Clear the data for the currently selected site.
     *
     * @param history clear history if true.
     * @param cookies clear cookies if true.
     * @param cache clear cache if true.
     */
    void clearData(bool history, bool cookies, bool cache);

private:
    QWebEngineView* streamingWebView;
    QLineEdit* streamingUrl;
    QComboBox* sitesCombo;
    QList<std::pair<QString,QUrl>> streamingSites;
    std::pair<QString,QUrl> streamingSitesDefault;
    std::pair<QString,QUrl> currentSite;
    double zoomFactor;
};

#endif
