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
/*
 * Source code inspired by
 * - https://github.com/pulseaudio/pavucontrol
 * - https://github.com/cdemoulins/pamixer
 * - https://github.com/dbarac/pulseaudio-spectrum-visualizer
 */
#include "xPlayerPulseAudioControls.h"
#include "xPlayerAnalyzerWidget.h"

#include <QThread>
#include <QDebug>
#include <cmath>
#include <qdatetime.h>

const pa_sample_spec xPlay_paSampleSpec = {
    PA_SAMPLE_FLOAT32LE,
    44100,
    1
};

const pa_buffer_attr xPlay_paBufferAttr = {
    xPlayer::AnalyzerMaxNoOfSamples / 4,  // Only reserve buffer for a quarter of samples read.
    (uint32_t)-1,
    (uint32_t)-1,
    (uint32_t)-1,
    (uint32_t)-1,
};

xPlayerPulseAudioControls* xPlayerPulseAudioControls::paControls = nullptr;

xPlayerPulseAudioControls::xPlayerPulseAudioControls():
    QObject(),
    defaultSinkName(),
    defaultMonitor(),
    defaultSinkDescription(),
    defaultSinkVolume(),
    defaultSinkVolumeAverage(0),
    defaultSinkVolumePercent(0),
    paState(PA_DISCONNECTED),
    paRetValue(0),
    paMonitorStream(nullptr),
    paMonitorTimer(nullptr) {
    // Only create the main loop here. Do not connect to the pulseaudio server here.
    paMainLoop = pa_mainloop_new();
    paMainLoopApi = pa_mainloop_get_api(paMainLoop);
    paContext = pa_context_new(paMainLoopApi, "xPlayerPulseAudioControls");
    // Allocate sample buffer.
    paMonitorBuffer = new float[xPlayer::AnalyzerMaxNoOfSamples];
    // Create monitor time and connect signal. Do not start yet.
    paMonitorTimer = new QTimer(this);
    paMonitorTimer->setTimerType(Qt::PreciseTimer);
    connect(paMonitorTimer, &QTimer::timeout, this, &xPlayerPulseAudioControls::paMonitorThread);
}

xPlayerPulseAudioControls::~xPlayerPulseAudioControls() {
    // Free sample buffer.
    delete[] paMonitorBuffer;
    // Free monitor stream.
    pa_simple_free(paMonitorStream);
    // Reset state, disconnect and free the main loop.
    pa_context_set_state_callback(paContext, nullptr, nullptr);
    paDisconnect();
    pa_context_unref(paContext);
    pa_mainloop_free(paMainLoop);
}

xPlayerPulseAudioControls* xPlayerPulseAudioControls::controls() {
    // Create and return singleton controls.
    // We have several volume widgets that share these controls and need to be updated whenever one of
    // the widgets does change the settings.
    if (paControls == nullptr) {
        paControls = new xPlayerPulseAudioControls;
    }
    return paControls;
}

int xPlayerPulseAudioControls::getVolume() {
    // Retrieve volume (in percent) only if connected to pulseaudio.
    if (paConnect()) {
        defaultSinkVolumePercent = static_cast<int>(round(defaultSinkVolumeAverage*100.0/PA_VOLUME_NORM));
        return defaultSinkVolumePercent;
    } else {
        return -1;
    }
}

void xPlayerPulseAudioControls::setVolume(int vol) {
    // Only set volume if connected to pulseaudio.
    if (paConnect()) {
        // Translate volume percentages into the corresponding values for pulseaudio.
        auto paVolume = PA_VOLUME_NORM * vol / 100;
        pa_cvolume_set(&defaultSinkVolume, defaultSinkVolume.channels, paVolume);
        pa_operation *op = pa_context_set_sink_volume_by_index(paContext, 0, &defaultSinkVolume, nullptr, nullptr);
        paIterate(op);
        pa_operation_unref(op);
        defaultSinkVolumePercent = vol;
        emit volume(defaultSinkVolumePercent);
    }
}

void xPlayerPulseAudioControls::setMuted(bool mute) {
    // Only set the mute state if connected to pulseaudio.
    // Emulate previous mute behavior by saving prior volume setting.
    // Therefore, slightly different to setVolume(0)
    if (paConnect()) {
        auto paVolume = (mute) ? PA_VOLUME_MUTED : PA_VOLUME_NORM * defaultSinkVolumePercent / 100;
        pa_cvolume_set(&defaultSinkVolume, defaultSinkVolume.channels, paVolume);
        pa_operation *op = pa_context_set_sink_volume_by_index(paContext, 0, &defaultSinkVolume, nullptr, nullptr);
        paIterate(op);
        pa_operation_unref(op);
        emit muted(mute);
    }
}

void xPlayerPulseAudioControls::enableMonitor() {
#ifdef XPLAYER_USE_ANALYZER
    if (paConnect()) {
        if (!paMonitorTimer->isActive()) {
            paMonitorTimer->start(xPlayer::AnalyzerRepeatMS);
        }
    }
#endif
}

void xPlayerPulseAudioControls::disableMonitor() {
#ifdef XPLAYER_USE_ANALYZER
    if (paMonitorTimer->isActive()) {
        paMonitorTimer->stop();
        // Send cleanup samples.
        memset(paMonitorBuffer, 0, xPlayer::AnalyzerMaxNoOfSamples*sizeof(float));
        emit monitorSamples(paMonitorBuffer, xPlayer::AnalyzerMaxNoOfSamples);
    }
#endif
}

void xPlayerPulseAudioControls::paMonitorThread() {
    int paError;

    if (paMonitorTimer) {
        if (pa_simple_read(paMonitorStream, paMonitorBuffer, xPlayer::AnalyzerMaxNoOfSamples*sizeof(float), &paError) >= 0) {
            emit monitorSamples(paMonitorBuffer, xPlayer::AnalyzerMaxNoOfSamples);
        } else {
            qDebug() << "sample reading failed";
        }
    }

}

bool xPlayerPulseAudioControls::paConnect() {
    if (paState == PA_CONNECTED) {
        return true;
    } else {
        int paError = -1;

        pa_context_set_state_callback(paContext, &xPlayerPulseAudioControls::paCallbackState, this);
        if (pa_context_connect(paContext, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
            emit error("Unable to connect to pulse audio. Unable to control audio.");
            return false;
        }
        paState = PA_CONNECTING;
        while (paState == PA_CONNECTING) {
            if (pa_mainloop_iterate(paMainLoop, 1, &paRetValue) < 0) {
                emit error(QString("Error while iterating pulse audio operations. Error code: %1").arg(paRetValue));
                return false;
            }
        }
        if (paState == PA_ERROR) {
            emit error("Error trying to connect to pulse audio. Unable to control audio.");
        }
        pa_operation* op = pa_context_get_server_info(paContext, &xPlayerPulseAudioControls::paCallbackServerInfo, this);
        paIterate(op);
        pa_operation_unref(op);
        qDebug() << "Default Sink: Name: " << QString::fromStdString(defaultSinkName);
        qDebug() << "Default Sink: Monitor: " << QString::fromStdString(defaultMonitor);
        // Get info for default sink only if we connect to pulseaudio.
        op = pa_context_get_sink_info_by_name(paContext, defaultSinkName.c_str(), &xPlayerPulseAudioControls::paCallbackSinkList, this);
        paIterate(op);
        pa_operation_unref(op);
        qDebug() << "Default Sink: Description: " << defaultSinkDescription;
        qDebug() << "Default Sink: Volume: " << defaultSinkVolumePercent;
        qDebug() << "Default Sink: Channels: " << defaultSinkVolume.channels;
        /* Create monitor stream. */
        paMonitorStream = pa_simple_new(nullptr, "xPlayerPulseAudioMonitor", PA_STREAM_RECORD,
                                        defaultMonitor.c_str(),
                                        "record", &xPlay_paSampleSpec, nullptr, &xPlay_paBufferAttr, &paError);
        qDebug() << "xPlayerPulseAudioControls: error: " << paError;
        return true;
    }
}

void xPlayerPulseAudioControls::paDisconnect() {
    if (paState == PA_CONNECTED) {
        pa_context_disconnect(paContext);
    }
}

void xPlayerPulseAudioControls::paIterate(pa_operation* op) {
    // Iterate over the pulseaudio main loop until the specified operation was processed.
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
        pa_mainloop_iterate(paMainLoop, 1, &paRetValue);
        // Micro sleep for 100ms
        QThread::msleep(100);
    }
}

void xPlayerPulseAudioControls::paCallbackState(pa_context* context, void* data) {
    // Callback for observing state changes while trying to connect to the pulseaudio server.
    auto vControls = reinterpret_cast<xPlayerPulseAudioControls*>(data);
    switch(pa_context_get_state(context)) {
        case PA_CONTEXT_READY: {
            vControls->paState = PA_CONNECTED;
        } break;
        case PA_CONTEXT_FAILED: {
            vControls->paState = PA_ERROR;
        } break;
        default: break;
    }
}

void xPlayerPulseAudioControls::paCallbackServerInfo(pa_context* context, const pa_server_info* info, void* data) {
    // Callback for observing the server info
    auto vControls = reinterpret_cast<xPlayerPulseAudioControls*>(data);
    switch(pa_context_get_state(context)) {
        case PA_CONTEXT_READY: {
            vControls->defaultSinkName = info->default_sink_name;
            vControls->defaultMonitor = info->default_sink_name + std::string(".monitor");
        } break;
        case PA_CONTEXT_FAILED: {
            vControls->defaultSinkName.clear();
        } break;
        default: break;
    }
}

void xPlayerPulseAudioControls::paCallbackSinkList(pa_context* context, const pa_sink_info* info, int eol, void* data) {
    Q_UNUSED(context)
    // Check if any error occurred parsing the sink info.
    if (eol != 0) {
        return;
    }
    // Save the necessary information required for volume control.
    auto vControls = reinterpret_cast<xPlayerPulseAudioControls*>(data);
    vControls->defaultSinkDescription = info->description;
    vControls->defaultSinkVolume = info->volume;
    vControls->defaultSinkVolumeAverage = pa_cvolume_avg(&info->volume);
}


