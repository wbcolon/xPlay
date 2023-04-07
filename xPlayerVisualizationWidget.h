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
#ifndef __XPLAYERVISUALIZATIONWIDGET_H__
#define __XPLAYERVISUALIZATIONWIDGET_H__

#include <QOpenGLWidget>
#include <QMenu>
#include <libprojectM/projectM.hpp>

class xPlayerVisualizationWidget:public QOpenGLWidget {
    Q_OBJECT

public:
    explicit xPlayerVisualizationWidget(QWidget* parent = nullptr);
    ~xPlayerVisualizationWidget() override;

public slots:
    /**
     * Show the title as part of the visualization.
     *
     * @param title the title shown as string.
     */
    void showTitle(const QString& title);
    /**
     * Update current visualization with the given stereo data.
     *
     * The function always expects a multiple of 512 samples for the left and right channel.
     *
     * @param left the data for the left channel.
     * @param right the data for the right channel.
     */
    void visualizationStereo(const QVector<qint16>& left, const QVector<qint16>& right);

signals:
    /**
     * Signal for enable/disable the full window mode.
     */
    void visualizationFullWindow(bool enabled);
    /**
     * Signal for exiting visualization.
     */
    void visualizationExiting();
    /**
     * Emitted if we are unable to create a projectM instance.
     */
    void visualizationError();

protected:
    /**
     * Initialize OpenGL and projectM object.
     */
    void initializeGL() override;
    /**
     * Initialize OpenGL as required by projectM.
     */
    void initializeVisualizationGL();
    /**
     * Called upon resize of OpenGL widget.
     *
     * @param glWidth the new width as integer.
     * @param glHeight the new height as integer.
     */
    void resizeGL(int glWidth, int glHeight) override;
    /**
     * Paint the OpenGL widget.
     */
    void paintGL() override;
    /**
     * Called whenever an event is relevant for the widget.
     *
     * @param e the event triggered.
     * @return true if event was recognized, false otherwise.
     */
    bool event(QEvent* e) override;

private:
    /**
     * Check the font paths and preset directory in projectM config file.
     *
     * @return true if paths are correct, false otherwise.
     */
    bool checkVisualizationConfigFile();

    projectM* visualization;
    QString visualizationConfigPath;
    unsigned visualizationPresetIndex;
    QMenu* visualizationPresetMenu;
    std::map<QString, std::list<std::pair<int, QString>>> visualizationPresetMap;
    bool visualizationEnabled;
    bool visualizationFullWindowMode;
};

#endif
