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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "onlinestateindicator.h"
#include "connectdialog.h"
#include "waveletwidget.h"
#include "preferencesdialog.h"

#include "PyGoWaveApi/controller.h"
#include "PyGoWaveApi/model.h"
#include "model_qt.h"

#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QFocusEvent>
#include <QtCore/QSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	this->controller = new PyGoWave::Controller(this);
	this->controller->setObjectName("controller");
	this->indicator = new OnlineStateIndicator(this);
	this->indicator->setObjectName("indicator");

	this->ui->setupUi(this);
	this->ui->statusBar->addPermanentWidget(this->indicator);
	this->ui->waveList->setModel(new PyGoWaveList(this->controller, this));
	this->ui->waveList->setContextMenuPolicy(Qt::ActionsContextMenu);
	this->ui->waveList->addAction(this->ui->actOpenWave);
	this->ui->waveList->addAction(this->ui->actLeaveWave);

	this->ui->statusBar->showMessage(tr("Welcome!"), 5000);

	QSettings settings;
	if (!settings.value("Tray", true).toBool() || !settings.value("StartHidden", false).toBool())
		this->show();

	if (settings.value("InitialStatus", 0).toInt() == 1)
		this->on_actConnect_triggered();

	QIcon icon;
	icon.addPixmap(QPixmap(":/gui/icons/PyGoWave-256.png"));
	icon.addPixmap(QPixmap(":/gui/icons/PyGoWave-48.png"));
	icon.addPixmap(QPixmap(":/gui/icons/PyGoWave-32.png"));
	icon.addPixmap(QPixmap(":/gui/icons/PyGoWave-24.png"));
	icon.addPixmap(QPixmap(":/gui/icons/PyGoWave-16.png"));
	this->setWindowIcon(icon);
}

MainWindow::~MainWindow()
{
	delete this->ui;
}

void MainWindow::on_actConnect_triggered()
{
	QSettings settings;
	QString pw;
	settings.beginGroup("Connection");
	if (settings.value("Hostname", "pygowave.net").toString().isEmpty()) {
		this->show();
		QMessageBox::warning(this, tr("Error while connecting"), tr("No hostname has been set. Please enter a hostname in the preferences dialog."));
		return;
	}
	if (settings.value("Password", "").toString().isEmpty() || settings.value("Username", "").toString().isEmpty()) {
		this->show();
		ConnectDialog dlg;
		if (!dlg.exec())
			return;
		pw = dlg.password();
	}
	else
		pw = QString::fromUtf8(QByteArray::fromBase64(settings.value("Password", "").toByteArray()));

	this->ui->statusBar->showMessage("Connecting...");
	this->indicator->setState(OnlineStateIndicator::StateConnecting);
	this->ui->actConnect->setEnabled(false);
	this->ui->actDisconnect->setEnabled(true);
	this->controller->connectToHost(settings.value("Hostname", "pygowave.net").toString(), settings.value("Username", "").toString(), pw, settings.value("Port", 61613).toInt());
}

void MainWindow::on_controller_stateChanged(int state)
{
	if (state == PyGoWave::Controller::ClientConnected)
		this->ui->statusBar->showMessage("Authenticating...");
	else if (state == PyGoWave::Controller::ClientDisconnected) {
		this->indicator->setState(OnlineStateIndicator::StateOffline);
		this->ui->statusBar->showMessage("Disconnected");
		this->ui->actConnect->setEnabled(true);
		this->ui->actDisconnect->setEnabled(false);
		this->ui->actNew->setEnabled(false);
	}
	else if (state == PyGoWave::Controller::ClientOnline) {
		this->indicator->setState(OnlineStateIndicator::StateOnline);
		this->ui->statusBar->showMessage("Online");
		this->ui->actNew->setEnabled(true);
	}
}

void MainWindow::on_controller_errorOccurred(const QByteArray &waveletId, const QString &tag, const QString &desc)
{
	this->show();
	if (waveletId == "login") {
		QMessageBox::critical(this, tr("Error while logging in"), desc);
		this->controller->disconnectFromHost();
	}
	else if (waveletId == "manager")
		QMessageBox::critical(this, tr("Error in manager connection"), tr("%1\n\nTag: %2").arg(desc, tag));
	else
		QMessageBox::critical(this, tr("Error in Wavelet %1").arg(QString::fromAscii(waveletId)), tr("%1\n\nTag: %2").arg(desc, tag));
}

void MainWindow::on_waveList_doubleClicked(const QModelIndex &index)
{
	PyGoWave::WaveModel * wave = this->controller->wave(index.data(PyGoWaveList::WaveIdRole).toByteArray());
	if (wave) {
		PyGoWave::Wavelet * wavelet = wave->rootWavelet();
		if (this->waveletWidgets.contains(wavelet->id()))
			this->waveletWidgets[wavelet->id()]->setFocus();
		else
			this->controller->openWavelet(wavelet->id());
	}
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	foreach (QByteArray id, this->waveletWidgets.keys()) {
		WaveletWidget * ww = this->waveletWidgets[id];
		if (!ww->close()) {
			event->ignore();
			return;
		}
	}
	this->controller->disconnectFromHost();
	QMainWindow::closeEvent(event);
}

void MainWindow::wavelet_closing(const QByteArray &id)
{
	this->controller->closeWavelet(id);
	this->waveletWidgets.take(id)->deleteLater();
}

void MainWindow::on_controller_waveletOpened(const QByteArray &waveletId, bool isRoot)
{
	if (!isRoot)
		return;
	PyGoWave::Wavelet * wavelet = this->controller->wavelet(waveletId);
	WaveletWidget * ww = new WaveletWidget(wavelet, this->controller);
	this->waveletWidgets[wavelet->id()] = ww;
	connect(ww, SIGNAL(closing(QByteArray)), this, SLOT(wavelet_closing(QByteArray)));
	ww->show();
}

void MainWindow::on_actNew_triggered()
{
	bool ok = false;
	QString title = QInputDialog::getText(this, tr("New Wave"), tr("Please enter a title:"), QLineEdit::Normal, QString(), &ok);
	if (!ok)
		return;
	this->controller->createNewWave(title);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
	QMessageBox::aboutQt(this, tr("PyGoWave Desktop Client is powered by Qt"));
}

void MainWindow::on_actDisconnect_triggered()
{
	foreach (QByteArray id, this->waveletWidgets.keys()) {
		WaveletWidget * ww = this->waveletWidgets[id];
		if (!ww->close())
			return;
	}
	this->controller->disconnectFromHost();
}

void MainWindow::on_controller_waveAdded(const QByteArray &waveId, bool created, bool initial)
{
	PyGoWave::Wavelet * wavelet = this->controller->wave(waveId)->rootWavelet();
	if (created)
		this->controller->openWavelet(wavelet->id());
	if (!initial)
		this->indicator->showNotification(tr("New Wave"), tr("You have been added to the Wave \"%1\" created by %2.").arg(wavelet->title()).arg(wavelet->creator()->displayName()), 1);
}

void MainWindow::on_actLeaveWave_triggered()
{
	if (QMessageBox::question(this, tr("Please confirm"), tr("Do you really want to leave the Wave?\n\nWarning: If you do so, you cannot come back to this wave unless someone adds you again."), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		return;
	QModelIndex index = this->ui->waveList->currentIndex();
	this->controller->leaveWavelet(index.data(PyGoWaveList::RootWaveletIdRole).toByteArray());
}

void MainWindow::on_actOpenWave_triggered()
{
	this->on_waveList_doubleClicked(this->ui->waveList->currentIndex());
}

void MainWindow::on_waveList_activated(const QModelIndex &index)
{
	this->ui->actLeaveWave->setEnabled(index.isValid());
	this->ui->actOpenWave->setEnabled(index.isValid());
}

void MainWindow::on_waveList_clicked(const QModelIndex &index)
{
	this->ui->actLeaveWave->setEnabled(index.isValid());
	this->ui->actOpenWave->setEnabled(index.isValid());
}

void MainWindow::on_action_Preferences_triggered()
{
	PreferencesDialog prefdlg;
	prefdlg.exec();
	this->indicator->updateTraySettings();
}

void MainWindow::on_indicator_stateChangeRequested(int istate)
{
	OnlineStateIndicator::OnlineState state = (OnlineStateIndicator::OnlineState) istate;
	switch (state) {
		case OnlineStateIndicator::StateOnline:
			this->on_actConnect_triggered();
			break;
		case OnlineStateIndicator::StateAway:
			break;
		case OnlineStateIndicator::StateBusy:
			break;
		case OnlineStateIndicator::StateInvisible:
			break;
		case OnlineStateIndicator::StateConnecting:
			break;
		case OnlineStateIndicator::StateOffline:
			this->on_actDisconnect_triggered();
			break;
	}
}

void MainWindow::focusInEvent(QFocusEvent * event)
{
	this->indicator->stopAnimation();
	QMainWindow::focusInEvent(event);
}
