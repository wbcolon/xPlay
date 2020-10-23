#include "xPlayerWidgetQwt.h"

#include <cmath>

#include <QGridLayout>
#include <QDial>
#include <QIcon>

#include <qwt/qwt_knob.h>
#include <qwt/qwt_date_scale_draw.h>

/**
 * Little helper class to adjust the tick labels
 */
class xPlayerWidgetScaleDraw:public QwtScaleDraw {
public:
    xPlayerWidgetScaleDraw() = default;
    ~xPlayerWidgetScaleDraw() = default;

    virtual QwtText label(double value) const {
        return QwtText(QString("%1:%2").arg(static_cast<int>(round(value)/60000)).
                arg((static_cast<int>(round(value))/1000)%60, 2, 10, QChar('0')));
    }
};

xPlayerWidgetQwt::xPlayerWidgetQwt(xMusicPlayer* musicPlayer, QWidget* parent, Qt::WindowFlags flags):
    xPlayerWidget(musicPlayer, parent, flags) {
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
    trackSlider = new QwtSlider(Qt::Horizontal, this);
    // Scale below
    trackSlider->setScalePosition(QwtSlider::LeadingScale);
    // Single step is 1 second
    trackSlider->setSingleSteps(1000);
    // Marker every 30 seconds
    trackSlider->setScaleStepSize(30000);
    trackSlider->setTracking(false);
    trackSlider->setScaleDraw(new xPlayerWidgetScaleDraw());
    // Slider initially empty
    trackSlider->setLowerBound(0);
    trackSlider->setUpperBound(0);
    trackSlider->setGroove(false);
    trackSlider->setTrough(true);

    // Create buttons for play/pause and stop
    playPauseButton = new QPushButton(QIcon(":/images/xplay-play.svg"), tr("Play"), this);
    auto stopButton = new QPushButton(QIcon(":/images/xplay-stop.svg"), tr("Stop"), this);
    // Create buttons for playlist control, previous, next and clear queue.
    auto prevButton = new QPushButton(QIcon(":/images/xplay-previous.svg"), tr("Prev"), this);
    auto nextButton = new QPushButton(QIcon(":/images/xplay-next.svg"), tr("Next"), this);
    nextButton->setLayoutDirection(Qt::RightToLeft);
    //nextButton->setStyleSheet("padding-left: 20px; padding-right: 20px;");
    auto clearButton = new QPushButton(QIcon(":/images/xplay-eject.svg"), tr("Clear"), this);
    auto volumeDial = new QwtKnob(this);
    volumeDial->setLowerBound(0);
    volumeDial->setUpperBound(100);
    volumeDial->setScaleStepSize(20);
    volumeDial->setWrapping(false);

    // Create the basic player widget layout.
    // Add labels, buttons and the slider.
    auto playerLayout = new QGridLayout(this);
    playerLayout->setSpacing(0);
    playerLayout->addWidget(new QLabel(tr("Artist"), this), 0, 0);
    playerLayout->addWidget(artistName, 0, 1, 1, 5);
    playerLayout->addWidget(new QLabel(tr("Album"), this), 1, 0);
    playerLayout->addWidget(albumName, 1, 1, 1, 5);
    playerLayout->addWidget(new QLabel(tr("Track"), this), 2, 0);
    playerLayout->addWidget(trackName, 2, 1, 1, 5);
    playerLayout->addWidget(trackPlayed, 4, 0);
    playerLayout->addWidget(trackLength, 4, 5);
    playerLayout->addWidget(trackSlider, 4, 1, 1, 4);
    // Create a layout for the music player and playlist control buttons.
    auto controlLayout = new QGridLayout();
    controlLayout->addWidget(playPauseButton, 0, 5, 1, 2);
    controlLayout->addWidget(stopButton, 1, 5, 1, 2);
    controlLayout->addWidget(prevButton, 2, 5, 1, 1);
    controlLayout->addWidget(nextButton, 2, 6, 1, 1);
    controlLayout->addWidget(clearButton, 3, 5, 1, 2);
    controlLayout->setColumnMinimumWidth(4, 20);
    controlLayout->addWidget(volumeDial, 0, 0, 4, 4);
    auto volumeLabel = new QLabel(tr("Volume"));
    volumeLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(volumeLabel, 3, 0, 1, 4);
    // Add the control layout to the player layout.
    playerLayout->setColumnMinimumWidth(6, 20);
    playerLayout->addLayout(controlLayout, 0, 7, 5, 1);
    // Connect the buttons to player widget and/or to the music player.
    connect(playPauseButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::playPause);
    connect(stopButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::stop);
    connect(prevButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::prev);
    connect(nextButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::next);
    connect(clearButton, &QPushButton::pressed, musicPlayer, &xMusicPlayer::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerWidgetQwt::clearQueue);
    connect(clearButton, &QPushButton::pressed, this, &xPlayerWidgetQwt::clear);
    // Connect the music player to the player widget.
    connect(musicPlayer, &xMusicPlayer::currentTrack, this, &xPlayerWidgetQwt::currentTrack);
    connect(musicPlayer, &xMusicPlayer::currentTrackPlayed, this, &xPlayerWidgetQwt::currentTrackPlayed);
    connect(musicPlayer, &xMusicPlayer::currentTrackLength, this, &xPlayerWidgetQwt::currentTrackLength);
    connect(musicPlayer, &xMusicPlayer::currentState, this, &xPlayerWidgetQwt::currentState);
    // Connect the track slider to the music player. Do proper conversion using lambdas.
    connect(trackSlider, &QwtSlider::sliderMoved, [=](double position) { musicPlayer->seek(static_cast<qint64>(position)); } );
    // Connect the volume slider to the music player. Do proper conversion using lambdas.
    connect(volumeDial, &QwtKnob::valueChanged, [=](double volume) { musicPlayer->setVolume(static_cast<int>(volume)); } );
    // Do not resize the player widget vertically
    setFixedHeight(sizeHint().height());
    // Setup volume
    volumeDial->setValue(musicPlayer->getVolume());
}

void xPlayerWidgetQwt::clear() {
    // Reset the play/pause button and clear all track info.
    artistName->clear();
    albumName->clear();
    trackName->clear();
    trackLength->clear();
    trackPlayed->clear();
    // Reset slider
    trackSlider->setLowerBound(0);
    trackSlider->setUpperBound(0);
}

void xPlayerWidgetQwt::currentTrack(int index, const QString& artist, const QString& album, const QString& track) {
    // Display the current track information (without length)
    artistName->setText(artist);
    albumName->setText(album);
    trackName->setText(track);
    // Signal index update to the Queue.
    emit currentQueueTrack(index);
}

// Simple lambda to convert ms into readable time format in QString.
auto msToFormat = [](qint64 ms) {
   return QString("%1:%2.%3").arg(ms/60000).arg((ms/1000)%60, 2, 10, QChar('0')).arg((ms%1000)/10, 2, 10, QChar('0'));
};

// Little lambda to determine the divider for our track slider.
auto scaleDivider = [](double length) {
    for (auto scaleDivider : { 10000.0, 30000.0, 60000.0, 120000.0, 300000.0, 600000.0, 1200000.0 }) {
        if ((length / scaleDivider) <= 10.0) {
            return scaleDivider;
        }
    }
    return 1200000.0;
};

void xPlayerWidgetQwt::currentTrackLength(qint64 length) {
    // Update the length of the current track.
    trackLength->setText(msToFormat(length));
    // Set scale steps.
    trackSlider->setScaleStepSize(scaleDivider(length));
    trackSlider->setLowerBound(0);
    trackSlider->setUpperBound(length);
    trackSlider->setValue(0);
}

void xPlayerWidgetQwt::currentTrackPlayed(qint64 played) {
    // Update the time played for the current track.
    trackPlayed->setText(msToFormat(played));
    // Update the slider position.
    trackSlider->setValue(played);
}

void xPlayerWidgetQwt::currentState(xMusicPlayer::State state) {
    // Update the play/pause button based on the state of the music player.
    switch (state) {
        case xMusicPlayer::PlayingState: {
            playPauseButton->setIcon(QIcon(":/images/xplay-pause.svg"));
            playPauseButton->setText(tr("Pause"));
        } break;
        case xMusicPlayer::PauseState:
        case xMusicPlayer::StopState: {
            playPauseButton->setIcon(QIcon(":/images/xplay-play.svg"));
            playPauseButton->setText(tr("Play"));
        } break;
    }
}

void xPlayerWidgetQwt::setVolume(int vol) {
    Q_UNUSED(vol)
}