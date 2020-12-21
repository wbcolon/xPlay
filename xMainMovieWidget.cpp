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

#include <QGridLayout>
#include <QGroupBox>
#include <QApplication>

// Function addGroupBox has to be defined before the constructor due to the auto return.
auto xMainMovieWidget::addGroupBox(const QString& boxLabel, QWidget* parent) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, parent);
    auto list = new QListWidget(groupBox);
    auto boxLayout = new QHBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}


xMainMovieWidget::xMainMovieWidget(xMoviePlayer* player, QWidget* parent):
        QStackedWidget(parent),
        currentMovieName(),
        moviePlayer(player),
        fullWindow(false),
        autoPlayNextMovie(false) {
    // main widget with controls.
    mainWidget = new QWidget(parent);
    // Create group boxes for tags, directories and movies.
    auto [ tagBox, tagList_ ] = addGroupBox(tr("Tags"), mainWidget);
    auto [ directoryBox, directoryList_ ] = addGroupBox(tr("Directories"), mainWidget);
    auto [ movieBox, movieList_ ] = addGroupBox(tr("Movies"), mainWidget);
    // Create group box for the player widget.
    auto moviePlayerBox = new QGroupBox(tr("Player"), mainWidget);
    auto moviePlayerLayout = new QHBoxLayout();
    moviePlayerWidget = new xPlayerMovieWidget(moviePlayer, mainWidget);
    moviePlayerLayout->addWidget(moviePlayerWidget);
    moviePlayerBox->setLayout(moviePlayerLayout);
    // Setup tag, directory and movie lists.
    tagList = tagList_;
    tagList->setSortingEnabled(true);
    directoryList = directoryList_;
    directoryList->setSortingEnabled(true);
    movieList = movieList_;
    // Stacked widget setup for movie player.
    movieStack = new QStackedWidget(mainWidget);
    movieStack->addWidget(new QWidget(mainWidget));
    moviePlayer->setParent(mainWidget);
    movieStack->addWidget(moviePlayer);
    movieStack->setCurrentIndex(0);
    // Layout for the main movie widget.
    auto movieLayout = new QGridLayout(mainWidget);
    movieLayout->addWidget(moviePlayerBox, 0, 0, 2, 5);
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
    connect(this, &xMainMovieWidget::setMovie, moviePlayer, &xMoviePlayer::setMovie);
    connect(this, &xMainMovieWidget::setMovieQueue, moviePlayer, &xMoviePlayer::setMovieQueue);
    connect(this, &xMainMovieWidget::clearMovieQueue, moviePlayer, &xMoviePlayer::clearMovieQueue);
    connect(moviePlayer, &xMoviePlayer::currentMoviePath, this, &xMainMovieWidget::updateSelectedMovie);
    connect(moviePlayer, &xMoviePlayer::currentMovieName, moviePlayerWidget, &xPlayerMovieWidget::currentMovie);
    connect(moviePlayer, &xMoviePlayer::currentMovieName, this, &xMainMovieWidget::updateWindowTitle);
    connect(moviePlayer, &xMoviePlayer::currentMovieLength, moviePlayerWidget, &xPlayerMovieWidget::currentMovieLength);
    connect(moviePlayer, &xMoviePlayer::currentMoviePlayed, moviePlayerWidget, &xPlayerMovieWidget::currentMoviePlayed);
    connect(moviePlayer, &xMoviePlayer::currentSubtitles, moviePlayerWidget, &xPlayerMovieWidget::currentSubtitles);
    connect(moviePlayer, &xMoviePlayer::currentAudioChannels, moviePlayerWidget, &xPlayerMovieWidget::currentAudioChannels);
    connect(moviePlayer, &xMoviePlayer::currentState, moviePlayerWidget, &xPlayerMovieWidget::currentState);
    // Update stack widget based on player state.
    connect(moviePlayer, &xMoviePlayer::currentState, this, &xMainMovieWidget::currentState);
    // Connect full window.
    connect(moviePlayerWidget, &xPlayerMovieWidget::toggleFullWindow, this, &xMainMovieWidget::toggleFullWindow);
    connect(moviePlayer, &xMoviePlayer::toggleFullWindow, this, &xMainMovieWidget::toggleFullWindow);
    connect(moviePlayerWidget, &xPlayerMovieWidget::autoPlayNextMovie, this, &xMainMovieWidget::setAutoPlayNextMovie);
    // Prepare Stack
    addWidget(mainWidget);
    setCurrentWidget(mainWidget);
}

void xMainMovieWidget::initializeView() {
    updateWindowTitle(currentMovieName);
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
    auto title = QApplication::applicationName();
    fullWindow = mode;
    if (mode) {
        movieStack->removeWidget(moviePlayer);
        moviePlayer->setParent(this);
        addWidget(moviePlayer);
        setCurrentWidget(moviePlayer);
        title += " - " + currentMovieName;
    } else {
        removeWidget(moviePlayer);
        moviePlayer->setParent(mainWidget);
        movieStack->addWidget(moviePlayer);
        movieStack->setCurrentWidget(moviePlayer);
        setCurrentWidget(mainWidget);
    }
    emit showWindowTitle(title);
    emit showMenuBar(!fullWindow);
}

void xMainMovieWidget::scannedTags(const QStringList& tags) {
    tagList->clear();
    tagList->addItems(tags);
}

void xMainMovieWidget::scannedDirectories(const QStringList& directories) {
    directoryList->clear();
    if (!directories.isEmpty()) {
        directoryList->addItem(".");
        directoryList->addItems(directories);
    } else {
        auto currentTag = tagList->currentItem();
        if (currentTag) {
            emit scanForTagAndDirectory(currentTag->text(), ".");
        }
    }
}

void xMainMovieWidget::scannedMovies(const std::vector<std::pair<QString,QString>>& movies) {
    movieList->clear();
    currentMovies.clear();
    for (const auto& movie : movies) {
        movieList->addItem(movie.first);
        currentMovies.push_back(movie.second);
    }
    qDebug() << "xMainMovieWidget: no of scanned movies: " << movies.size();
}

void xMainMovieWidget::selectTag(int index) {
    if ((index >= 0) && (index < tagList->count())) {
        directoryList->clear();
        movieList->clear();
        emit scanForTag(tagList->item(index)->text());
    }
}

void xMainMovieWidget::selectDirectory(int index) {
    if ((index >= 0) && (index < directoryList->count())) {
        movieList->clear();
        auto tag = tagList->currentItem()->text();
        auto directory = directoryList->item(index)->text();
        emit scanForTagAndDirectory(tag, directory);
    }
}

void xMainMovieWidget::selectMovie(QListWidgetItem* movieItem) {
    auto movieIndex = movieList->row(movieItem);
    if ((movieIndex >= 0) && (movieIndex < currentMovies.size())) {
        updateMovieQueue(movieIndex);
        emit setMovie(currentMovies[movieIndex], movieItem->text());
    }
}

void xMainMovieWidget::updateSelectedMovie(const QString& path) {
    auto movieIndex = movieList->currentRow();
    auto pathIndex = currentMovies.indexOf(path);
    // Update selection only if pathIndex is valid and not the currently selected index.
    if ((pathIndex != movieIndex) && (pathIndex >= 0) && (pathIndex < movieList->count())) {
        movieList->setCurrentItem(movieList->item(pathIndex));
    }
}

void xMainMovieWidget::setAutoPlayNextMovie(bool mode) {
    autoPlayNextMovie = mode;
    updateMovieQueue(movieList->currentRow());
}

void xMainMovieWidget::updateMovieQueue(int index) {
    if ((index >= 0) && (index < currentMovies.size())) {
        if (autoPlayNextMovie) {
            QList<std::pair<QString,QString>> queue;
            for (auto i = index+1; i < currentMovies.size(); ++i) {
                qDebug() << "QUEUE: " << movieList->item(i)->text();
                queue.push_back(std::make_pair(currentMovies[i], movieList->item(i)->text()));
            }
            emit setMovieQueue(queue);
        } else {
            emit clearMovieQueue();
        }
    } else {
        emit clearMovieQueue();
    }
}

void xMainMovieWidget::updateWindowTitle(const QString& name) {
    currentMovieName = name;
    if (fullWindow) {
        emit showWindowTitle(QApplication::applicationName()+" - "+currentMovieName);
    }
}

void xMainMovieWidget::currentState(xMoviePlayer::State state) {
    qDebug() << "xMainMovieWidget: state: " << state;
    switch (state) {
        case xMoviePlayer::PlayingState:
        case xMoviePlayer::PauseState: {
            movieStack->setCurrentIndex(1);
        } break;
        case xMoviePlayer::StoppingState: {
            setFullWindow(false);
        } break;
        case xMoviePlayer::StopState: {
            setFullWindow(false);
            movieStack->setCurrentIndex(0);
        } break;
    }
}