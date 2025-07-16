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

#include "xPlayerAnalyzerWidget.h"

#include <QFrame>
#include <QResizeEvent>


xPlayerAnalyzerWidget::xPlayerAnalyzerWidget(int aRows, int aColumns, int aSize, QWidget* parent,
                                             Qt::WindowFlags flags):
    QWidget(parent, flags),
    analyzerRows(aRows),
    analyzerColumns(aColumns),
    analyzerSize(aSize),
	analyzerFrames(nullptr) {

	resizeAnalyzerFrames(aRows, aColumns);
	reverseIndexBits();
	setFixedHeight(sizeHint().height());
}

void xPlayerAnalyzerWidget::resizeAnalyzerFrames(int rows, int columns) {

	if (analyzerFrames) {
		for (int i = 0; i < analyzerRows; ++i) {
			for (int j = 0; j < analyzerColumns; ++j) {
				delete analyzerFrames[i][j];
			}
			delete[] analyzerFrames[i];
		}
		delete[] analyzerFrames;
		analyzerFrames = nullptr;
		delete analyzerLayout;
	}

	analyzerRows = rows;
	analyzerColumns = columns;
	// Setup the layout for the analyzer grid.
	setContentsMargins(0, 0, 0, 0);
	analyzerLayout = new xPlayerLayout(this);
	analyzerLayout->setContentsMargins(0, 0, 0, 0);
	analyzerLayout->addColumnStretcher(0, 8);
	analyzerLayout->addColumnStretcher(analyzerColumns+1, 8);
	analyzerLayout->setSpacing(xPlayer::AnalyzerFrameSpaceing);
	// Create the analyzer frame grid.
	analyzerFrames = new QFrame**[analyzerRows];
	for (int i = 0; i < analyzerRows; ++i) {
		analyzerFrames[i] = new QFrame*[analyzerColumns];
		for (int j = 0; j < analyzerColumns; ++j) {
			analyzerFrames[i][j] = new QFrame(this);
			analyzerFrames[i][j]->setFixedSize(analyzerSize, analyzerSize);
			analyzerFrames[i][j]->setFrameStyle(QFrame::Plain);
			analyzerFrames[i][j]->setAutoFillBackground(true);
			analyzerFrames[i][j]->setPalette(QPalette(Qt::darkGray));
			analyzerFrames[i][j]->setContentsMargins(0, 0, 0, 0);
			analyzerLayout->addWidget(analyzerFrames[i][j], i, j + 1);
		}
	}
	setLayout(analyzerLayout);
}



void xPlayerAnalyzerWidget::reverseIndexBits() {
	uint16_t reversed;
	uint16_t bits = static_cast<uint16_t>(log2(xPlayer::AnalyzerMaxNoOfSamples));

	for (uint16_t i = 0; i < xPlayer::AnalyzerMaxNoOfSamples; i++) {
		analyzerIndices[i] = i;
		reversed = 0;
		for (uint16_t bit = 0; bit < bits; bit++) {
			reversed <<= 1;
			reversed += analyzerIndices[i] & 1;
			analyzerIndices[i] >>= 1;
		}
		analyzerIndices[i] = reversed;
	}
}

void xPlayerAnalyzerWidget::fft(std::complex<double>* result, const float* buffer, uint16_t noSamples) {
	//smallest FFT subproblem, each frequency bin is the same as the sample
	//with index = indices[bin] = reverse_bits(bin)
	for (uint16_t bin = 0; bin < noSamples; bin++) {
		result[bin].real(buffer[analyzerIndices[bin]]);
		result[bin].imag(0.0);
	}
	//combine solutions to FFT subproblems
	//M = current subproblem size (number of bins)
	for (uint16_t M = 2; M <= noSamples; M *= 2) {
		for (uint16_t subproblem = 0; subproblem < noSamples / M; subproblem++) {
			for (uint16_t k = 0; k < M / 2; k++) {
				std::complex<double> w = {0, -2 * M_PI * k / M};
				auto twiddle_factor = exp(w);

				uint16_t even_index = subproblem * M + k;
				uint16_t odd_index = subproblem * M + k + M / 2;
				auto even = result[even_index];
				auto odd = twiddle_factor * result[odd_index];

				result[even_index] = even + odd;
				result[odd_index] = even - odd;
			}
		}
	}
}

void xPlayerAnalyzerWidget::applyHannWindow(float* samples, uint16_t noSamples)
{
	for (uint16_t i = 0; i < noSamples; i++) {
		auto ratio = static_cast<float>(i) / static_cast<float>(noSamples - 1);
		auto weight = static_cast<float>(0.5 * (1 - cos(2 * M_PI * ratio)));
		samples[i] *= weight;
	}
}

void xPlayerAnalyzerWidget::applyHammingWindow(float* samples, uint16_t noSamples)
{
	for (uint16_t i = 0; i < noSamples; i++) {
		auto ratio = static_cast<float>(i) / static_cast<float>(noSamples - 1);
		auto weight = static_cast<float>(0.54 * (0.46 * cos(2 * M_PI * ratio)));
		samples[i] *= weight;
	}
}

void xPlayerAnalyzerWidget::displaySpectrum(std::complex<double>* result, uint16_t noSamples)
{
	int bins = noSamples / 4 / analyzerColumns; //display 1/4 of result bins
	for (int i = 0; i < analyzerColumns; i++) {
		int magnitude = 0;
		for (int j = 0; j < bins; j++) {
			auto freq = result[bins * i + j];
			magnitude += static_cast<int>(xPlayer::AnalyzerScalingFactor * sqrt(
				freq.real() * freq.real() + freq.imag() * freq.imag()));
		}
		magnitude /= bins;
		for (int j = 0; j < analyzerRows; j++) {
			if (magnitude >= analyzerRows - j) {
				analyzerFrames[j][i]->setPalette(QPalette(Qt::blue));
				analyzerFrames[j][i]->setAutoFillBackground(true);
			} else {
				analyzerFrames[j][i]->setPalette(QPalette(Qt::darkGray));
				analyzerFrames[j][i]->setAutoFillBackground(true);
			}
		}
	}
}

void xPlayerAnalyzerWidget::updateAnalyzer(float* samples, int noSamples) {

	if (noSamples > xPlayer::AnalyzerMaxNoOfSamples) {
		noSamples = xPlayer::AnalyzerMaxNoOfSamples;
	}
	applyHammingWindow(samples, noSamples);
	// applyHannWindow(samples, noSamples);
	fft(analyzerResult, samples, noSamples);
	displaySpectrum(analyzerResult, noSamples);
}

void xPlayerAnalyzerWidget::resizeEvent(QResizeEvent* event) {
	if (event != nullptr) {
		qDebug() << "Old Size: " << event->oldSize();
		qDebug() << "OldNo: " << analyzerColumns;
		// width divided by size plus spacing pixel for the space in-between.
		auto updatedColums = event->size().width()/(analyzerSize+xPlayer::AnalyzerFrameSpaceing);
		resizeAnalyzerFrames(analyzerRows, updatedColums);
		qDebug() << "Size: " << event->size();
		qDebug() << "No: " << updatedColums;
	}
	QWidget::resizeEvent(event);
}
