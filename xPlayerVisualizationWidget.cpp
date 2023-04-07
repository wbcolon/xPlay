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
 * Some code borrowed form qmmp project
 * https://sourceforge.net/projects/qmmp-dev/
 */
#include "xPlayerVisualizationWidget.h"
#include "xPlayerConfiguration.h"
#include "xPlayerUI.h"

#include <QDialog>
#include <QComboBox>
#include <QMouseEvent>
#include <QEvent>
#include <QMenu>
#include <QDebug>

#include <cstring>

xPlayerVisualizationWidget::xPlayerVisualizationWidget(QWidget *parent):
        QOpenGLWidget(parent),
        visualization(nullptr),
        visualizationPresetIndex(0),
        visualizationPresetMenu(nullptr),
        visualizationPresetMap(),
        visualizationEnabled(true),
        visualizationFullWindowMode(false) {
}

xPlayerVisualizationWidget::~xPlayerVisualizationWidget() {
    delete visualization;
}

void xPlayerVisualizationWidget::initializeGL() {
    if (!visualizationEnabled) {
        qWarning() << "Problems with projectM. Visualization disabled.";
        return;
    }
    // Create projectM object if necessary.
    if (visualization == nullptr) {
        try {
            visualizationConfigPath = xPlayerConfiguration::configuration()->getVisualizationConfigPath();
            // We need to check the projectM configuration before trying to create an object.
            // Invalid file path may lead to crashes or a hanging application.
            // Maybe removed if projectM code is improved on.
            visualizationEnabled = checkVisualizationConfigFile();
            if (visualizationEnabled) {
                visualization = new projectM(visualizationConfigPath.toStdString());
            } else {
                emit visualizationError();
                return;
            }
        } catch (...) {
            // Problems creating projectM object.
            visualization = nullptr;
            visualizationEnabled = false;
            qCritical() << "Unable to initialize projectM. Check your projectM configuration.";
            emit visualizationError();
            return;
        }

        if (visualization) {
            auto configPreset = xPlayerConfiguration::configuration()->getVisualizationPreset();
            visualizationPresetIndex = 0;
            // Initialize widget.
            initializeVisualizationGL();
            // Keep the currently selected preset. Do no switch after a certain time.
            visualization->setPresetLock(true);
            // Process the presets.
            visualizationPresetMap.clear();
            for (unsigned i = 0; i < visualization->getPlaylistSize(); ++i) {
                auto presetName = QString::fromStdString(visualization->getPresetName(i));
                if (presetName == configPreset) {
                    visualizationPresetIndex = i;
                }
                // Split up the preset name.
                auto presetSplit = presetName.split(" - ");
                if (presetSplit.size() >= 2) {
                    auto presetAuthor = presetSplit.takeFirst();
                    auto presetType = presetSplit.join(" - ");
                    if (visualizationPresetMap.find(presetAuthor) == visualizationPresetMap.end()) {
                        visualizationPresetMap[presetAuthor] = std::list<std::pair<int, QString>>{};
                    }
                    visualizationPresetMap[presetAuthor].emplace_back(std::make_pair(i, presetType));
                }
            }
            // Select saved preset and update preset name.
            visualization->selectPreset(visualizationPresetIndex);

            // Delete old and create new menu from presets.
            delete visualizationPresetMenu;
            visualizationPresetMenu = new QMenu();
            visualizationPresetMenu->setStyleSheet("QMenu { menu-scrollable: 1; }");
            for (const auto &preset: visualizationPresetMap) {
                auto titleMenu = visualizationPresetMenu->addMenu(preset.first);
                for (const auto &title: preset.second) {
                    titleMenu->addAction(title.second, [=]() {
                        visualizationPresetIndex = title.first;
                        visualization->selectPreset(visualizationPresetIndex, true);
                        xPlayerConfiguration::configuration()->setVisualizationPreset(preset.first + " - " + title.second);
                    });
                }
            }
        }
    }
    QOpenGLWidget::initializeGL();
}

void xPlayerVisualizationWidget::initializeVisualizationGL() {
    // Initialize OpenGL. Borrowed from qmmp and projectM code.
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0);
    // Setup our viewport
    glViewport(0, 0, width(), height());
    // Change to the projection matrix and set our viewing volume.
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glLineStipple(2, 0xAAAA);
}

void xPlayerVisualizationWidget::resizeGL(int glWidth, int glHeight) {
    if (visualizationEnabled) {
        visualization->projectM_resetGL(glWidth, glHeight);
        initializeGL();
    }
    QOpenGLWidget::resizeGL(glWidth, glHeight);
}

void xPlayerVisualizationWidget::paintGL() {
    if (visualizationEnabled) {
        visualization->renderFrame();
        update();
    }
    QOpenGLWidget::paintGL();
}

bool xPlayerVisualizationWidget::event(QEvent* e) {
    if (visualizationEnabled) {
        switch (e->type()) {
            case QEvent::Show: {
                makeCurrent();
                emit visualizationFullWindow(visualizationFullWindowMode);
            } break;
            case QEvent::Hide: {
                doneCurrent();
            } break;
            case QEvent::KeyRelease: {
                auto keyEvent = reinterpret_cast<QKeyEvent *>(e);
                if (keyEvent->key() == Qt::Key_Escape) {
                    qDebug() << "ESC pressed. Exiting visualization.";
                    emit visualizationExiting();
                }
            } break;
            case QEvent::MouseButtonRelease: {
                auto mouseEvent = reinterpret_cast<QMouseEvent *>(e);
                if ((mouseEvent->button() == Qt::RightButton) && (visualizationPresetMenu)) {
                    visualizationPresetMenu->exec(mapToGlobal(mouseEvent->pos()));
                }
                if (mouseEvent->button() == Qt::LeftButton) {
                    if (mouseEvent->modifiers() & Qt::ControlModifier) {
                        if (mouseEvent->modifiers() & Qt::ShiftModifier) {
                            visualizationPresetIndex = (visualizationPresetIndex + 1) % visualization->getPlaylistSize();
                            visualization->selectPreset(visualizationPresetIndex);
                        }
                        showTitle(QString());
                        showTitle(QString::fromStdString(visualization->getPresetName(visualizationPresetIndex)));
                    }
                }
            } break;
            case QEvent::MouseButtonDblClick: {
                auto mouseEvent = reinterpret_cast<QMouseEvent *>(e);
                if (mouseEvent->button() == Qt::LeftButton) {
                    visualizationFullWindowMode = !visualizationFullWindowMode;
                    emit visualizationFullWindow(visualizationFullWindowMode);
                }
            } break;
            default: break;
        }
    }
    return QOpenGLWidget::event(e);
}

void xPlayerVisualizationWidget::showTitle(const QString& title) {
    if (visualizationEnabled) {
        visualization->projectM_setTitle(title.toStdString());
    }
}

void xPlayerVisualizationWidget::visualizationStereo(const QVector<qint16>& left, const QVector<qint16>& right) {
    // Did we configure the visualization.
    if ((visualizationEnabled) && (isVisible())) {
        short pcmData[2][512] {{0}};
        // Add data to the visualization. Only full sets of 512 samples.
        for (int i = 0; i < left.count()/512; ++i) {
            // Copy data to pcmData. Left channel is 0, right channel is 1.
            std::memcpy(pcmData[0], left.data()+(i * 512), 512 * sizeof(short));
            std::memcpy(pcmData[1], right.data()+(i * 512), 512 * sizeof(short));
            visualization->pcm()->addPCM16(pcmData);
        }
        paintGL();
    }
}

bool xPlayerVisualizationWidget::checkVisualizationConfigFile() {
    // Read the projectM config file.
    QSettings visualizationConfig(visualizationConfigPath, QSettings::NativeFormat);

    auto titleFont = visualizationConfig.value("Title Font").toString();
    auto menuFont = visualizationConfig.value("Menu Font").toString();
    auto presetPath = visualizationConfig.value("Preset Path").toString();

    // Extract the paths and remove any comments and unwanted whitespace.
    titleFont = titleFont.section("#", 0 , 0).trimmed();
    menuFont = menuFont.section("#", 0 , 0).trimmed();
    presetPath = presetPath.section("#", 0 , 0).trimmed();

    qDebug() << "projectM (Preset Path): " << presetPath;
    qDebug() << "projectM (Title Font): " << titleFont;
    qDebug() << "projectM (Menu Font): " << menuFont;

    // Check the font files and preset directory.
    if ((!std::filesystem::is_directory(presetPath.toStdString())) ||
        (!std::filesystem::is_regular_file(titleFont.toStdString())) ||
        (!std::filesystem::is_regular_file(menuFont.toStdString()))) {
        qCritical() << "Illegal entries in the projectM configuration. Disable visualization.";
        return false;
    }

    return true;
}
