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

#ifndef ONLINESTATEINDICATOR_H
#define ONLINESTATEINDICATOR_H

#include <QtGui/QWidget>
#include <QtGui/QSystemTrayIcon>

class QTimer;
class QMenu;

namespace Ui {
    class OnlineStateIndicator;
}

class OnlineStateIndicator : public QWidget
{
    Q_OBJECT

public:
	enum OnlineState {
		StateOffline,
		StateConnecting,
		StateOnline,
		StateAway,
		StateBusy,
		StateInvisible
	};

    OnlineStateIndicator(QWidget *parent = 0);
    ~OnlineStateIndicator();

	void setState(OnlineState state);
	OnlineState state();

	void updateTraySettings();

public slots:
	void showNotification(const QString &title, const QString &msg, int icon = 0);
	void stopAnimation();

signals:
	void stateChangeRequested(int);

protected:
    void changeEvent(QEvent *e);

private slots:
	void on_actOffline_triggered();
	void on_actInvisible_triggered();
	void on_actBusy_triggered();
	void on_actAway_triggered();
	void on_actOnline_triggered();
	void on_blinkTimer_timeout();
	void on_trayBlinkTimer_timeout();

	void tray_activated(QSystemTrayIcon::ActivationReason reason);

private:
	Ui::OnlineStateIndicator * ui;
	QTimer * blinkTimer;
	QTimer * trayBlinkTimer;
	bool blinkState;
	bool trayBlinkState;
	OnlineState m_state;
	QSystemTrayIcon * m_tray;
	QMenu * m_actionsMenu;
	QPixmap m_currentIcon;
	bool m_animateTray;
	bool m_showPopups;
};

#endif // ONLINESTATEINDICATOR_H
