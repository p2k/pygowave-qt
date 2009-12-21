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

#include "onlinestateindicator.h"
#include "ui_onlinestateindicator.h"

#include <QtCore/QTimer>
#include <QtCore/QSettings>

#include <QtGui/QMenu>

OnlineStateIndicator::OnlineStateIndicator(QWidget *parent) : QWidget(parent), ui(new Ui::OnlineStateIndicator)
{
	this->blinkTimer = new QTimer(this);
	this->blinkTimer->setObjectName("blinkTimer");
	this->blinkTimer->setInterval(500);
	this->trayBlinkTimer = new QTimer(this);
	this->trayBlinkTimer->setObjectName("trayBlinkTimer");
	this->trayBlinkTimer->setInterval(500);
	this->ui->setupUi(this);
	this->m_actionsMenu = new QMenu(this);
	this->m_actionsMenu->addActions(QList<QAction*>()
		<< this->ui->actOnline
		<< this->ui->actAway
		<< this->ui->actBusy
		<< this->ui->actInvisible
		<< this->ui->actOffline
	);
	this->ui->icon->addActions(this->m_actionsMenu->actions());
	this->m_actionsMenu->addSeparator();
	this->m_actionsMenu->addAction(this->ui->actHide);
	this->ui->icon->setContextMenuPolicy(Qt::ActionsContextMenu);

	this->m_indicatorPixmaps[OnlineStateIndicator::StateOffline] = QPixmap(":/gui/icons/im-user-offline.png");
	this->m_indicatorPixmaps[OnlineStateIndicator::StateOnline] = QPixmap(":/gui/icons/im-user.png");
	this->m_indicatorPixmaps[OnlineStateIndicator::StateConnecting] = QPixmap(":/gui/icons/im-user.png");
	this->m_indicatorPixmaps[OnlineStateIndicator::StateAway] = QPixmap(":/gui/icons/im-user-away.png");
	this->m_indicatorPixmaps[OnlineStateIndicator::StateBusy] = QPixmap(":/gui/icons/im-user-busy.png");
	this->m_indicatorPixmaps[OnlineStateIndicator::StateInvisible] = QPixmap(":/gui/icons/im-user-invisible.png");

	this->m_trayIcons[OnlineStateIndicator::StateOffline] = QIcon(":/gui/icons/tray-offline-22.png");
	this->m_trayIcons[OnlineStateIndicator::StateOnline] = QIcon(":/gui/icons/tray-online-22.png");
	this->m_trayIcons[OnlineStateIndicator::StateConnecting] = QIcon(":/gui/icons/tray-online-22.png");
	this->m_trayIcons[OnlineStateIndicator::StateAway] = QIcon(":/gui/icons/tray-away-22.png");
	this->m_trayIcons[OnlineStateIndicator::StateBusy] = QIcon(":/gui/icons/tray-busy-22.png");
	this->m_trayIcons[OnlineStateIndicator::StateInvisible] = QIcon(":/gui/icons/tray-invisible-22.png");

	this->m_trayIcons[OnlineStateIndicator::StateOffline].addFile(":/gui/icons/tray-offline-16.png");
	this->m_trayIcons[OnlineStateIndicator::StateOnline].addFile(":/gui/icons/tray-online-16.png");
	this->m_trayIcons[OnlineStateIndicator::StateConnecting].addFile(":/gui/icons/tray-online-16.png");
	this->m_trayIcons[OnlineStateIndicator::StateAway].addFile(":/gui/icons/tray-away-16.png");
	this->m_trayIcons[OnlineStateIndicator::StateBusy].addFile(":/gui/icons/tray-busy-16.png");
	this->m_trayIcons[OnlineStateIndicator::StateInvisible].addFile(":/gui/icons/tray-invisible-16.png");

	this->m_state = OnlineStateIndicator::StateOffline;
	this->blinkState = false;
	this->trayBlinkState = false;
	this->m_tray = NULL;
	this->updateTraySettings();
}

OnlineStateIndicator::~OnlineStateIndicator()
{
	delete this->ui;
}

void OnlineStateIndicator::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
		this->ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void OnlineStateIndicator::on_blinkTimer_timeout()
{
	this->ui->icon->setPixmap(blinkState ? this->m_indicatorPixmaps[OnlineStateIndicator::StateConnecting] : this->m_indicatorPixmaps[OnlineStateIndicator::StateOffline]);
	if (this->m_tray && this->m_animateTray)
		this->m_tray->setIcon(blinkState ? this->m_trayIcons[OnlineStateIndicator::StateConnecting] : this->m_trayIcons[OnlineStateIndicator::StateOffline]);
	blinkState = !blinkState;
}

void OnlineStateIndicator::on_trayBlinkTimer_timeout()
{
	if (QApplication::activeWindow() == this->window())
		this->stopAnimation();
	this->m_tray->setIcon(trayBlinkState ? QIcon(":/gui/icons/new-message.png") : this->m_trayIcons[this->m_state] );
	trayBlinkState = !trayBlinkState;
}

void OnlineStateIndicator::setState(OnlineStateIndicator::OnlineState state)
{
	this->m_state = state;
	if (state != OnlineStateIndicator::StateConnecting)
		this->blinkTimer->stop();
	switch (state) {
		case OnlineStateIndicator::StateOffline:
			this->ui->icon->setToolTip(tr("Offline"));
			break;
		case OnlineStateIndicator::StateConnecting:
			this->blinkTimer->start();
			this->ui->icon->setToolTip(tr("Connecting..."));
			break;
		case OnlineStateIndicator::StateOnline:
			this->ui->icon->setToolTip(tr("Online"));
			break;
		case OnlineStateIndicator::StateAway:
			this->ui->icon->setToolTip(tr("Away"));
			break;
		case OnlineStateIndicator::StateBusy:
			this->ui->icon->setToolTip(tr("Busy"));
			break;
		case OnlineStateIndicator::StateInvisible:
			this->ui->icon->setToolTip(tr("Invisible"));
			break;
	}
	this->ui->icon->setPixmap(this->m_indicatorPixmaps[this->m_state]);
	if (this->m_tray) {
		this->m_tray->setIcon(this->m_trayIcons[this->m_state]);
		this->m_tray->setToolTip(tr("PyGoWave Desktop Client - %1").arg(this->ui->icon->toolTip()));
	}
}

OnlineStateIndicator::OnlineState OnlineStateIndicator::state()
{
	return this->m_state;
}

void OnlineStateIndicator::updateTraySettings()
{
	QSettings settings;

	if (settings.value("Tray", true).toBool()) {
		this->ui->actHide->setEnabled(true);
		if (!this->m_tray) {
			this->m_tray = new QSystemTrayIcon(this->m_trayIcons[this->m_state], this);
			connect(this->m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(tray_activated(QSystemTrayIcon::ActivationReason)));
			connect(this->m_tray, SIGNAL(messageClicked()), this, SLOT(tray_messageClicked()));
			this->m_tray->setToolTip(tr("PyGoWave Desktop Client - %1").arg(this->ui->icon->toolTip()));
			this->m_tray->setContextMenu(this->m_actionsMenu);
			this->m_tray->setVisible(true);
		}
	}
	else {
		this->ui->actHide->setEnabled(false);
		if (this->m_tray) {
			delete this->m_tray;
			this->m_tray = NULL;
		}
	}

	this->m_animateTray = settings.value("Events/AnimateTray", true).toBool();
	this->m_showPopups = settings.value("Events/ShowPopups", true).toBool();
}

void OnlineStateIndicator::on_actOnline_triggered()
{
	if (this->m_state != OnlineStateIndicator::StateOnline)
		emit stateChangeRequested((int) OnlineStateIndicator::StateOnline);
}

void OnlineStateIndicator::on_actAway_triggered()
{
	if (this->m_state != OnlineStateIndicator::StateAway)
		emit stateChangeRequested((int) OnlineStateIndicator::StateAway);
}

void OnlineStateIndicator::on_actBusy_triggered()
{
	if (this->m_state != OnlineStateIndicator::StateAway)
		emit stateChangeRequested((int) OnlineStateIndicator::StateAway);
}

void OnlineStateIndicator::on_actInvisible_triggered()
{
	if (this->m_state != OnlineStateIndicator::StateInvisible)
		emit stateChangeRequested((int) OnlineStateIndicator::StateInvisible);
}

void OnlineStateIndicator::on_actOffline_triggered()
{
	if (this->m_state != OnlineStateIndicator::StateOffline)
		emit stateChangeRequested((int) OnlineStateIndicator::StateOffline);
}

void OnlineStateIndicator::tray_activated(QSystemTrayIcon::ActivationReason reason)
{
#ifndef Q_OS_MAC
	if (reason == QSystemTrayIcon::Trigger) {
		if (this->window()->isHidden() || QApplication::activeWindow() == this->window())
			this->on_actHide_triggered();
		else {
			this->window()->activateWindow();
			this->window()->raise();
		}
	}
#else
	Q_UNUSED(reason)
#endif
	this->stopAnimation();
}

void OnlineStateIndicator::tray_messageClicked()
{
	if (this->window()->isHidden())
		this->on_actHide_triggered();
	else {
		this->window()->activateWindow();
		this->window()->raise();
	}
	this->stopAnimation();
}

void OnlineStateIndicator::on_actHide_triggered()
{
	if (this->window()->isHidden()) {
		this->window()->show();
		this->ui->actHide->setText(tr("Hide"));
	}
	else {
		this->window()->hide();
		this->ui->actHide->setText(tr("Show"));
	}
}

void OnlineStateIndicator::showNotification(const QString &title, const QString &msg, int icon)
{
	if (icon < 0 || icon > 3)
		icon = 0;
	if (this->m_tray && this->m_showPopups)
		this->m_tray->showMessage(title, msg, (QSystemTrayIcon::MessageIcon) icon);
	if (this->m_animateTray)
		this->trayBlinkTimer->start();
}

void OnlineStateIndicator::stopAnimation()
{
	this->trayBlinkTimer->stop();
	if (this->m_tray)
		this->m_tray->setIcon(this->m_trayIcons[this->m_state]);
}
