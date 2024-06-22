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
#include "xMovieLibrary.h"

#include <QGroupBox>
#include <QApplication>
#include <QDateTime>
#include <QSplitter>

// Function addGroupBox has to be defined before the constructor due to the auto return.
auto xMainMovieWidget::addGroupBox(const QString& boxLabel, bool displayTime, QWidget* parent) {
    // Create a QGroupBox with the given label and embed
    // a QListWidget.
    auto groupBox = new QGroupBox(boxLabel, parent);
    groupBox->setFlat(xPlayer::UseFlatGroupBox);
    auto list = new xPlayerListWidget(groupBox, displayTime);
    auto boxLayout = new QHBoxLayout();
    boxLayout->addWidget(list);
    groupBox->setLayout(boxLayout);
    return std::make_pair(groupBox,list);
}


xMainMovieWidget::xMainMovieWidget(xMoviePlayer* player, QWidget* parent):
        QStackedWidget(parent),
        currentMovieName(),
        currentMovieTag(),
        currentMovieDirectory(),
        moviePlayer(player),
        autoPlayNextMovie(false),
        useDatabaseMovieOverlay(true),
        databaseCutOff(0) {
    // main widget with controls.
    mainWidget = new QWidget(parent);
    // main splitter for non-fullscreen.
    auto mainWidgetSplitter = new QSplitter(mainWidget);
    mainWidgetSplitter->setOrientation(Qt::Horizontal);
    // Widget for music player and stacked widget.
    auto boxFilterWidget = new QWidget(mainWidgetSplitter);
    auto boxFilterLayout = new QVBoxLayout(boxFilterWidget);
    // Splitter for tags, directories and movies.
    auto boxSplitter = new QSplitter(boxFilterWidget);
    boxSplitter->setOrientation(Qt::Vertical);
    // Create widget for tag/directory
    auto tagDirectoryBoxes = new QWidget(boxSplitter);
    auto tagDirectoryLayout = new QHBoxLayout();
    // Create group boxes for tags, directories and movies.
    auto [ tagBox, tagList_ ] = addGroupBox(tr("Tags"), false, tagDirectoryBoxes);
    auto [ directoryBox, directoryList_ ] = addGroupBox(tr("Directories"), false, tagDirectoryBoxes);
    auto [ movieBox, movieList_ ] = addGroupBox(tr("Movies"), true, boxSplitter);
    // Layout the tag and box lists horizontally.
    tagDirectoryLayout->addWidget(tagBox);
    tagDirectoryLayout->addWidget(directoryBox);
    tagDirectoryBoxes->setLayout(tagDirectoryLayout);
    // Add tag box/directory box widget and movie box to splitter
    boxSplitter->addWidget(tagDirectoryBoxes);
    boxSplitter->addWidget(movieBox);
    boxSplitter->setCollapsible(0, false);
    boxSplitter->setCollapsible(1, false);
    // Movie box should have three times the size of the tag/directory boxes.
    boxSplitter->setStretchFactor(1, 3);
    // Setup tag, directory and movie lists.
    tagList = tagList_;
    tagList->enableSorting(true);
    directoryList = directoryList_;
    directoryList->enableSorting(true);
    movieList = movieList_;
    movieList->setMinimumWidth(xPlayer::AlbumListMinimumWidth);
    // Add filter for movies
    movieFilterBox = new QGroupBox(tr("Filter Movies"), mainWidget);
    movieFilterBox->setFlat(xPlayer::UseFlatGroupBox);
    movieFilter = new QLineEdit(movieFilterBox);
    auto movieFilterBoxLayout = new QVBoxLayout();
    movieFilterBoxLayout->addWidget(movieFilter);
    movieFilterBox->setLayout(movieFilterBoxLayout);
    // Add filter
    boxFilterLayout->addWidget(boxSplitter);
    boxFilterLayout->addWidget(movieFilterBox);
    // Widget for music player and stacked widget.
    auto moviePlayerStackedWidget = new QWidget(mainWidgetSplitter);
    auto moviePlayerStackedLayout = new xPlayerLayout(moviePlayerStackedWidget);
    // Create group box for the player widget.
    moviePlayerWidget = new xPlayerMovieWidget(moviePlayer, moviePlayerStackedWidget);
    // Stacked widget setup for movie player.
    movieStack = new QStackedWidget(moviePlayerStackedWidget);
    movieStack->addWidget(new QWidget(moviePlayerStackedWidget));
    moviePlayer->setParent(moviePlayerStackedWidget);
    movieStack->addWidget(moviePlayer);
    movieStack->setCurrentIndex(0);
    // Layout for the main movie widget.
    moviePlayerStackedLayout->addWidget(moviePlayerWidget, 0, 0, 2, 5);
    moviePlayerStackedLayout->addWidget(movieStack, 2, 0, 8, 5);
    moviePlayerStackedLayout->setRowStretch(8, 2);
    // Add widget to splitter.
    mainWidgetSplitter->addWidget(moviePlayerStackedWidget);
    mainWidgetSplitter->addWidget(boxFilterWidget);
    mainWidgetSplitter->setCollapsible(0, false);
    mainWidgetSplitter->setStretchFactor(0, 5);
    mainWidgetSplitter->setCollapsible(1, false);
    mainWidgetSplitter->setStretchFactor(1, 3);
    // Layout for main widget.
    auto movieLayout = new xPlayerLayout(mainWidget);
    movieLayout->addWidget(mainWidgetSplitter);
    // Connect signals.
    connect(tagList, &xPlayerListWidget::currentListIndexChanged, this, &xMainMovieWidget::selectTag);
    connect(directoryList, &xPlayerListWidget::currentListIndexChanged, this, &xMainMovieWidget::selectDirectory);
    connect(movieList, &xPlayerListWidget::listItemDoubleClicked, this, &xMainMovieWidget::selectMovie);
    connect(movieFilter, &QLineEdit::textChanged, movieList, &xPlayerListWidget::updateFilter);
    // Connect to movie player.
    connect(this, &xMainMovieWidget::setMovie, moviePlayer, &xMoviePlayer::setMovie);
    connect(this, &xMainMovieWidget::setMovieQueue, moviePlayer, &xMoviePlayer::setMovieQueue);
    connect(this, &xMainMovieWidget::clearMovieQueue, moviePlayer, &xMoviePlayer::clearMovieQueue);
    connect(moviePlayer, &xMoviePlayer::currentMovie, this, &xMainMovieWidget::updateSelectedMovie);
    connect(moviePlayer, &xMoviePlayer::currentMovie, moviePlayerWidget, &xPlayerMovieWidget::currentMovie);
    connect(moviePlayer, &xMoviePlayer::currentMovie, this, &xMainMovieWidget::updateWindowTitle);
    connect(moviePlayer, &xMoviePlayer::currentMovieLength, moviePlayerWidget, &xPlayerMovieWidget::currentMovieLength);
    connect(moviePlayer, &xMoviePlayer::currentMoviePlayed, moviePlayerWidget, &xPlayerMovieWidget::currentMoviePlayed);
    connect(moviePlayer, &xMoviePlayer::currentMoviePlayed, this, &xMainMovieWidget::updateWindowTitlePlayBack);
    connect(moviePlayer, &xMoviePlayer::currentSubtitles, moviePlayerWidget, &xPlayerMovieWidget::currentSubtitles);
    connect(moviePlayer, &xMoviePlayer::currentAudioChannels, moviePlayerWidget, &xPlayerMovieWidget::currentAudioChannels);
    connect(moviePlayer, &xMoviePlayer::currentChapters, moviePlayerWidget, &xPlayerMovieWidget::currentChapters);
    connect(moviePlayer, &xMoviePlayer::currentState, moviePlayerWidget, &xPlayerMovieWidget::currentState);
    // Update stack widget based on player state.
    connect(moviePlayer, &xMoviePlayer::currentState, this, &xMainMovieWidget::currentState);
    // Connect full window.
    connect(moviePlayer, &xMoviePlayer::fullWindowMode, this, &xMainMovieWidget::setFullWindow);
    connect(moviePlayerWidget, &xPlayerMovieWidget::autoPlayNextMovie, this, &xMainMovieWidget::setAutoPlayNextMovie);
    // Connect database.
    connect(moviePlayer, &xMoviePlayer::updatePlayedMovie, this, &xMainMovieWidget::updatePlayedMovie);
    // Connect configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedDatabaseMovieOverlay,
            this, &xMainMovieWidget::updatedDatabaseMovieOverlay);
    // Connect filters.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieViewFilters, [this]() {
        auto visible = xPlayerConfiguration::configuration()->getMovieViewFilters();
        movieFilterBox->setVisible(visible);
        if (!visible) {
            movieFilter->clear();
        }
    });
    // Prepare Stack
    addWidget(mainWidget);
    setCurrentWidget(mainWidget);
}

void xMainMovieWidget::initializeView() {
    updateWindowTitle(std::filesystem::path(), currentMovieName, currentMovieTag, currentMovieDirectory);
    emit showMenuBar(!moviePlayer->getFullWindowMode());
}

void xMainMovieWidget::clear() {
    moviePlayer->setFullWindowMode(false);
    // Clear queue.
    moviePlayer->stop();
    moviePlayerWidget->clear();
    // Clear tags, directories and movies lists.
    scannedTags(QStringList());
    scannedDirectories(QStringList());
    scannedMovies(std::vector<xMovieLibraryEntry*>());
}

void xMainMovieWidget::setFullWindow(bool mode) {
    qDebug() << "xMainMovieWidget: setFullWindow: " << mode;
    if (mode) {
        movieStack->removeWidget(moviePlayer);
        moviePlayer->setParent(this);
        addWidget(moviePlayer);
        setCurrentWidget(moviePlayer);
        QApplication::setOverrideCursor(Qt::BlankCursor);
    } else {
        removeWidget(moviePlayer);
        moviePlayer->setParent(mainWidget);
        movieStack->addWidget(moviePlayer);
        movieStack->setCurrentWidget(moviePlayer);
        setCurrentWidget(mainWidget);
        QApplication::restoreOverrideCursor();
    }
    emit showWindowTitle(createWindowTitle());
    emit showMenuBar(!mode);
}

void xMainMovieWidget::scannedTags(const QStringList& tags) {
    tagList->clearItems();
    tagList->addListItems(tags);
    updatePlayedTags();
}

void xMainMovieWidget::scannedDirectories(const QStringList& directories) {
    directoryList->clearItems();
    if (!directories.isEmpty()) {
        directoryList->addListItem(".");
        directoryList->addListItems(directories);
    } else {
        auto currentTag = tagList->currentItem();
        if (currentTag) {
            emit scanForTagAndDirectory(currentTag->text(), ".");
        }
    }
    updatePlayedDirectories();
}

void xMainMovieWidget::scannedMovies(const std::vector<xMovieLibraryEntry*>& movies) {
    movieList->clearItems();
    currentMovies.clear();
    for (const auto& movie : movies) {
        movieList->addListItem(movie);
        currentMovies.push_back(movie);
    }
    updatePlayedMovies();
    movieList->updateItems();
    qDebug() << "xMainMovieWidget: no of scanned movies: " << movies.size();
}

void xMainMovieWidget::selectTag(int index) {
    if ((index >= 0) && (index < tagList->count())) {
        directoryList->clearItems();
        movieList->clearItems();
        emit scanForTag(tagList->listItem(index)->text());
    }
}

void xMainMovieWidget::selectDirectory(int index) {
    if ((index >= 0) && (index < directoryList->count())) {
        movieList->clearItems();
        auto tag = tagList->currentItem()->text();
        auto directory = directoryList->listItem(index)->text();
        emit scanForTagAndDirectory(tag, directory);
    }
}

void xMainMovieWidget::selectMovie(xPlayerListWidgetItem* movieItem) {
    auto movieIndex = movieList->listIndex(movieItem);
    if ((movieIndex >= 0) && (movieIndex < currentMovies.size())) {
        auto currentMovie = currentMovies[movieIndex];
        updateMovieQueue(movieIndex);
        emit setMovie(currentMovie->getPath(), currentMovie->getMovieName(), currentMovie->getTagName(), currentMovie->getDirectoryName());
    }
}

void xMainMovieWidget::updateSelectedMovie(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory) {
    Q_UNUSED(name)
    Q_UNUSED(tag)
    Q_UNUSED(directory)
    auto movieIndex = movieList->currentListIndex();
    int pathIndex = -1;
    for (int i = 0; i < currentMovies.size(); ++i) {
        if (currentMovies[i]->getPath() == path) {
            pathIndex = i;
            break;
        }
    }
    // Update selection only if pathIndex is valid and not the currently selected listIndex.
    if ((pathIndex != movieIndex) && (pathIndex >= 0) && (pathIndex < movieList->count())) {
        movieList->setCurrentListIndex(pathIndex);
        updatePlayedMovies();
    }
}

void xMainMovieWidget::setAutoPlayNextMovie(bool mode) {
    autoPlayNextMovie = mode;
    updateMovieQueue(movieList->currentListIndex());
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
            tagList->listItem(i)->removeIcon();
        }
        // Clear directory list.
        for (auto i = 0; i < directoryList->count(); ++i) {
            directoryList->listItem(i)->removeIcon();
        }
        // Clear movie list.
        for (auto i = 0; i < movieList->count(); ++i) {
            movieList->listItem(i)->removeIcon();
            movieList->listItem(i)->removeToolTip();
        }
    }
}

void xMainMovieWidget::updateMovieQueue(int index) {
    if ((index >= 0) && (index < currentMovies.size())) {
        if (autoPlayNextMovie) {
            QList<std::pair<std::filesystem::path,QString>> queue;
            for (auto i = index+1; i < currentMovies.size(); ++i) {
                qDebug() << "updateMovieQueue: " << movieList->listItem(i)->text();
                queue.push_back(std::make_pair(currentMovies[i]->getPath(), currentMovies[i]->getMovieName()));
            }
            emit setMovieQueue(queue);
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
        auto tagItem = tagList->listItem(i);
        auto tag = tagItem->text();
        // Clear icon and tooltip.
        tagItem->removeIcon();
        tagItem->removeToolTip();
        for (const auto& [playedTag, playCount] : playedTags) {
            // Update icon and tooltip if movie already played.
            if (playedTag == tag) {
                tagItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
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
    auto tagItem = tagList->currentItem();
    if ((!tagItem) || (directoryList->count() == 0)) {
        return;
    }
    auto tag = tagItem->text();
    auto playedDirectories = xPlayerDatabase::database()->getPlayedDirectories(tag, databaseCutOff);
    for (auto i = 0; i < directoryList->count(); ++i) {
        auto directoryItem = directoryList->listItem(i);
        auto directory = directoryItem->text();
        // Clear icon and tooltip.
        directoryItem->removeIcon();
        directoryItem->removeToolTip();
        for (const auto& [playedDirectory, playCount] : playedDirectories) {
            // Update icon and tooltip if movie already played.
            if (playedDirectory == directory) {
                directoryItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
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
    auto tagItem = tagList->currentItem();
    // No tag currently selected. We do not need to update.
    if (!tagItem) {
        return;
    }
    auto tag = tagItem->text();
    auto directory = (directoryList->count() > 0) ? directoryList->currentItem()->text() : ".";
    auto playedMovies = xPlayerDatabase::database()->getPlayedMovies(tag, directory, databaseCutOff);
    for (auto i = 0; i < movieList->count(); ++i) {
        auto movieItem = movieList->listItem(i);
        auto movie = movieItem->text();
        // Clear icon and tooltip.
        movieItem->removeIcon();
        movieItem->removeToolTip();
        for (auto playedMovie = playedMovies.begin(); playedMovie != playedMovies.end(); ++playedMovie) {
            auto [currentMovie, playCount, timeStamp] = *playedMovie;
            // Update icon and tooltip if movie already played.
            if (currentMovie == movie) {
                movieItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
                // Adjust tooltip to play count "once" vs "x times".
                if (playCount > 1) {
                    movieItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                            arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
                } else {
                    movieItem->addToolTip(QString(tr("played once, last time on %1")).
                            arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
                }
                // Remove element to speed up search in the next iteration.
                playedMovies.erase(playedMovie);
                // End update if no more movies need to be marked.
                if (playedMovies.isEmpty()) {
                    return;
                }
                // Break loop and move on to the next movie listItem.
                break;
            }
        }
    }
}

void xMainMovieWidget::updatePlayedMovie(const QString& tag, const QString& directory,
                                         const QString& movie, int playCount, qint64 timeStamp) {
    // Only update if the database overlay is enabled.
    if (!useDatabaseMovieOverlay) {
        return;
    }
    auto tagItem = tagList->currentItem();
    // Update the tags.
    auto tagPlayedItems = tagList->findListItems(tag);
    auto tagPlayCount = xPlayerDatabase::database()->getMaxViewCount(tag, QString(), QString(), databaseCutOff);
    for (auto& tagPlayedItem : tagPlayedItems) {
        tagPlayedItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(tagPlayCount));
    }
    // If no tag selected or the tag does not match the selected
    // tags then we do not need to update the albums.
    if ((!tagItem) || (tagItem->text() != tag)) {
        return;
    }
    // Handle special case if we only have the "." directory.
    if (directoryList->count() > 0) {
        auto directoryItem = directoryList->currentItem();
        // Update the directorys.
        auto directoryPlayedItems = directoryList->findListItems(directory);
        auto directoryPlayCount = xPlayerDatabase::database()->getMaxPlayCount(tag, directory, QString(), databaseCutOff);
        for (auto& directoryPlayedItem : directoryPlayedItems) {
            directoryPlayedItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(directoryPlayCount));
        }
        // If no directory selected or the directory does not match the selected
        // directory then we do not need to update the tracks.
        if ((!directoryItem) || (directoryItem->text() != directory)) {
            return;
        }
    }
    auto moviePlayedItems = movieList->findListItems(movie);
    for (auto& moviePlayedItem : moviePlayedItems) {
        moviePlayedItem->setIcon(xPlayerConfiguration::configuration()->getPlayedLevelIcon(playCount));
        if (playCount > 1) {
            moviePlayedItem->addToolTip(QString(tr("played %1 times, last time on %2")).arg(playCount).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
        } else {
            moviePlayedItem->addToolTip(QString(tr("played once, last time on %1")).
                    arg(QDateTime::fromMSecsSinceEpoch(timeStamp).toString(Qt::TextDate)));
        }
    }
}

void xMainMovieWidget::updateWindowTitle(const std::filesystem::path& path, const QString& name, const QString& tag, const QString& directory) {
    Q_UNUSED(path)
    currentMovieName = name;
    currentMovieTag = tag;
    currentMovieDirectory = directory;
    if (moviePlayer->getFullWindowMode()) {
        emit showWindowTitle(createWindowTitle());
    }
}

void xMainMovieWidget::updateWindowTitlePlayBack(qint64 timeStamp) {
    if (moviePlayer->getFullWindowMode()) {
        emit showWindowTitle(QString("%1 - %2:%3:%4").
                arg(createWindowTitle()).
                arg(timeStamp/3600000).
                arg((timeStamp/60000)%60, 2, 10, QChar('0')).
                arg((timeStamp/1000)%60, 2, 10, QChar('0')));
    }
}


QString xMainMovieWidget::createWindowTitle() {
    if (moviePlayer->getFullWindowMode()) {
        if (currentMovieDirectory == ".") {
            return QString("%1 - (%2) - %3").arg(QApplication::applicationName(), currentMovieTag, currentMovieName);
        } else {
            return QString("%1 - (%2/%3) - %4").arg(QApplication::applicationName(), currentMovieTag, currentMovieDirectory, currentMovieName);
        }
    } else {
        return QApplication::applicationName();
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
            moviePlayer->setFullWindowMode(false);
        } break;
        case xMoviePlayer::StopState: {
            moviePlayer->setFullWindowMode(false);
            movieStack->setCurrentIndex(0);
        } break;
        case xMoviePlayer::ResetState: {
            // The libvlc media player has been restarted.
            moviePlayer->setFullWindowMode(false);
            movieStack->setCurrentIndex(0);
            // Clear movie player widget.
            moviePlayerWidget->clear();
        }
    }
}