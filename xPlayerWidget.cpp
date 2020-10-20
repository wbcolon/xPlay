#include "xPlayerWidget.h"

#include <QGridLayout>
#include <QMediaControl>
#include <QMediaService>

xPlayerWidget::xPlayerWidget(xMusicPlayer* musicPlayer, QWidget* parent, Qt::WindowFlags flags):
    QWidget(parent, flags) {
    // Create labels for artist, album and track
    artistName = new QLabel(this);
    albumName = new QLabel(this);
    trackName = new QLabel(this);
    // Create labels for length of the track and time played.
    // Labels located on the left and right of a slider.
    trackLength = new QLabel(this);
    trackLength->setAlignment(Qt::AlignCenter);
    trackPlayed = new QLabel(this);
    trackPlayed->setAlignment(Qt::AlignCenter);
    // Create a slider that displays the played time and can be used
    // to seek within a track.
    trackSlider = new QSlider(Qt::Horizontal, this);
    // Single step is 1 second
    trackSlider->setSingleStep(1000);
    // Marker every 30 seconds
    trackSlider->setTickInterval(30000);
    trackSlider->setTickPosition(QSlider::TicksBelow);
    // Create buttons for play/pause and stop
    playPauseButton = new QPushButton(tr("Play"), this);
    auto stopButton = new QPushButton(tr("Stop"), this);
    // Create buttons for playlist control, previous, next and clear queue.
    auto prevButton = new QPushButton(tr("Prev"), this);
    auto nextButton = new QPushButton(tr("Next"), this);
    auto clearButton = new QPushButton(tr("Clear Queue"), this);
    // Create the basic player widget layout.
    // Add labels, buttons and the slider.
    auto playerLayout = new QGridLayout(this);
    playerLayout->setSpacing(0);
    playerLayout->addWidget(new QLabel(tr("Artist:"), this), 0, 0);
    playerLayout->addWidget(artistName, 0, 1, 1, 5);
    playerLayout->addWidget(new QLabel(tr("Album:"), this), 1, 0);
    playerLayout->addWidget(albumName, 1, 1, 1, 5);
    playerLayout->addWidget(new QLabel(tr("Track:"), this), 2, 0);
    playerLayout->addWidget(trackName, 2, 1, 1, 5);
    playerLayout->addWidget(trackPlayed, 4, 0);
    playerLayout->addWidget(trackLength, 4, 5);
    playerLayout->addWidget(trackSlider, 4, 1, 1, 4);
    // Create a layout for the music player and playlist control buttons.
    auto controlLayout = new QGridLayout();
    controlLayout->addWidget(playPauseButton, 0, 0, 1, 2);
    controlLayout->addWidget(stopButton, 1, 0, 1, 2);
    controlLayout->addWidget(prevButton, 2, 0, 1, 1);
    controlLayout->addWidget(nextButton, 2, 1, 1, 1);
    controlLayout->addWidget(clearButton, 3, 0, 1, 2);
    // Add the control layout to the player layout.
    playerLayout->addLayout(controlLayout, 0, 6, 5, 1);
    // Connect the buttons to player widget and/or to the music player.
    connect(playPauseButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::playPause);
    connect(playPauseButton, &QPushButton::pressed, this, &xPlayerWidget::playPause);
    connect(stopButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::stop);
    connect(stopButton, &QPushButton::pressed, this, &xPlayerWidget::stop);
    connect(prevButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::prev);
    connect(nextButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::next);
    connect(clearButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerWidget::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerWidget::clear);
    // Connect the music player to the player widget.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xPlayerWidget::currentTrack);
    connect(musicPlayer, &xMusicPlayer::currentTrackPlayed, this, &xPlayerWidget::currentTrackPlayed);
    connect(musicPlayer, &xMusicPlayer::currentTrackLength, this, &xPlayerWidget::currentTrackLength);
    // Connect the slider to the music player
    connect(trackSlider, &QSlider::sliderMoved, musicPlayer, &xMusicPlayer::seek);
    // Do not resize the player widget vertically
    setFixedHeight(sizeHint().height());
}

void xPlayerWidget::playPause() {
    // Toggle the play/pause button text.
    if (playPauseButton->text().startsWith(tr("Play"))) {
        playPauseButton->setText(tr("Pause"));
    } else {
        playPauseButton->setText(tr("Play"));
    }
}

void xPlayerWidget::stop() {
    // Reset the play/pause button.
    playPauseButton->setText(tr("Play"));
}

void xPlayerWidget::clear() {
    // Reset the play/pause button and clear all track info.
    playPauseButton->setText(tr("Play"));
    artistName->clear();
    albumName->clear();
    trackName->clear();
    trackLength->clear();
    trackPlayed->clear();
}

void xPlayerWidget::currentTrack(int index, const QString& artist, const QString& album, const QString& track) {
    // Display the current track information (without length)
    artistName->setText(artist);
    albumName->setText(album);
    trackName->setText(track);
    // Signal index update to the Queue.
    emit currentQueueTrack(index);
}

// Simple lambda to convert ms into readable time format in QString.
auto msToFormat = [](qint64 ms) {
   return QString("%1:%2.%3").arg(ms/60000).arg((ms/1000)%60, 2, 10, QChar('0')).arg(ms%1000, 3, 10, QChar('0'));
};

void xPlayerWidget::currentTrackLength(qint64 length) {
    // Update the length of the current track.
    trackLength->setText(msToFormat(length));
    // Set maximum of slider to the length of the track. Reset the slider position-
    trackSlider->setMaximum(length);
    trackSlider->setSliderPosition(0);
}

void xPlayerWidget::currentTrackPlayed(qint64 played) {
    // Update the time played for the current track.
    trackPlayed->setText(msToFormat(played));
    // Update the slider position.
    trackSlider->setSliderPosition(played);
}
