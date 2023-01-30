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
#ifndef __XPLAYERVOLUMEWIDGET_H__
#define __XPLAYERVOLUMEWIDGET_H__

#include <QDial>
#include <QPushButton>


class xPlayerVolumeDial:public QDial {
    Q_OBJECT

public:
    explicit xPlayerVolumeDial(QWidget* parent=nullptr);
    ~xPlayerVolumeDial() override = default;

protected:
    /**
     * Overload paint function. Add value text in the dial middle.
     */
    void paintEvent(QPaintEvent* event) override;
};

class xPlayerVolumeWidget:public QWidget {
    Q_OBJECT

public:
    explicit xPlayerVolumeWidget(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerVolumeWidget() override = default;
    /**
      * Retrieve the current volume of displayed in the UI.
      *
      * @return the volume as integer in between 0 and 100.
      */
    [[nodiscard]] int getVolume() const;
    /**
     * Return the mute state for the volume widget.
     *
     * @return true if volume know is disabled, false otherwise.
     */
    [[nodiscard]] bool isMuted() const;

signals:
    /**
     * Signal an update of the volume displayed.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    void volume(int vol);
    /**
     * Signal an update of the muted state displayed.
     *
     * @param mute the state of muted as boolean.
     */
    void muted(bool mute);


public slots:
    /**
     * Set the volume displayed in the UI.
     *
     * @param vol the volume as integer in between 0 and 100.
     */
    void setVolume(int vol);
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute);
    /**
     * Toggle the mute mode.
     */
    virtual void toggleMuted();

private slots:
    /**
     * Called if valueChanged is emitted.
     *
     * @param vol the current value as int.
     */
    void volumeChanged(int vol);

private:
    int currentVolume;
    bool currentMuted;
    QPushButton* volumeMuteButton;
    xPlayerVolumeDial* volumeKnob;
};

#endif
