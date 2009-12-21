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

#include "avatarloader.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtGui/QPainter>

AvatarLoader * AvatarLoader::g_instance = NULL;

AvatarLoader * AvatarLoader::instance()
{
	if (AvatarLoader::g_instance == NULL)
		AvatarLoader::g_instance = new AvatarLoader();
	return AvatarLoader::g_instance;
}

AvatarLoader::AvatarLoader() : m_defaultAvatar(":/gui/gfx/default-avatar.png")
{
	this->mgr = new QNetworkAccessManager(this);
	this->mgr->setObjectName("mgr");
	QMetaObject::connectSlotsByName(this);
}

QPixmap AvatarLoader::get(const QString & url)
{
	if (this->m_avatarCache.contains(url))
		return this->m_avatarCache[url];
	else
		this->queueRequest(url);
	return QPixmap();
}

void AvatarLoader::queueRequest(const QString &url)
{
	if (this->m_requestQueue.contains(url))
		return;
	this->m_avatarCache[url] = QPixmap();
	this->m_requestQueue.append(url);
	if (this->m_requestQueue.size() == 1)
		this->doNextRequest();
}

void AvatarLoader::doNextRequest()
{
	if (this->m_requestQueue.isEmpty())
		return;
	QString url = this->m_requestQueue.first();
	QNetworkRequest request;
	request.setUrl(QUrl(url));
	request.setRawHeader("User-Agent", "PyGoWaveDesktopClient/0.1");
	this->mgr->get(request);
}

QPixmap AvatarLoader::getPrepared(const QString & url, const QSize &size)
{
	QPixmap pix = this->get(url);
	if (pix.isNull())
		return pix;
	QImage img(size, QImage::Format_RGB32);
	QPainter painter(&img);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.setPen(QPen(QColor(184, 198, 217)));
	painter.setBrush(QBrush(Qt::white));
	painter.drawRect(0, 0, size.width()-1, size.height()-1);
	painter.drawPixmap(1, 1, size.width()-2, size.height()-2, pix.scaled(size.width()-2, size.height()-2));
	return QPixmap::fromImage(img);
}

QPixmap AvatarLoader::defaultAvatar()
{
	return this->m_defaultAvatar;
}

QPixmap AvatarLoader::defaultAvatarPrepared(const QSize &size)
{
	QPixmap pix = this->defaultAvatar();
	QImage img(size, QImage::Format_RGB32);
	QPainter painter(&img);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.setPen(QPen(QColor(184, 198, 217)));
	painter.setBrush(QBrush(Qt::white));
	painter.drawRect(0, 0, size.width()-1, size.height()-1);
	painter.drawPixmap(1, 1, size.width()-2, size.height()-2, pix.scaled(size.width()-2, size.height()-2));
	return QPixmap::fromImage(img);
}

void AvatarLoader::on_mgr_finished(QNetworkReply * reply)
{
	QString url = this->m_requestQueue.takeFirst();
	bool success = false;
	reply->deleteLater();
	if (reply->error() == QNetworkReply::NoError) {
		QString dataType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
		if (dataType.startsWith("image/")) {
			QPixmap pix;
			pix.loadFromData(reply->readAll());
			if (!pix.isNull()) {
				this->m_avatarCache[url] = pix;
				success = true;
			}
		}
	}
	emit avatarReady(url, success);
	this->doNextRequest();
}
