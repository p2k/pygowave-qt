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

#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QtGui/QWidget>

namespace PyGoWave {
	class Participant;
}

namespace Ui {
    class ParticipantWidget;
}

class ParticipantWidget : public QWidget
{
    Q_OBJECT
public:
	enum DisplayStyle {
		StyleNormal,
		StyleSmall,
		StyleSearchbox
	};
	ParticipantWidget(PyGoWave::Participant * participant, DisplayStyle style = StyleNormal, QWidget *parent = 0);
    ~ParticipantWidget();

private slots:
	void onlineStateChanged(bool online);
	void dataChanged();
	void avatarReady(const QString &url, bool valid);

protected:
    void changeEvent(QEvent *e);

private:
	Ui::ParticipantWidget * ui;
	PyGoWave::Participant * participant;
};

#endif // PARTICIPANTWIDGET_H
