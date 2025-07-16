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


#ifndef __XPLAYERANALYZERWIDGET_H__
#define __XPLAYERANALYZERWIDGET_H__

#include <xPlayerUI.h>
#include <xPlayerConfiguration.h>

#include <QWidget>
#include <QFrame>

#include <complex>
#include <unistd.h>


class xPlayerAnalyzerWidget:public QWidget {
	Q_OBJECT

public:
	explicit xPlayerAnalyzerWidget(int aRows, int aColumns, int aSize, QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::WindowFlags());
    ~xPlayerAnalyzerWidget() override = default;

public slots:
	void updateAnalyzer(float* samples, int noSamples);

protected slots:
	void resizeEvent(QResizeEvent* event) override;

protected:
	void resizeAnalyzerFrames(int rows, int columns);
	void reverseIndexBits();
	void fft(std::complex<double>* result, const float* buffer, uint16_t noSamples);
	void displaySpectrum(std::complex<double>* result, uint16_t noSamples);
	static void applyHannWindow(float* samples, uint16_t noSamples);
	static void applyHammingWindow(float* samples, uint16_t noSamples);

    int analyzerRows;
    int analyzerColumns;
    int analyzerSize;
	QFrame*** analyzerFrames;
    xPlayerLayout* analyzerLayout;
	uint16_t analyzerIndices[xPlayer::AnalyzerMaxNoOfSamples]{};
	std::complex<double> analyzerResult[xPlayer::AnalyzerMaxNoOfSamples];
};

#endif /* __XPLAYERANALYZERWIDGET_H__ */