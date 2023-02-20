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

#include "xPlayerMovieWidget.h"
#include "xPlayerVolumeWidget.h"
#include "xPlayerPulseAudioControls.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"

#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QMenu>
#include <QPushButton>
#include <QApplication>

xPlayerMovieWidget::xPlayerMovieWidget(xMoviePlayer* player, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        moviePlayer(player),
        moviePlayerState(xMoviePlayer::StopState) {

    // Slider widget for showing movie length and played time.
    sliderWidget = new xPlayerSliderWidget(this);
    sliderWidget->useHourScale(true);
    // Name of the movie (filename).
    movieLabel = new QLabel(this);
    movieLabel->setAlignment(Qt::AlignCenter);
    movieLabel->setWordWrap(true);
    movieLabel->setStyleSheet("font-weight: bold");
    // Control Tab
    auto controlTab = new QTabWidget(this);
    auto controlTabPlayer = new QWidget(this);
    controlTabRotel = new xPlayerRotelWidget(this);
    controlTab->setTabPosition(QTabWidget::West);
    controlTab->addTab(controlTabPlayer, "xPlay");
    controlTab->addTab(controlTabRotel, "Rotel");
    controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    // Create control buttons widget.
    controlButtonWidget = new xPlayerControlButtonWidget(xPlayerControlButtonWidget::MoviePlayerMode, controlTabPlayer);
    // Control Box.
    auto controlBox = new QWidget(controlTabPlayer);
    // Chapters.
    chapterLabel = new QLabel(tr("Chapter"), controlBox);
    chapterLabel->setContentsMargins(xPlayerLayout::SmallSpace, 0, 0, 0);
    chapterLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    // Audio Tracks.
    chapterBox = new QComboBox(controlBox);
    audioChannelBox = new QComboBox(controlBox);
    auto audioChannelLabel = new QLabel(tr("Audio"), controlBox);
    audioChannelLabel->setContentsMargins(xPlayerLayout::SmallSpace, 0, 0, 0);
    audioChannelLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    // Subtitles.
    subtitleBox = new QComboBox(controlBox);
    auto subtitleLabel = new QLabel(tr("Subtitle"), controlBox);
    subtitleLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    subtitleLabel->setContentsMargins(xPlayerLayout::SmallSpace, 0, 0, 0);
    // Video options menu.
    optionsMenuButton = new QPushButton(tr("Options"), controlBox);
    createOptionsMenu();
    // Control Box Layout.
    auto controlBoxLayout = new xPlayerLayout(controlBox);
    controlBoxLayout->setContentsMargins(xPlayerLayout::LargeSpace, xPlayerLayout::LargeSpace, xPlayerLayout::LargeSpace, 0);
    controlBoxLayout->setSpacing(0);
    controlBoxLayout->addWidget(optionsMenuButton, 1, 0);
    controlBoxLayout->addColumnSpacer(1, xPlayerLayout::SeparatorSpace);
    controlBoxLayout->addWidget(audioChannelLabel, 0, 2, 1, 2);
    controlBoxLayout->addWidget(audioChannelBox, 1, 2, 1, 2);
    controlBoxLayout->addWidget(subtitleLabel, 0, 4, 1, 2);
    controlBoxLayout->addWidget(subtitleBox, 1, 4, 1, 2);
    controlBoxLayout->addColumnSpacer(6, xPlayerLayout::LargeSpace);
    controlBoxLayout->addWidget(chapterLabel, 0, 7, 1, 2);
    controlBoxLayout->addWidget(chapterBox, 1, 7, 1, 2);
    // Volume widget.
    auto volumeWidget = new xPlayerVolumeWidget(controlTabPlayer);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, moviePlayer, &xMoviePlayer::setVolume);
    connect(volumeWidget, &xPlayerVolumeWidget::muted, moviePlayer, &xMoviePlayer::setMuted);
    connect(moviePlayer, &xMoviePlayer::currentMoviePlayed, sliderWidget, &xPlayerSliderWidget::setPlayed);
    connect(moviePlayer, &xMoviePlayer::currentMovieLength, sliderWidget, &xPlayerSliderWidget::setLength);
    // Layout
    auto controlLayout = new xPlayerLayout(controlTabPlayer);
    controlLayout->setSpacing(xPlayerLayout::NoSpace);
    controlLayout->addWidget(volumeWidget, 0, 0);
    controlLayout->addColumnSpacer(1, xPlayerLayout::MediumSpace);
    controlLayout->addWidget(controlButtonWidget, 0, 2);
    // Fix sizes of the control tab
    controlTabPlayer->setFixedWidth(controlTabPlayer->sizeHint().width());
    controlTabRotel->setFixedWidth(controlTabPlayer->sizeHint().width());
    controlTab->setFixedWidth(controlTab->sizeHint().width());
    // Layout
    auto movieLayout = new xPlayerLayout(this);
    movieLayout->setSpacing(xPlayerLayout::NoSpace);
    movieLayout->addWidget(movieLabel, 0, 0, 1, 9);
    movieLayout->addWidget(sliderWidget, 1, 0, 1, 9);
    movieLayout->addWidget(controlBox, 2, 0, 2, 9);
    movieLayout->addWidget(controlTab, 0, 10, 4, 1);
    movieLabel->setText("no movie");
    // Connect Rotel amp widget configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelWidget, [=]() {
        controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    } );
    // Connect the buttons to player widget and/or to the music player.
    connect(controlButtonWidget, &xPlayerControlButtonWidget::playPausePressed, moviePlayer, &xMoviePlayer::playPause);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::stopPressed, moviePlayer, &xMoviePlayer::stop);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::previousPressed, moviePlayer, &xMoviePlayer::previousChapter);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::nextPressed, moviePlayer, &xMoviePlayer::nextChapter);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::rewindPressed, [=]() {
        moviePlayer->jump(-xMoviePlayer::ForwardRewindDelta);
    });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::forwardPressed, [=]() {
        moviePlayer->jump(xMoviePlayer::ForwardRewindDelta);
    });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::fullWindowPressed, this, &xPlayerMovieWidget::fullWindowPressed);
    // Connect combo boxes.
    connect(audioChannelBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectAudioChannel(int)));
    connect(subtitleBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectSubtitle(int)));
    connect(chapterBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(playChapter(int)));
    // Following change in combobox will also trigger a select of the audio channel, subtitle of chapter.
    // Disconnect the corresponding signals before setting and connect afterwards.
    connect(moviePlayer, &xMoviePlayer::currentAudioChannel, [=](int index) {
        disconnect(audioChannelBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectAudioChannel(int)));
        audioChannelBox->setCurrentIndex(index);
        connect(audioChannelBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectAudioChannel(int)));
    });
    connect(moviePlayer, &xMoviePlayer::currentSubtitle, [=](int index) {
        disconnect(subtitleBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectSubtitle(int)));
        subtitleBox->setCurrentIndex(index);
        connect(subtitleBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectSubtitle(int)));
    });
    connect(moviePlayer, &xMoviePlayer::currentChapter, [=](int chapter) {
        // Disconnect the playChapter signal.
        disconnect(chapterBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(playChapter(int)));
        chapterBox->setCurrentIndex(chapter);
        connect(chapterBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(playChapter(int)));
    });
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedMovieAudioCompression, [=]() {
        bool enabled = xPlayerConfiguration::configuration()->getMovieAudioCompression();
        optionsAudioCompression->setChecked(enabled);
        moviePlayer->setAudioCompressionMode(enabled);
    });
    // Seek.
    connect(sliderWidget, &xPlayerSliderWidget::seek, moviePlayer, &xMoviePlayer::seek);
    // Movie player volume updates.
    connect(moviePlayer, &xMoviePlayer::currentVolume, volumeWidget, &xPlayerVolumeWidget::setVolume);
    // Setup volume
    volumeWidget->setVolume(moviePlayer->getVolume());
}

void xPlayerMovieWidget::createOptionsMenu() {
    auto optionsMenu = new QMenu(optionsMenuButton);
    // Autoplay next.
    auto optionsAutoplayNext = new QAction(tr("Autoplay Next"), optionsMenu);
    optionsAutoplayNext->setCheckable(true);
    optionsAutoplayNext->setChecked(false);
    connect(optionsAutoplayNext, &QAction::triggered, this, &xPlayerMovieWidget::autoPlayNextMovie);
    // Compress Audio.
    optionsAudioCompression = new QAction(tr("Audio Compression"), optionsMenu);
    optionsAudioCompression->setCheckable(true);
    optionsAudioCompression->setChecked(xPlayerConfiguration::configuration()->getMovieAudioCompression());
    connect(optionsAudioCompression, &QAction::triggered, moviePlayer, &xMoviePlayer::setAudioCompressionMode);
    // Deinterlace.
    auto optionsDeinterlace = new QAction(tr("Deinterlace"), optionsMenu);
    optionsDeinterlace->setCheckable(true);
    optionsDeinterlace->setChecked(false);
    connect(optionsDeinterlace, &QAction::triggered, moviePlayer, &xMoviePlayer::setDeinterlaceMode);
    // Crop submenu.
    auto cropSubmenu = new QMenu(tr("Crop"), optionsMenu);
    auto cropActions = new QActionGroup(cropSubmenu);
    cropActions->setExclusive(true);
    auto cropDisabled = cropActions->addAction(tr("Disable"));
    cropDisabled->setCheckable(true);
    cropDisabled->setChecked(true);
    connect(cropDisabled, &QAction::triggered, [=](bool) {
        moviePlayer->setCropAspectRatio(QString());
    });
    cropSubmenu->addAction(cropDisabled);
    cropSubmenu->addSeparator();
    for (auto&& supported : xMoviePlayer::supportedAspectRatio()) {
        auto cropSupported = cropActions->addAction(supported.first);
        cropSupported->setCheckable(true);
        connect(cropSupported, &QAction::triggered, [=](bool) {
            moviePlayer->setCropAspectRatio(supported.second);
        });
        cropSubmenu->addAction(cropSupported);
    }
    // Compose menu.
    optionsMenu->addAction(optionsAutoplayNext);
    optionsMenu->addSeparator();
    optionsMenu->addAction(optionsAudioCompression);
    optionsMenu->addAction(optionsDeinterlace);
    optionsMenu->addMenu(cropSubmenu);
    optionsMenuButton->setMenu(optionsMenu);
}

void xPlayerMovieWidget::clear() {
    // Clear the audio/subtitle combo box and reset the movie label.
    audioChannelBox->clear();
    subtitleBox->clear();
    movieLabel->setText("no movie");
    sliderWidget->clear();
}

void xPlayerMovieWidget::currentMovie(const std::filesystem::path& path, const QString& movie, const QString& tag, const QString& directory) {
    Q_UNUSED(path)
    Q_UNUSED(tag)
    Q_UNUSED(directory)
    audioChannelBox->clear();
    subtitleBox->clear();
    movieLabel->setText(movie);
}

void xPlayerMovieWidget::currentAudioChannels(const QStringList& audioChannels, const QStringList& audioCodecs) {
    if (updateComboBoxEntries(audioChannelBox, audioChannels)) {
        if (audioChannels.count() == audioCodecs.count()) {
            for (auto codec = 0; codec < audioCodecs.count(); ++codec) {
                audioChannelBox->setItemData(codec, audioCodecs[codec], Qt::ToolTipRole);
            }
        }
    }
}

void xPlayerMovieWidget::currentSubtitles(const QStringList& subtitles) {
    updateComboBoxEntries(subtitleBox, subtitles);
}

void xPlayerMovieWidget::currentChapters(const QStringList& chapters) {
    if (chapters.count() > 0) {
        // Enable chapter section and update entries.
        chapterLabel->setEnabled(true);
        chapterBox->setEnabled(true);
        updateComboBoxEntries(chapterBox, chapters);
    } else {
        // Disable chapter section and clear entries.
        chapterLabel->setEnabled(false);
        chapterBox->setEnabled(false);
        chapterBox->clear();
    }
}

void xPlayerMovieWidget::currentMoviePlayed(qint64 played) {
    sliderWidget->setPlayed(played);
}

void xPlayerMovieWidget::currentMovieLength(qint64 length) {
    sliderWidget->setLength(length);
}

void xPlayerMovieWidget::currentState(xMoviePlayer::State state) {
    // Update the play/pause button based on the state of the music player.
    moviePlayerState = state;
    switch (moviePlayerState) {
        case xMoviePlayer::PlayingState: {
            controlButtonWidget->setPlayPauseState(false);
        } break;
        case xMoviePlayer::PauseState:
        case xMoviePlayer::StopState: {
            controlButtonWidget->setPlayPauseState(true);
        } break;
        default: break;
    }
}

void xPlayerMovieWidget::fullWindowPressed() {
    if ((moviePlayerState == xMoviePlayer::PlayingState) ||
        (moviePlayerState == xMoviePlayer::PauseState)) {
        emit toggleFullWindow();
    }
}

bool xPlayerMovieWidget::updateComboBoxEntries(QComboBox* comboBox, const QStringList& entries) {
    // Avoid constant updates
    qDebug() << "xPlayerMovieWidget: updateComboBoxEntries(0): " << comboBox->count() << ", " << entries.size();
    if (comboBox->count() == entries.size()) {
        for (auto i = 0; i < comboBox->count(); ++i) {
            qDebug() << "xPlayerMovieWidget: updateComboBoxEntries(1): " << comboBox->itemText(i) << ", " << entries[i];
            // Refresh if we find unmatched items.
            if (comboBox->itemText(i) != entries[i]) {
                comboBox->clear();
                comboBox->addItems(entries);
                return true;
            }
        }
        return false;
    } else {
        comboBox->clear();
        comboBox->addItems(entries);
        return true;
    }
}
