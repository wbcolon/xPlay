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
#include "xPlayerPulseAudioControls.h"

#include <QDebug>
#include <cmath>

xPlayerPulseAudioControls* xPlayerPulseAudioControls::paControls = nullptr;

xPlayerPulseAudioControls::xPlayerPulseAudioControls():
        QObject(),
        defaultSinkName(),
        defaultSinkDescription(),
        defaultSinkVolume(),
        defaultSinkVolumeAverage(0),
        defaultSinkVolumePercent(0),
        paRetValue(0) {
    paMainLoop = pa_mainloop_new();

    paMainLoopApi = pa_mainloop_get_api(paMainLoop);
    paContext = pa_context_new(paMainLoopApi, "xPlayerPulseAudioControls");
    pa_context_set_state_callback(paContext, &xPlayerPulseAudioControls::paCallbackState, this);
    if (pa_context_connect(paContext, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        qCritical() << "Unable to connect to pulse audio. Unable to control audio.";
        return;
    }
    paState = PA_CONNECTING;
    while (paState == PA_CONNECTING) {
        if (pa_mainloop_iterate(paMainLoop, 1, &paRetValue) < 0) {
            qCritical() << "Error while iterating pulse audio operations. Error code: " << paRetValue;
            return;
        }
    }
    if (paState == PA_ERROR) {
        qCritical() << "Error trying to connect to pulse audio. Unable to control audio.";
        return;
    }
}

xPlayerPulseAudioControls* xPlayerPulseAudioControls::controls() {
    // Create and return singleton controls.
    if (paControls == nullptr) {
        paControls = new xPlayerPulseAudioControls;
    }
    return paControls;
}

int xPlayerPulseAudioControls::getVolume() {
    paDefaultSinkInfo();
    defaultSinkVolumePercent = static_cast<int>(round(defaultSinkVolumeAverage*100.0/PA_VOLUME_NORM));
    return defaultSinkVolumePercent;
}

void xPlayerPulseAudioControls::setVolume(int vol) {
    auto paVolume = PA_VOLUME_NORM * vol / 100;
    paDefaultSinkInfo();
    pa_cvolume* new_cvolume = pa_cvolume_set(&defaultSinkVolume, defaultSinkVolume.channels, paVolume);
    pa_operation* op;
    op = pa_context_set_sink_volume_by_index(paContext, 0, new_cvolume, nullptr, nullptr);
    paIterate(op);
    pa_operation_unref(op);
    defaultSinkVolumePercent = vol;
    emit volume(defaultSinkVolumePercent);
}

void xPlayerPulseAudioControls::setMuted(bool mute) {
    paDefaultSinkInfo();
    auto paVolume = (mute) ? PA_VOLUME_MUTED : PA_VOLUME_NORM * defaultSinkVolumePercent / 100;
    pa_cvolume* new_cvolume = pa_cvolume_set(&defaultSinkVolume, defaultSinkVolume.channels, paVolume);
    pa_operation* op;
    op = pa_context_set_sink_volume_by_index(paContext, 0, new_cvolume, nullptr, nullptr);
    paIterate(op);
    pa_operation_unref(op);
    emit muted(mute);
}

void xPlayerPulseAudioControls::paDefaultSinkInfo() {
    pa_operation* op = pa_context_get_sink_info_by_index(paContext, 0, &xPlayerPulseAudioControls::paCallbackSinkList, this);
    paIterate(op);
    pa_operation_unref(op);
    qDebug() << "Default Sink: Name: " << defaultSinkName;
    qDebug() << "Default Sink: Description: " << defaultSinkDescription;
    qDebug() << "Default Sink: Volume: " << defaultSinkVolumePercent;
}

void xPlayerPulseAudioControls::paIterate(pa_operation* op) {
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
        pa_mainloop_iterate(paMainLoop, 1, &paRetValue);
        usleep(100);
    }
}

void xPlayerPulseAudioControls::paCallbackState(pa_context* context, void* data) {
    auto vControls = reinterpret_cast<xPlayerPulseAudioControls*>(data);
    switch(pa_context_get_state(context)) {
        case PA_CONTEXT_READY: {
            vControls->paState = PA_CONNECTED;
        } break;
        case PA_CONTEXT_FAILED: {
            vControls->paState = PA_ERROR;
        } break;
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_TERMINATED: break;
    }
}

void xPlayerPulseAudioControls::paCallbackSinkList(pa_context* context, const pa_sink_info* info, int eol, void* data) {
    Q_UNUSED(context)
    if (eol != 0) {
        return;
    }
    auto vControls = reinterpret_cast<xPlayerPulseAudioControls*>(data);
    vControls->defaultSinkName = info->name;
    vControls->defaultSinkDescription = info->description;
    vControls->defaultSinkVolume = info->volume;
    vControls->defaultSinkVolumeAverage = pa_cvolume_avg(&info->volume);
}


