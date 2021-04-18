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

#include "xMainMovieWidget.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"
#include "xPlayerDatabase.h"

#include <QGroupBox>
#include <QApplication>
#include <QDateTime>

// Function addGroupBox has to be defined before the constructor due to the auto return.
auto xMainMovieWidget::addGroupBox(const QString& boxLabel, QWidget* parent) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, parent);
    groupBox->setFlat(xPlayerUseFlatGroupBox);
    auto list = new xPlayerListWidget(groupBox);
    auto boxLayout = new QHBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}


xMainMovieWidget::xMainMovieWidget(xMoviePlayerX* player, QWidget* parent):
        QStackedWidget(parent),
        currentMovieName(),
        currentMovieTag(),
        currentMovieDirectory(),
        moviePlayer(player),
        fullWindow(false),
        autoPlayNextMovie(false),
        useDatabaseMovieOverlay(true),
        databaseCutOff(0) {
    // main widget with controls.
    mainWidget = new QWidget(parent);
    // Create group boxes for tags, directories and movies.
    auto [ tagBox, tagList_ ] = addGroupBox(tr("Tags"), mainWidget);
    auto [ directoryBox, directoryList_ ] = addGroupBox(tr("Directories"), mainWidget);
    auto [ movieBox, movieList_ ] = addGroupBox(tr("Movies"), mainWidget);
    // Create group box for the player widget.
    moviePlayerWidget = new xPlayerMovieWidget(moviePlayer, mainWidget);
    // Setup tag, directory and movie lists.
    tagList = tagList_;
    tagList->enableSorting(true);
    directoryList = directoryList_;
    directoryList->enableSorting(true);
    movieList = movieList_;
    // Stacked widget setup for movie player.
    movieStack = new QStackedWidget(mainWidget);
    movieStack->addWidget(new QWidget(mainWidget));
    moviePlayer->setParent(mainWidget);
    movieStack->addWidget(moviePlayer);
    movieStack->setCurrentIndex(0);
    // Layout for the main movie widget.
    auto movieLayout = new xPlayerLayout(mainWidget);
    movieLayout->addWidget(moviePlayerWidget, 0, 0, 2, 5);
    movieLayout->addWidget(movieStack, 2, 0, 8, 5);
    movieLayout->addWidget(tagBox, 0, 5, 2, 2);
    movieLayout->addWidget(directoryBox, 2, 5, 2, 2);
    movieLayout->addWidget(movieBox, 4, 5, 6, 2);
    movieLayout->setColumnStretch(0, 6);
    movieLayout->setColumnStretch(6, 2);
    movieLayout->setRowStretch(8, 2);
    // Connect signals.
    connect(tagList, &QListWidget::currentRowChanged, this, &xMainMovieWidget::selectTag);
    connect(directoryList, &QListWidget::currentRowChanged, this, &xMainMovieWidget::selectDirectory);
    connect(movieList, &QListWidget::itemDoubleClicked, this, &xMainMovieWidget::selectMovie);
    // Connect to movie player.
    connect(this, &xMainMovieWidget::setMovie, moviePlayer, &xMoviePlayerX::setMovie);
    connect(this, &xMainMovieWidget::setMovieQueue, moviePlayer, &xMoviePlayerX::setMovieQueue);
    connect(this, &xMainMovieWidget::clearMovieQueue, moviePlayer, &xMoviePlayerX::clearMovieQueue);
    connect(moviePlayer, &xMoviePlayerX::currentMovie, this, &xMainMovieWidget::updateSelectedMovie);
    connect(moviePlayer, &xMoviePlayerX::currentMovie, moviePlayerWidget, &xPlayerMovieWidget::currentMovie);
    connect(moviePlayer, &xMoviePlayerX::currentMovie, this, &xMainMovieWidget::updateWindowTitle);
    connect(moviePlayer, &xMoviePlayerX::currentMovieLength, moviePlayerWidget, &xPlayerMovieWidget::currentMovieLength);
    connect(moviePlayer, &xMoviePlayerX::currentMoviePlayed, moviePlayerWidget, &xPlayerMovieWidget::currentMoviePlayed);
    connect(moviePlayer, &xMoviePlayerX::currentSubtitles, moviePlayerWidget, &xPlayerMovieWidget::currentSubtitles);
    connect(moviePlayer, &xMoviePlayerX::currentAudioChannels, moviePlayerWidget, &xPlayerMovieWidget::currentAudioChannels);
    connect(moviePlayer, &xMoviePlayerX::currentState, moviePlayerWidget, &xPlayerMovieWidget::currentState);
    // Update stack widget based on player state.
    connect(moviePlayer, &xMoviePlayerX::currentState, this, &xMainMovieWidget::currentState);
    // Connect full window.
    connect(moviePlayerWidget, &xPlayerMovieWidget::toggleFullWindow, this, &xMainMovieWidget::toggleFullWindow);
    connect(moviePlayer, &xMoviePlayerX::toggleFullWindow, this, &xMainMovieWidget::toggleFullWindow);
    connect(moviePlayerWidget, &xPlayerMovieWidget::autoPlayNextMovie, this, &xMainMovieWidget::setAutoPlayNextMovie);
    // Connect database.
    connect(moviePlayer, &xMoviePlayerX::currentMovie, this, &xMainMovieWidget::currentMovie);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMovieOverlay,
            this, &xMainMovieWidget::updatedDatabaseMovieOverlay);
    // Prepare Stack
    addWidget(mainWidget);
    setCurrentWidget(mainWidget);
}

void xMainMovieWidget::initializeView() {
    updateWindowTitle(QString(), currentMovieName, currentMovieTag, currentMovieDirectory);
    emit showMenuBar(!fullWindow);
}

void xMainMovieWidget::clear() {
    setFullWindow(false);
    // Clear queue.
    moviePlayer->stop();
    moviePlayerWidget->clear();
    // Clear tags, directories and movies lists.
    scannedTags(QStringList());
    scannedDirectories(QStringList());
    scannedMovies(std::vector<std::pair<QString,QString>>());
}

void xMainMovieWidget::toggleFullWindow() {
    // Toggle the mode.
    setFullWindow(!fullWindow);
}

void xMainMovieWidget::setFullWindow(bool mode) {
    qDebug() << "xMainMovieWidget: setFullWindow: " << mode;
    // If mode is already correct then do nothing.
    if (fullWindow == mode) {
        return;
    }
    fullWindow = mode;
    if (mode) {
        movieStack->removeWidget(moviePlayer);
        moviePlayer->setParent(this);
        addWidget(moviePlayer);
        setCurrentWidget(moviePlayer);
    } else {
        removeWidget(moviePlayer);
        moviePlayer->setParent(mainWidget);
        movieStack->addWidget(moviePlayer);
        movieStack->setCurrentWidget(moviePlayer);
        setCurrentWidget(mainWidget);
    }
    emit showWindowTitle(createWindowTitle());
    emit showMenuBar(!fullWindow);
}

void xMainMovieWidget::scannedTags(const QStringList& tags) {
    tagList->clearItems();
    tagList->addItemWidgets(tags);
    updatePlayedTags();
}

void xMainMovieWidget::scannedDirectories(const QStringList& directories) {
    directoryList->clearItems();
    if (!directories.isEmpty()) {
        directoryList->addItemWidget(".");
        directoryList->addItemWidgets(directories);
    } else {
        auto currentTag = tagList->currentItemWidget();
        if (currentTag) {
            emit scanForTagAndDirectory(currentTag->text(), ".");
        }
    }
    updatePlayedDirectories();
}

void xMainMovieWidget::scannedMovies(const std::vector<std::pair<QString,QString>>& movies) {
    movieList->clearItems();
    currentMovies.clear();
    for (const auto& movie : movies) {
        movieList->addItemWidget(movie.first);
        currentMovies.push_back(movie.second);
    }
    updatePlayedMovies();
    qDebug() << "xMainMovieWidget: no of scanned movies: " << movies.size();
}

void xMainMovieWidget::selectTag(int index) {
    if ((index >= 0) && (index < tagList->count())) {
        directoryList->clearItems();
        movieList->clearItems();
        emit scanForTag(tagList->itemWidget(index)->text());
    }
}

void xMainMovieWidget::selectDirectory(int index) {
    if ((index >= 0) && (index < directoryList->count())) {
        movieList->clearItems();
        auto tag = tagList->currentItemWidget()->text();
        auto directory = directoryList->itemWidget(index)->text();
        emit scanForTagAndDirectory(tag, directory);
    }
}

void xMainMovieWidget::selectMovie(QListWidgetItem* movieItem) {
    auto movieIndex = movieList->row(movieItem);
    if ((movieIndex >= 0) && (movieIndex < currentMovies.size())) {
        QString tag = tagList->currentItemWidget()->text();
        QString directory { "." };
        if (directoryList->count() > 0) {
            directory = directoryList->currentItemWidget()->text();
        }
        updateMovieQueue(movieIndex);
        emit setMovie(currentMovies[movieIndex], movieList->itemWidget(movieIndex)->text(), tag, directory);
    }
}

void xMainMovieWidget::updateSelectedMovie(const QString& path, const QString& name, const QString& tag, const QString& directory) {
    Q_UNUSED(name)
    Q_UNUSED(tag)
    Q_UNUSED(directory)
    auto movieIndex = movieList->currentRow();
    auto pathIndex = currentMovies.indexOf(path);
    // Update selection only if pathIndex is valid and not the currently selected index.
    if ((pathIndex != movieIndex) && (pathIndex >= 0) && (pathIndex < movieList->count())) {
        movieList->setCurrentItem(movieList->item(pathIndex));
        updatePlayedMovies();
    }
}

void xMainMovieWidget::setAutoPlayNextMovie(bool mode) {
    autoPlayNextMovie = mode;
    updateMovieQueue(movieList->currentRow());
}

void xMainMovieWidget::updatedDatabaseMovieOverlay() {
    useDatabaseMovieOverlay = xPlayerConfiguration::configuration()->getDatabaseMovieOverlay();
    databaseCutOff = xPlayerConfiguration::configuration()->getDatabaseCutOff();
    if (useDatabaseMovieOverlay) {
        updatePlayedTags();
        updatePlayedDirectories();
        updatePlayedMovies();
    } else {
        // Clear tag list.
        for (auto i = 0; i < tagList->count(); ++i) {
            tagList->itemWidget(i)->removeIcon();
        }
        // Clear directory list.
        for (auto i = 0; i < directoryList->count(); ++i) {
            directoryList->itemWidget(i)->removeIcon();
        }
        // Clear movie list.
        for (auto i = 0; i < movieList->count(); ++i) {
            movieList->itemWidget(i)->removeIcon();
            movieList->itemWidget(i)->removeToolTip();
        }
    }
}

void xMainMovieWidget::updateMovieQueue(int index) {
    if ((index >= 0) && (index < currentMovies.size())) {
        if (autoPlayNextMovie) {
            QList<std::pair<QString,QString>> queue;
            for (auto i = index+1; i < currentMovies.size(); ++i) {
                qDebug() << "QUEUE: " << movieList->itemWidget(i)->text();
                queue.push_back(std::make_pair(currentMovies[i], movieList->itemWidget(i)->text()));
            }
            QString tag = tagList->currentItemWidget()->text();
            QString directory { "." };
            if (directoryList->count() > 0) {
                directory = directoryList->currentItemWidget()->text();
            }
            emit setMovieQueue(queue, tag, directory);
        } else {
            emit clearMovieQueue();
        }
    } else {
        emit clearMovieQueue();
    }
}

void xMainMovieWidget::updatePlayedTags() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMovieOverlay) {
        return;
    }
    auto playedTags = xPlayerDatabase::database()->getPlayedTags(databaseCutOff);
    for (auto i = 0; i < tagList->count(); ++i) {
        auto tagItem = tagList->itemWidget(i);
        auto tag = tagItem->text();
        // Clear icon and tooltip.
        tagItem->removeIcon();
        tagItem->removeToolTip();
        for (const auto& playedTag : playedTags) {
            // Update icon and tooltip if movie already played.
            if (playedTag == tag) {
                tagItem->setIcon(":images/xplay-star.svg");
                break;
            }
        }
    }
}

void xMainMovieWidget::updatePlayedDirectories() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMovieOverlay) {
        return;
    }
    auto tagItem = tagList->currentItemWidget();
    if ((!tagItem) || (directoryList->count() == 0)) {
        return;
    }
    auto tag = tagItem->text();
    auto playedDirectories = xPlayerDatabase::database()->getPlayedDirectories(tag, databaseCutOff);
    for (auto i = 0; i < directoryList->count(); ++i) {
        auto directoryItem = directoryList->itemWidget(i);
        auto directory = directoryItem->text();
        // Clear icon and tooltip.
        directoryItem->removeIcon();
        directoryItem->removeToolTip();
        for (const auto& playedDirectory : playedDirectories) {
            // Update icon and tooltip if movie already played.
            if (playedDirectory == directory) {
                directoryItem->setIcon(":images/xplay-star.svg");
                break;
            }
        }
    }
}

void xMainMovieWidget::updatePlayedMovies() {
    // Only update if database overlay is enabled. Exit otherwise.
    if (!useDatabaseMovieOverlay) {
        return;
    }
    auto tagItem = tagList->currentItemWidget();
    // No tag currently selected. We do not need to update.
    if (!tagItem) {
        return;
    }
    auto tag = tagItem->text();
    auto directory = (directoryList->count() > 0) ? directoryList->currentItemWidget()->text() : ".";
    auto playedMovies = xPlayerDatabase::database()->getPlayedMovies(tag, directory, databaseCutOff);
    for (auto i = 0; i < movieList->count(); ++i) {
        auto movieItem = movieList->itemWidget(i);
        auto movie = movieItem->text();
        // Clear icon and tooltip.
        movieItem->removeIcon();
        movieItem->removeToolTip();
        for (auto playedMovie = playedMovies.begin(); playedMovie != playedMovies.end(); ++playedMovie) {
            // Update icon and tooltip if movie already played.
            if (std::get<0>(*playedMovie) == movie) {
                movieItem->setIcon(":images/xplay-star.svg");
                // Adjust tooltip to play count "once" vs "x times".
                auto playCount = std::get<1>(*playedMovie);
                if (playCount > 1) {
                    movieItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                            arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(std::get<2>(*playedMovie))).toString(Qt::DefaultLocaleLongDate)));
                } else {
                    movieItem->addToolTip(QString(tr("played once, last time on %1")).
                            arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(std::get<2>(*playedMovie))).toString(Qt::DefaultLocaleLongDate)));
                }
                // Remove element to speed up search in the next iteration.
                playedMovies.erase(playedMovie);
                // End update if no more movies need to be marked.
                if (playedMovies.isEmpty()) {
                    return;
                }
                // Break loop and move on to the next movie item.
                break;
            }
        }
    }
}

void xMainMovieWidget::updatePlayedMovie(const QString& tag, const QString& directory,
                                         const QString& movie, int playCount, quint64 timeStamp) {
    // Only update if the database overlay is enabled.
    if (!useDatabaseMovieOverlay) {
        return;
    }
    auto tagItem = tagList->currentItemWidget();
    // Update the tags.
    auto tagPlayedItems = tagList->findItemWidgets(tag);
    for (auto& tagPlayedItem : tagPlayedItems) {
        tagPlayedItem->setIcon(":images/xplay-star.svg");
    }
    // If no tag selected or the tag does not match the selected
    // tags then we do not need to update the albums.
    if ((!tagItem) || (tagItem->text() != tag)) {
        return;
    }
    // Handle special case if we only have the "." directory.
    if (directoryList->count() > 0) {
        auto directoryItem = directoryList->currentItemWidget();
        // Update the directorys.
        auto directoryPlayedItems = directoryList->findItemWidgets(directory);
        for (auto& directoryPlayedItem : directoryPlayedItems) {
            directoryPlayedItem->setIcon(":images/xplay-star.svg");
        }
        // If no directory selected or the directory does not match the selected
        // directory then we do not need to update the tracks.
        if ((!directoryItem) || (directoryItem->text() != directory)) {
            return;
        }
    }
    auto moviePlayedItems = movieList->findItemWidgets(movie);
    for (auto& moviePlayedItem : moviePlayedItems) {
        moviePlayedItem->setIcon(":images/xplay-star.svg");
        if (playCount > 1) {
            moviePlayedItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                    arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timeStamp)).toString(Qt::DefaultLocaleLongDate)));
        } else {
            moviePlayedItem->addToolTip(QString(tr("played once, last time on %1")).
                    arg(QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timeStamp)).toString(Qt::DefaultLocaleLongDate)));
        }
    }
}

void xMainMovieWidget::updateWindowTitle(const QString& path, const QString& name, const QString& tag, const QString& directory) {
    Q_UNUSED(path)
    currentMovieName = name;
    currentMovieTag = tag;
    currentMovieDirectory = directory;
    if (fullWindow) {
        emit showWindowTitle(createWindowTitle());
    }
}

QString xMainMovieWidget::createWindowTitle() {
    if (fullWindow) {
        if (currentMovieDirectory == ".") {
            return QString("%1 - (%2) - %3").arg(QApplication::applicationName(), currentMovieTag, currentMovieName);
        } else {
            return QString("%1 - (%2/%3) - %4").arg(QApplication::applicationName(), currentMovieTag, currentMovieDirectory, currentMovieName);
        }
    } else {
        return QApplication::applicationName();
    }
}

void xMainMovieWidget::currentState(xMoviePlayerX::State state) {
    qDebug() << "xMainMovieWidget: state: " << state;
    switch (state) {
        case xMoviePlayerX::PlayingState:
        case xMoviePlayerX::PauseState: {
            movieStack->setCurrentIndex(1);
        } break;
        case xMoviePlayerX::StoppingState: {
            setFullWindow(false);
        } break;
        case xMoviePlayerX::StopState: {
            setFullWindow(false);
            movieStack->setCurrentIndex(0);
        } break;
    }
}

void xMainMovieWidget::currentMovie(const QString& path, const QString& name,
                                    const QString& tag, const QString& directory) {
    Q_UNUSED(path)
    // Update database.
    auto result = xPlayerDatabase::database()->updateMovieFile(name, tag, directory);
    if (result.second > 0) {
        // Update database overlay.
        updatePlayedMovie(tag, directory, name, result.first, result.second);
    }
}
