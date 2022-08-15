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
#ifndef __XPLAYERPULSEAUDIOCONTROLS_H__
#define __XPLAYERPULSEAUDIOCONTROLS_H__

#include <QPushButton>
#include <qwt/qwt_knob.h>
#include <pulse/pulseaudio.h>

class xPlayerPulseAudioControls:public QObject {
    Q_OBJECT
public:
    /**
     * Return the pulseaudio volume controls.
     *
     * @return pointer to a singleton of the volume controls.
     */
    static xPlayerPulseAudioControls* controls();

public slots:
    /**
     * Get the volume for the default sink
     *
     * @return the current volume in percent.
     */
    [[nodiscard]] int getVolume();
    /**
     * Set the volume of the default sink.
     *
     * @param vol the new volume in percent.
     */
    void setVolume(int vol);
    /**
     * Set the mute mode.
     *
     * @param mute enable mute if true, disable otherwise.
     */
    void setMuted(bool mute);

signals:
    /**
     * Signal emitted if volume is updated by a setVolume command.
     *
     * @param vol the new volume as integer.
     */
    void volume(int vol);
    /**
     * Signal emitted if the mute state is updated by a setMuted command.
     *
     * @param m the new mute state as boolean.
     */
    void muted(bool m);

private:
    typedef enum {
        PA_CONNECTING,
        PA_CONNECTED,
        PA_ERROR
    } pa_state_t;
    /**
     * Constructor. Create socket and connect signals.
     */
    xPlayerPulseAudioControls();
    /**
     * Destructor. Default.
     */
    ~xPlayerPulseAudioControls() override = default;


    // Pulseaudio functions.
    void paDefaultSinkInfo();
    void paIterate(pa_operation* op);
    // Callback for pulseaudio.
    static void paCallbackState(pa_context* context, void* data);
    static void paCallbackSinkList(pa_context* context, const pa_sink_info* info, int eol, void* data);

    QString defaultSinkName;
    QString defaultSinkDescription;
    pa_cvolume defaultSinkVolume;
    pa_volume_t defaultSinkVolumeAverage;
    int defaultSinkVolumePercent;
    pa_mainloop* paMainLoop;
    pa_mainloop_api* paMainLoopApi;
    pa_context* paContext;
    pa_state_t paState;
    int paRetValue;

    static xPlayerPulseAudioControls* paControls;
};

#endif
