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

#include "participantwidget.h"
#include "ui_participantwidget.h"

#include "avatarloader.h"

#include "PyGoWaveApi/model.h"

ParticipantWidget::ParticipantWidget(PyGoWave::Participant * participant, ParticipantWidget::DisplayStyle style, QWidget *parent) : QWidget(parent), ui(new Ui::ParticipantWidget)
{
	this->ui->setupUi(this);
	this->participant = participant;
	connect(participant, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(participant, SIGNAL(onlineStateChanged(bool)), this, SLOT(onlineStateChanged(bool)));
	connect(AvatarLoader::instance(), SIGNAL(avatarReady(QString,bool)), this, SLOT(avatarReady(QString,bool)));
	switch (style) {
		case ParticipantWidget::StyleNormal:
			break;
		case ParticipantWidget::StyleSmall:
			this->setMinimumSize(32, 30);
			this->setMaximumSize(32, 30);
			this->resize(32, 30);
			this->ui->avatar->resize(29, 29);
			this->ui->indicator->setGeometry(23, 22, 8, 8);
			break;
		case ParticipantWidget::StyleSearchbox:
			this->setMinimumSize(0, 35);
			this->setMaximumSize(16777215, 35);
			this->resize(32, 35);
			break;
	}
	this->dataChanged();
}

ParticipantWidget::~ParticipantWidget()
{
	delete this->ui;
}

void ParticipantWidget::changeEvent(QEvent *e)
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

void ParticipantWidget::onlineStateChanged(bool online)
{
	this->ui->indicator->setVisible(online);
}

void ParticipantWidget::dataChanged()
{
	this->ui->avatar->setToolTip(this->participant->displayName());
	QPixmap avatar = AvatarLoader::instance()->get(this->participant->thumbnailUrl());
	if (!avatar.isNull())
		this->ui->avatar->setPixmap(avatar);
	this->onlineStateChanged(this->participant->isOnline());
}

void ParticipantWidget::avatarReady(const QString &url, bool valid)
{
	if (url != this->participant->thumbnailUrl())
		return;
	if (valid)
		this->ui->avatar->setPixmap(AvatarLoader::instance()->get(url));
	else
		this->ui->avatar->setPixmap(AvatarLoader::instance()->defaultAvatar());
}
