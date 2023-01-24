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
#include "xPlayerRotelWidget.h"
#include "xPlayerVolumeWidget.h"
#include "xPlayerUI.h"

#include <QList>
#include <QSpinBox>


xPlayerRotelWidget::xPlayerRotelWidget(QWidget *parent, Qt::Orientation orientation, Qt::WindowFlags flags):
        QWidget(parent, flags) {
    auto rotelLayout = new xPlayerLayout(this);
    rotelLayout->setSpacing(xPlayerLayout::NoSpace);
    // Create a volume knob.
    rotelVolume = new xPlayerVolumeWidget(this);
    // Create rotel control buttons widget.
    auto rotelControlButton = new QWidget(this);
    auto rotelControlButtonLayout = new xPlayerLayout();
    // Create a combo box for the input sources and add valid sources.
    rotelSource = new QComboBox(rotelControlButton);
    for (const auto& source : Rotel::Sources) {
        rotelSource->addItem(source);
    }
    // Insert separator and the special power off source.
    rotelSource->insertSeparator(rotelSource->count());
    rotelSource->addItem(Rotel::Source_PowerOff);
    rotelBass = new QSpinBox(rotelControlButton);
    rotelBass->setRange(-10, 10);
    rotelTreble = new QSpinBox(rotelControlButton);
    rotelTreble->setRange(-10, 10);
    rotelBassLabel = new QLabel(tr("Bass"), rotelControlButton);
    rotelBassLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    rotelTrebleLabel = new QLabel(tr("Treble"), rotelControlButton);
    rotelTrebleLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    rotelBalanceLabel = new QLabel(tr("Balance"), rotelControlButton);
    rotelBalanceLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    rotelBalance = new xPlayerBalanceWidget(rotelControlButton);
    rotelBalance->setRange(15);
    rotelControlButtonLayout->addRowStretcher(0);
    rotelControlButtonLayout->addWidget(rotelSource, 1, 0, 1, 2);
    rotelControlButtonLayout->addRowSpacer(2, xPlayerLayout::SmallSpace);
    rotelControlButtonLayout->addWidget(rotelBassLabel, 3, 0);
    rotelControlButtonLayout->addWidget(rotelTrebleLabel, 3, 1);
    rotelControlButtonLayout->addWidget(rotelBass, 4, 0);
    rotelControlButtonLayout->addWidget(rotelTreble, 4, 1);
    rotelControlButtonLayout->addRowSpacer(5, xPlayerLayout::SmallSpace);
    rotelControlButtonLayout->addWidget(rotelBalanceLabel, 6, 0, 1, 2);
    rotelControlButtonLayout->addWidget(rotelBalance, 7, 0, 1, 2);
    rotelControlButtonLayout->addRowStretcher(8);
    rotelControlButtonLayout->setContentsMargins(0, 0, 0, 0);
    rotelControlButton->setLayout(rotelControlButtonLayout);
    rotelControlButton->setContentsMargins(0, 0, 0, 0);
    rotelControlButton->setFixedWidth(xPlayer::ControlButtonWidgetWidth);
    // Add the volume knob, bass, treble and the source input to the layout
    if (orientation == Qt::Horizontal) {
        rotelLayout->addWidget(rotelVolume, 0, 0);
        rotelLayout->addColumnSpacer(1, xPlayerLayout::MediumSpace);
        rotelLayout->addWidget(rotelControlButton, 0, 2);
    } else {
        rotelLayout->addWidget(rotelVolume, 0, 0);
        rotelLayout->addRowSpacer(1, xPlayerLayout::MediumSpace);
        // Adjust width for vertical layout.
        rotelVolume->setFixedWidth(xPlayer::ControlButtonWidgetWidth);
        rotelLayout->addWidget(rotelControlButton, 1, 0);
    }
    rotelControls = xPlayerRotelControls::controls();
    // Connect the widgets to the amp commands.
    QObject::connect(rotelVolume, &xPlayerVolumeWidget::volume, rotelControls, &xPlayerRotelControls::setVolume);
    QObject::connect(rotelVolume, &xPlayerVolumeWidget::muted, rotelControls, &xPlayerRotelControls::setMuted);
    QObject::connect(rotelBass, SIGNAL(valueChanged(int)), rotelControls, SLOT(setBass(int)));
    QObject::connect(rotelTreble, SIGNAL(valueChanged(int)), rotelControls, SLOT(setTreble(int)));
    QObject::connect(rotelSource, SIGNAL(currentTextChanged(QString)), rotelControls, SLOT(setSource(QString)));
    QObject::connect(rotelBalance, &xPlayerBalanceWidget::balance, rotelControls, &xPlayerRotelControls::setBalance);
    // Connect Rotel controls to Rotel widget.
    QObject::connect(rotelControls, &xPlayerRotelControls::volume, this, &xPlayerRotelWidget::updateVolume);
    QObject::connect(rotelControls, &xPlayerRotelControls::bass, this, &xPlayerRotelWidget::updateBass);
    QObject::connect(rotelControls, &xPlayerRotelControls::treble, this, &xPlayerRotelWidget::updateTreble);
    QObject::connect(rotelControls, &xPlayerRotelControls::balance, this, &xPlayerRotelWidget::updateBalance);
    QObject::connect(rotelControls, &xPlayerRotelControls::source, this, &xPlayerRotelWidget::updateSource);
    QObject::connect(rotelControls, &xPlayerRotelControls::muted, this, &xPlayerRotelWidget::updateMuted);
    QObject::connect(rotelControls, &xPlayerRotelControls::connected, this, &xPlayerRotelWidget::connected);
    QObject::connect(rotelControls, &xPlayerRotelControls::disconnected, this, &xPlayerRotelWidget::disconnected);
    // Disable until connected.
    disconnected();
}

void xPlayerRotelWidget::updateVolume(int vol) {
    qDebug() << "xPlayerRotel: updateVolume: " << vol;
    QObject::disconnect(rotelVolume, &xPlayerVolumeWidget::volume, rotelControls, &xPlayerRotelControls::setVolume);
    rotelVolume->setVolume(vol);
    QObject::connect(rotelVolume, &xPlayerVolumeWidget::volume, rotelControls, &xPlayerRotelControls::setVolume);
}

void xPlayerRotelWidget::updateBass(int b) {
    qDebug() << "xPlayerRotel: updateBass: " << b;
    QObject::disconnect(rotelBass, SIGNAL(valueChanged(int)), rotelControls, SLOT(setBass(int)));
    rotelBass->setValue(b);
    QObject::connect(rotelBass, SIGNAL(valueChanged(int)), rotelControls, SLOT(setBass(int)));
}

void xPlayerRotelWidget::updateTreble(int t) {
    qDebug() << "xPlayerRotel: updateTreble: " << t;
    QObject::disconnect(rotelTreble, SIGNAL(valueChanged(int)), rotelControls, SLOT(setTreble(int)));
    rotelTreble->setValue(t);
    QObject::connect(rotelTreble, SIGNAL(valueChanged(int)), rotelControls, SLOT(setTreble(int)));
}

void xPlayerRotelWidget::updateBalance(int b) {
    qDebug() << "xPlayerRotel: updateBalance: " << b;
    QObject::disconnect(rotelBalance, &xPlayerBalanceWidget::balance, rotelControls, &xPlayerRotelControls::setBalance);
    rotelBalance->setBalance(b);
    QObject::connect(rotelBalance, &xPlayerBalanceWidget::balance, rotelControls, &xPlayerRotelControls::setBalance);
}

void xPlayerRotelWidget::updateSource(const QString& source) {
    qDebug() << "xPlayerRotel: updateSource: " << source;
    QObject::disconnect(rotelSource, SIGNAL(currentTextChanged(QString)), rotelControls, SLOT(setSource(QString)));
    rotelSource->setCurrentIndex(Rotel::Sources.indexOf(source));
    QObject::connect(rotelSource, SIGNAL(currentTextChanged(QString)), rotelControls, SLOT(setSource(QString)));
}

void xPlayerRotelWidget::updateMuted(bool mute) {
    qDebug() << "xPlayerRotel: updateMute: " << mute;
    // No disconnect necessary. Own function does not issue signal.
    rotelVolume->setMuted(mute);
}

void xPlayerRotelWidget::connected() {
    qDebug() << "xPlayerRotel: connected";
    // Enable all widgets.
    rotelVolume->setEnabled(true);
    rotelSource->setEnabled(true);
    rotelTrebleLabel->setEnabled(true);
    rotelTreble->setEnabled(true);
    rotelBassLabel->setEnabled(true);
    rotelBass->setEnabled(true);
    rotelBalanceLabel->setEnabled(true);
    rotelBalance->setEnabled(true);
}

void xPlayerRotelWidget::disconnected() {
    qDebug() << "xPlayerRotel: disconnected";
    rotelVolume->setEnabled(false);
    rotelSource->setEnabled(false);
    rotelTrebleLabel->setEnabled(false);
    rotelTreble->setEnabled(false);
    rotelBassLabel->setEnabled(false);
    rotelBass->setEnabled(false);
    rotelBalanceLabel->setEnabled(false);
    rotelBalance->setEnabled(false);
}

