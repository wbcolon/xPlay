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

#include <QPushButton>

#ifndef __XPLAYERCONTROLBUTTONWIDGET_H__
#define __XPLAYERCONTROLBUTTONWIDGET_H__

class xPlayerControlButtonWidget:public QWidget {
    Q_OBJECT

public:
    enum xPlayerControlButtonMode { MusicPlayerMode, MoviePlayerMode };

    explicit xPlayerControlButtonWidget(xPlayerControlButtonMode mode, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerControlButtonWidget() override = default;
    /**
     * Set the state of the play/pause button.
     *
     * Put button in state "play" if the state is true and
     * put the button in "pause" if the state is false.
     *
     * @param mode the new state for the play/pause button.
     */
    void setPlayPauseState(bool state);
    /**
     * Return the current state of the play/pause button.
     * @return
     */
    [[nodiscard]] bool getPlayPauseState() const;

signals:
    /**
     * Signal emitted if the play/pause button is pressed.
     */
    void playPausePressed();
    /**
     * Signal emitted if the stop button is pressed.
     */
    void stopPressed();
    /**
     * Signal emitted if the previous button is pressed.
     */
    void previousPressed();
    /**
     * Signal emitted if the next button is pressed.
     */
    void nextPressed();
    /**
     * Signal emitted if the forward button is pressed.
     */
    void forwardPressed();
    /**
     * Signal emitted if the rewind button is pressed.
     */
    void rewindPressed();
    /**
     * Signal emitted if the clear button is pressed.
     */
    void clearPressed();
    /**
     * Signal emitted if the fullWindow button is pressed.
     */
    void fullWindowPressed();

private slots:
    /**
     * Called upon pressing the play/pause button to update its state.
     */
    void playPause();
    /**
     * Called upon pressing the stop button to update the play/pause button state.
     */
    void stop();

private:
    QPushButton* playPauseButton;
    bool playPauseState;
    xPlayerControlButtonMode controlMode;
};

#endif
