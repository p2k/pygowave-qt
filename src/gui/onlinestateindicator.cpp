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
	this->ui->icon->setContextMenuPolicy(Qt::ActionsContextMenu);

	this->m_currentIcon = QPixmap(":/gui/icons/im-user-offline.png");
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
	this->ui->icon->setPixmap(blinkState ? QPixmap(":/gui/icons/im-user.png") : QPixmap(":/gui/icons/im-user-offline.png"));
	if (this->m_tray && this->m_animateTray)
		this->m_tray->setIcon(QIcon(*this->ui->icon->pixmap()));
	blinkState = !blinkState;
}

void OnlineStateIndicator::on_trayBlinkTimer_timeout()
{
	if (QApplication::activeWindow() == this->window())
		this->stopAnimation();
	this->m_tray->setIcon(QIcon(trayBlinkState ? QPixmap(":/gui/icons/new-message.png") : this->m_currentIcon ));
	trayBlinkState = !trayBlinkState;
}

void OnlineStateIndicator::setState(OnlineStateIndicator::OnlineState state)
{
	this->m_state = state;
	if (state != OnlineStateIndicator::StateConnecting)
		this->blinkTimer->stop();
	switch (state) {
		case OnlineStateIndicator::StateOffline:
			this->m_currentIcon = QPixmap(":/gui/icons/im-user-offline.png");
			this->ui->icon->setToolTip(tr("Offline"));
			break;
		case OnlineStateIndicator::StateConnecting:
			this->m_currentIcon = QPixmap(":/gui/icons/im-user-offline.png");
			this->blinkTimer->start();
			this->ui->icon->setToolTip(tr("Connecting..."));
			break;
		case OnlineStateIndicator::StateOnline:
			this->m_currentIcon = QPixmap(":/gui/icons/im-user.png");
			this->ui->icon->setToolTip(tr("Online"));
			break;
		case OnlineStateIndicator::StateAway:
			this->m_currentIcon = QPixmap(":/gui/icons/im-user-away.png");
			this->ui->icon->setToolTip(tr("Away"));
			break;
		case OnlineStateIndicator::StateBusy:
			this->m_currentIcon = QPixmap(":/gui/icons/im-user-busy.png");
			this->ui->icon->setToolTip(tr("Busy"));
			break;
		case OnlineStateIndicator::StateInvisible:
			this->m_currentIcon = QPixmap(":/gui/icons/im-invisible-user.png");
			this->ui->icon->setToolTip(tr("Invisible"));
			break;
	}
	this->ui->icon->setPixmap(this->m_currentIcon);
	if (this->m_tray) {
		this->m_tray->setIcon(QIcon(this->m_currentIcon));
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
		if (!this->m_tray) {
			this->m_tray = new QSystemTrayIcon(QIcon(this->m_currentIcon), this);
			connect(this->m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(tray_activated(QSystemTrayIcon::ActivationReason)));
			this->m_tray->setToolTip(tr("PyGoWave Desktop Client - %1").arg(this->ui->icon->toolTip()));
			this->m_tray->setContextMenu(this->m_actionsMenu);
			this->m_tray->setVisible(true);
		}
	}
	else if (this->m_tray) {
		delete this->m_tray;
		this->m_tray = NULL;
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
	if (reason == QSystemTrayIcon::Trigger) {
		if (this->window()->isHidden())
			this->window()->show();
		else if (QApplication::activeWindow() == this->window())
			this->window()->hide();
		else {
			this->window()->activateWindow();
			this->window()->raise();
		}
	}
	this->stopAnimation();
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
		this->m_tray->setIcon(QIcon(this->m_currentIcon));
}
