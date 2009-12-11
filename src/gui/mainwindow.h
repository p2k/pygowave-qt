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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QMap>

namespace Ui
{
    class MainWindow;
}

namespace PyGoWave
{
	class Controller;
}

class QModelIndex;
class WaveletWidget;
class OnlineStateIndicator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
	void closeEvent(QCloseEvent * event);
	void focusInEvent(QFocusEvent * event);

private:
	Ui::MainWindow * ui;
	PyGoWave::Controller * controller;
	OnlineStateIndicator * indicator;
	QMap<QByteArray,WaveletWidget*> waveletWidgets;

private slots:
	void on_action_Preferences_triggered();
	void on_waveList_clicked(const QModelIndex &index);
	void on_waveList_activated(const QModelIndex &index);
	void on_actOpenWave_triggered();
	void on_actLeaveWave_triggered();
	void on_actDisconnect_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actNew_triggered();
	void on_waveList_doubleClicked(const QModelIndex &index);
	void on_actConnect_triggered();

	void on_controller_stateChanged(int);
	void on_controller_errorOccurred(const QByteArray &waveletId, const QString &tag, const QString &desc);
	void on_controller_waveletOpened(const QByteArray &waveletId, bool isRoot);
	void on_controller_waveAdded(const QByteArray &waveId, bool created, bool initial);

	void on_indicator_stateChangeRequested(int state);

	void wavelet_closing(const QByteArray &id);
};

#endif // MAINWINDOW_H
