/*
 * This file is part of the PyGoWave Desktop Client
 *
 * Copyright (C) 2009 Patrick Schneider <patrick.p2k.schneider@googlemail.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; see the file COPYING.  If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef WAVELETWIDGET_H
#define WAVELETWIDGET_H

#include <QtCore/QMap>
#include <QtGui/QWidget>
#include <QtWebKit/QWebPage>

namespace Ui {
    class WaveletWidget;
}

namespace PyGoWave {
	class Wavelet;
	class Controller;
	class JavaScriptAPI;
}

class ParticipantWidget;
class AddParticipantWindow;

class WaveletWidget : public QWidget
{
    Q_OBJECT

public:
	WaveletWidget(PyGoWave::Wavelet * wavelet, PyGoWave::Controller * controller, QWidget *parent = 0);
    ~WaveletWidget();

	PyGoWave::Wavelet * wavelet();

signals:
	void closing(const QByteArray &id);

protected:
    void changeEvent(QEvent *e);
	void closeEvent(QCloseEvent *e);

private:
	Ui::WaveletWidget * ui;
	PyGoWave::Wavelet * m_wavelet;
	PyGoWave::Controller * m_controller;
	PyGoWave::JavaScriptAPI * m_jsApi;
	QMap<QByteArray,ParticipantWidget*> m_participants;
	AddParticipantWindow * m_addPWindow;
	bool m_installedApi;

private slots:
	void on_cmdReply_clicked();
	void on_content_loadFinished(bool ok);
	void on_cmdAddParticipant_clicked();
	void on_jsApi_currentTextRangeChanged(int start, int end);

	void participantSelected(const QByteArray &id);
	void participantAddFinished();

	void wavelet_statusChange(const QByteArray & status);
	void wavelet_participantsChanged();
	void wave_waveletAboutToBeRemoved(const QByteArray & waveletId);

	void delay_loadFinished();
};

class ErrorLoggingWebPage : public QWebPage
{
public:
	ErrorLoggingWebPage(QObject * parent);

protected:
	void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
};

#endif // WAVELETWIDGET_H
