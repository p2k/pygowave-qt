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

#include <PyGoWaveApi/controller.h>
#include <PyGoWaveApi/model.h>
#include "model_qt.h"

#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QFocusEvent>
#include <QtCore/QSettings>
#include <QtGui/QDesktopWidget>

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
	delete this->ui->placeholderTab;
	if (settings.contains("WindowState"))
		this->restoreState(settings.value("WindowState").toByteArray());
	if (!settings.value("UseTabs").toBool()) {
		this->ui->waveTabs->hide();
		if (settings.contains("WindowGeometry"))
			this->setGeometry(settings.value("WindowGeometry").value<QRect>());
	}
	else {
		if (settings.contains("WindowGeometryTabbed")) {
			this->setGeometry(settings.value("WindowGeometryTabbed").value<QRect>());
			this->ui->splitter->restoreState(settings.value("WindowSplitTabbed").toByteArray());
		}
		else {
			this->resize(1024, this->height());
			this->ui->splitter->setSizes(QList<int>() << 310 << 714);
		}
	}

	if (!settings.value("Tray", true).toBool() || !settings.value("StartHidden", false).toBool())
		this->show();

	if (settings.value("InitialStatus", 0).toInt() == 1)
		this->on_actConnect_triggered();

	this->ui->waveTabs->addAction(this->ui->actDetachTab);

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
		if (this->waveletWidgets.contains(wavelet->id())) {
			WaveletWidget * ww = this->waveletWidgets[wavelet->id()];
			if (ww->parent() == NULL)
				ww->setFocus();
			else
				this->ui->waveTabs->setCurrentWidget(ww);
		}
		else
			this->controller->openWavelet(wavelet->id());
	}
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	QSettings settings;
	if (this->ui->waveTabs->isVisible()) {
		settings.setValue("WindowGeometryTabbed", this->geometry());
		settings.setValue("WindowSplitTabbed", this->ui->splitter->saveState());
	}
	else
		settings.setValue("WindowGeometry", this->geometry());
	settings.setValue("WindowState", this->saveState());
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

void MainWindow::on_waveTabs_tabCloseRequested(int index)
{
	WaveletWidget * ww = qobject_cast<WaveletWidget*>(this->ui->waveTabs->widget(index));
	ww->close();
	this->ui->actDetachTab->setEnabled(this->ui->waveTabs->count()-1 > 0);
}

void MainWindow::on_controller_waveletOpened(const QByteArray &waveletId, bool isRoot)
{
	if (!isRoot)
		return;
	PyGoWave::Wavelet * wavelet = this->controller->wavelet(waveletId);
	QSettings settings;
	WaveletWidget * ww;
	if (settings.value("UseTabs").toBool()) {
		ww = new WaveletWidget(wavelet, this->controller, this->ui->waveTabs);
		this->ui->waveTabs->addTab(ww, wavelet->title());
		this->ui->waveTabs->setCurrentWidget(ww);
		ww->setTabState(true, true);
		this->ui->actDetachTab->setEnabled(this);
	}
	else
		ww = new WaveletWidget(wavelet, this->controller);
	this->waveletWidgets[wavelet->id()] = ww;
	connect(ww, SIGNAL(closing(QByteArray)), this, SLOT(wavelet_closing(QByteArray)));
	connect(ww, SIGNAL(attachTabRequest()), this, SLOT(wavelet_attachRequest()));
	connect(ww, SIGNAL(detachTabRequest()), this, SLOT(wavelet_deachRequest()));
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
	if (!initial && !created) {
		this->indicator->showNotification(tr("New Wave"), tr("You have been added to the Wave \"%1\" created by %2.").arg(wavelet->title()).arg(wavelet->creator()->displayName()), 1);
		this->ui->waveList->scrollToBottom();
	}
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

	if (QSettings().value("UseTabs").toBool()) {
		if (!this->ui->waveTabs->isVisible())
			this->catchWavelets();
	}
	else {
		if (this->ui->waveTabs->isVisible())
			this->freeWavelets();
	}
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

void MainWindow::catchWavelets()
{
	QSettings settings;
	settings.setValue("WindowGeometry", this->geometry());
	
	if (settings.contains("WindowGeometryTabbed")) {
		this->setGeometry(settings.value("WindowGeometryTabbed").value<QRect>());
		this->ui->splitter->restoreState(settings.value("WindowSplitTabbed").toByteArray());
	}
	else {
		this->resize(1024, this->height());
		this->ui->splitter->setSizes(QList<int>() << 310 << 714);
	}
	this->ui->waveTabs->setVisible(true);
	foreach (WaveletWidget * ww, this->waveletWidgets.values()) {
		if (ww->parent() == NULL) {
			ww->hide();
			ww->setParent(this->ui->waveTabs);
			this->ui->waveTabs->addTab(ww, ww->wavelet()->title());
		}
		ww->setTabState(true, true);
	}
	this->ui->actDetachTab->setEnabled(this->ui->waveTabs->count() > 0);
}

void MainWindow::freeWavelets()
{
	static const int delta = 25;
	QSettings settings;
	settings.setValue("WindowGeometryTabbed", this->geometry());
	settings.setValue("WindowSplitTabbed", this->ui->splitter->saveState());

	this->ui->waveTabs->setVisible(false);
	QPoint pos = this->pos();
	QDesktopWidget * desktop = qApp->desktop();
	foreach (WaveletWidget * ww, this->waveletWidgets.values()) {
		if (ww->parent() != NULL) {
			this->ui->waveTabs->removeTab(this->ui->waveTabs->indexOf(ww));
			ww->setParent(NULL);
			if (pos.y() + ww->height() > desktop->height())
				pos.setY(desktop->height() - ww->height());
			if (pos.x() + ww->width() > desktop->width())
				pos.setX(desktop->width() - ww->width());
			if (pos.y() + delta + ww->height() < desktop->height())
				pos.ry() += delta;
			if (pos.x() + delta + ww->width() < desktop->width())
				pos.rx() += delta;
			ww->move(pos);
			ww->show();
		}
		ww->setTabState(false, false);
	}

	if (settings.contains("WindowGeometry"))
		this->setGeometry(settings.value("WindowGeometry").value<QRect>());
	else
		this->resize(314, this->height());
	this->ui->actDetachTab->setEnabled(false);
}

void MainWindow::on_actDetachTab_triggered()
{
	this->detachWavelet(qobject_cast<WaveletWidget*>(this->ui->waveTabs->currentWidget()));
}

void MainWindow::detachWavelet(WaveletWidget * ww)
{
	if (ww == NULL)
		return;
	int index = this->ui->waveTabs->indexOf(ww);
	if (index == -1)
		return;
	this->ui->waveTabs->removeTab(index);
	ww->setParent(NULL);
	QPoint pos = this->pos();
	QDesktopWidget * desktop = qApp->desktop();
	if (pos.y() + ww->height() > desktop->height())
		pos.setY(desktop->height() - ww->height());
	if (pos.x() + ww->width() > desktop->width())
		pos.setX(desktop->width() - ww->width());
	ww->move(pos);
	ww->show();
	ww->setTabState(true, false);
	this->ui->actDetachTab->setEnabled(this->ui->waveTabs->count() > 0);
}

void MainWindow::wavelet_attachRequest()
{
	WaveletWidget * ww = qobject_cast<WaveletWidget*>(this->sender());
	ww->hide();
	ww->setParent(this->ui->waveTabs);
	this->ui->waveTabs->addTab(ww, ww->wavelet()->title());
	this->ui->actDetachTab->setEnabled(true);
	ww->setTabState(true, true);
}

void MainWindow::wavelet_deachRequest()
{
	this->detachWavelet(qobject_cast<WaveletWidget*>(this->sender()));
}
