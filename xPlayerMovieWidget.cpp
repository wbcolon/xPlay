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
#include "xPlayerVolumeWidgetX.h"
#include "xPlayerUI.h"
#include "xPlayerConfiguration.h"

#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QCheckBox>
#include <QDebug>

xPlayerMovieWidget::xPlayerMovieWidget(xMoviePlayerX* player, QWidget *parent, Qt::WindowFlags flags):
        QWidget(parent, flags),
        moviePlayer(player),
        moviePlayerState(xMoviePlayerX::StopState) {

    // Slider widget for showing movie length and played time.
    sliderWidget = new xPlayerSliderWidgetX(this);
    sliderWidget->useScaleSections(8);
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

    audioChannelBox = new QComboBox(controlTabPlayer);
    auto audioChannelLabel = new QLabel(tr("Audio"), controlTabPlayer);
    audioChannelLabel->setContentsMargins(xPlayerLayout::SmallSpace, 0, 0, 0);
    audioChannelLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    subtitleBox = new QComboBox(controlTabPlayer);
    auto subtitleLabel = new QLabel(tr("Subtitle"), controlTabPlayer);
    subtitleLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    subtitleLabel->setContentsMargins(xPlayerLayout::SmallSpace, 0, 0, 0);
    auto scaleAndCropCheck = new QCheckBox(tr("Scale and Crop"), controlTabPlayer);
    scaleAndCropCheck->setChecked(false);
    auto autoPlayNextCheck = new QCheckBox(tr("Autoplay Next"), controlTabPlayer);
    autoPlayNextCheck->setChecked(false);
    auto volumeWidget = new xPlayerVolumeWidgetX(controlTabPlayer);
    // Connect the volume knob and track slider to the music player.
    connect(volumeWidget, &xPlayerVolumeWidget::volume, moviePlayer, &xMoviePlayerX::setVolume);
    connect(volumeWidget, &xPlayerVolumeWidget::muted, moviePlayer, &xMoviePlayerX::setMuted);
    connect(moviePlayer, &xMoviePlayerX::currentMoviePlayed, sliderWidget, &xPlayerSliderWidget::trackPlayed);
    connect(moviePlayer, &xMoviePlayerX::currentMovieLength, sliderWidget, &xPlayerSliderWidget::trackLength);
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
    movieLayout->addWidget(autoPlayNextCheck, 3, 1, 1, 1);
    movieLayout->addWidget(scaleAndCropCheck, 3, 2, 1, 1);
    movieLayout->addColumnSpacer(3, xPlayerLayout::LargeSpace);
    movieLayout->addWidget(audioChannelLabel, 2, 4, 1, 2);
    movieLayout->addWidget(audioChannelBox, 3, 4, 1, 2);
    movieLayout->addWidget(subtitleLabel, 2, 6, 1, 2);
    movieLayout->addWidget(subtitleBox, 3, 6, 1, 2);
    movieLayout->addWidget(controlTab, 0, 10, 4, 1);
    movieLabel->setText("no movie");
    // Connect Rotel amp widget configuration.
    connect(xPlayerConfiguration::configuration(), &xPlayerConfiguration::updatedRotelWidget, [=]() {
        controlTab->setTabEnabled(1, xPlayerConfiguration::configuration()->rotelWidget());
    } );
    // Connect the buttons to player widget and/or to the music player.
    connect(controlButtonWidget, &xPlayerControlButtonWidget::playPausePressed, moviePlayer, &xMoviePlayerX::playPause);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::stopPressed, moviePlayer, &xMoviePlayerX::stop);
    connect(controlButtonWidget, &xPlayerControlButtonWidget::rewindPressed, [=]() { moviePlayer->jump(-60000); });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::forwardPressed, [=]() { moviePlayer->jump(60000); });
    connect(controlButtonWidget, &xPlayerControlButtonWidget::fullWindowPressed, this, &xPlayerMovieWidget::fullWindowPressed);
    // Connect check boxes.
    connect(autoPlayNextCheck, &QCheckBox::clicked, this, &xPlayerMovieWidget::autoPlayNextMovie);
    connect(scaleAndCropCheck, &QCheckBox::clicked, moviePlayer, &xMoviePlayerX::setScaleAndCropMode);
    connect(moviePlayer, &xMoviePlayerX::scaleAndCropMode, scaleAndCropCheck, &QCheckBox::setChecked);
    // Connect combo boxes.
    connect(audioChannelBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectAudioChannel(int)));
    connect(subtitleBox, SIGNAL(currentIndexChanged(int)), moviePlayer, SLOT(selectSubtitle(int)));
    // Seek.
    connect(sliderWidget, &xPlayerSliderWidgetX::seek, moviePlayer, &xMoviePlayerX::seek);
    // Movie player volume updates.
    connect(moviePlayer, &xMoviePlayerX::currentVolume, volumeWidget, &xPlayerVolumeWidgetX::setVolume);
    // Setup volume
    volumeWidget->setVolume(moviePlayer->getVolume());
}

void xPlayerMovieWidget::clear() {
    // Clear the audio/subtitle combo box and reset the movie label.
    audioChannelBox->clear();
    subtitleBox->clear();
    movieLabel->setText("no movie");
    sliderWidget->clear();
}

void xPlayerMovieWidget::currentMovie(const QString& path, const QString& movie, const QString& tag, const QString& directory) {
    Q_UNUSED(path)
    Q_UNUSED(tag)
    Q_UNUSED(directory)
    movieLabel->setText(movie);
}

void xPlayerMovieWidget::currentAudioChannels(const QStringList& audioChannels) {
    updateComboBoxEntries(audioChannelBox, audioChannels);
}

void xPlayerMovieWidget::currentSubtitles(const QStringList& subtitles) {
    updateComboBoxEntries(subtitleBox, subtitles);
}

void xPlayerMovieWidget::currentMoviePlayed(qint64 played) {
    sliderWidget->trackPlayed(played);
}

void xPlayerMovieWidget::currentMovieLength(qint64 length) {
    sliderWidget->trackLength(length);
}

void xPlayerMovieWidget::currentState(xMoviePlayerX::State state) {
    // Update the play/pause button based on the state of the music player.
    moviePlayerState = state;
    switch (moviePlayerState) {
        case xMoviePlayerX::PlayingState: {
            controlButtonWidget->setPlayPauseState(false);
        } break;
        case xMoviePlayerX::PauseState:
        case xMoviePlayerX::StopState: {
            controlButtonWidget->setPlayPauseState(true);
        } break;
        default: break;
    }
}

void xPlayerMovieWidget::fullWindowPressed() {
    if ((moviePlayerState == xMoviePlayerX::PlayingState) ||
        (moviePlayerState == xMoviePlayerX::PauseState)) {
        emit toggleFullWindow();
    }
}

void xPlayerMovieWidget::updateComboBoxEntries(QComboBox* comboBox, const QStringList& entries) {
    // Avoid constant updates
    qDebug() << "xPlayerMovieWidget: updateComboBoxEntries(0): " << comboBox->count() << ", " << entries.size();
    if (comboBox->count() == entries.size()) {
        for (auto i = 0; i < comboBox->count(); ++i) {
            qDebug() << "xPlayerMovieWidget: updateComboBoxEntries(1): " << comboBox->itemText(i) << ", " << entries[i];
            // Refresh if we find unmatched items.
            if (comboBox->itemText(i) != entries[i]) {
                comboBox->clear();
                comboBox->addItems(entries);
                return;
            }
        }
    } else {
        comboBox->clear();
        comboBox->addItems(entries);
    }
}
