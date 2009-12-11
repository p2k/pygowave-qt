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

#ifndef AVATARLOADER_H
#define AVATARLOADER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtGui/QPixmap>

class QNetworkAccessManager;
class QNetworkReply;

class AvatarLoader : public QObject
{
	Q_OBJECT

public:
	static AvatarLoader * instance();

	QPixmap get(const QString & url);
	QPixmap getPrepared(const QString & url, const QSize &size);

	QPixmap defaultAvatar();
	QPixmap defaultAvatarPrepared(const QSize &size);

signals:
	void avatarReady(const QString & url, bool valid);

private slots:
	void on_mgr_finished(QNetworkReply * reply);

private:
	static AvatarLoader * g_instance;
    AvatarLoader();

	QNetworkAccessManager * mgr;

	QMap<QString, QPixmap> m_avatarCache;
	QPixmap m_defaultAvatar;
};

#endif // AVATARLOADER_H
