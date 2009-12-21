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

#include "waveletnetworkaccessmanager.h"
#include "avatarloader.h"

#include <QtNetwork/QNetworkRequest>
#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>

WaveletNetworkAccessManager::WaveletNetworkAccessManager(const QString &hostName, QObject * parent) : QNetworkAccessManager(parent)
{
	this->m_hostName = hostName;
}

QNetworkReply * WaveletNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
{
	QUrl url = req.url();
	if (url.scheme() == "avatar")
		return new AvatarReply(url);
	else if (url.scheme() == "wavelet") {
		if (url.queryItemValue("rc") == "gadget")
			return new GadgetReply(this, this->m_hostName, url);
		else if (url.queryItemValue("rc") == "gadget-wrapper") {
			QUrl rcUrl("wavelet:load");
			rcUrl.setQueryItems(QList< QPair<QString, QString> >() << QPair<QString, QString>("rc", "js") << QPair<QString, QString>("name", "gadget-wrapper"));
			return new ResourceReply(rcUrl);
		}
		else
			return new ResourceReply(url);
	}
	else
		return QNetworkAccessManager::createRequest(op, req, outgoingData);
}


AvatarReply::AvatarReply(const QUrl &url, QObject * parent) : QNetworkReply(parent)
{
	this->m_offset = 0;
	this->setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
	AvatarLoader * ldr = AvatarLoader::instance();
	QPixmap pix;
	if (url.path() != "default") {
		this->m_url = QString("http://%1").arg(url.path());
		connect(ldr, SIGNAL(avatarReady(QString,bool)), this, SLOT(avatarReady(QString,bool)));
		pix = ldr->get(this->m_url);
	}
	else
		pix = ldr->defaultAvatar();
	if (!pix.isNull()) {
		QBuffer buf(&this->m_data);
		buf.open(QIODevice::WriteOnly);
		pix.save(&buf, "PNG");
		buf.close();
		this->setHeader(QNetworkRequest::ContentLengthHeader, this->m_data.size());
		this->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
		qApp->postEvent(this, new QEvent(QEvent::User));
	}
	this->open(QIODevice::ReadOnly);
}

void AvatarReply::abort()
{
}

qint64 AvatarReply::bytesAvailable() const
{
	return (this->m_data.size() - this->m_offset) + QNetworkReply::bytesAvailable();
}

bool AvatarReply::isSequential() const
{
	return true;
}

qint64 AvatarReply::readData(char * data, qint64 maxSize)
{
	if (this->m_offset < this->m_data.size()) {
		qint64 end = qMin(this->m_offset + maxSize, (qint64) this->m_data.size());
		qint64 size = end - this->m_offset;
		memcpy(data, this->m_data.constData() + this->m_offset, size);
		this->m_offset += size;
		return size;
	}
	return 0;
}

void AvatarReply::avatarReady(const QString & url, bool valid)
{
	if (url == this->m_url) {
		if (valid) {
			QBuffer buf(&this->m_data);
			buf.open(QIODevice::WriteOnly);
			AvatarLoader::instance()->get(this->m_url).save(&buf, "PNG");
			buf.close();
			this->setHeader(QNetworkRequest::ContentLengthHeader, this->m_data.size());
			this->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
		}
		else {
			this->setHeader(QNetworkRequest::ContentLengthHeader, 0);
			this->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);
		}
		emit readyRead();
		emit finished();
	}
}

bool AvatarReply::event(QEvent * e)
{
	if (e->type() == QEvent::User) {
		emit readyRead();
		emit finished();
		return true;
	}
	else
		return QNetworkReply::event(e);
}


ResourceReply::ResourceReply(const QUrl &url, QObject * parent) : QNetworkReply(parent)
{
	static QStringList types = QStringList() << "main" << "css" << "js" << "gfx" << "icons";
	static QStringList exts = QStringList() << "html" << "css" << "js" << "png" << "png";
	static QStringList mimeTypes = QStringList() << "application/xhtml+xml" << "text/css" << "application/x-javascript" << "image/png" << "image/png";

	int typeId = types.indexOf(url.queryItemValue("rc"));
	if (typeId != -1) {
		this->setHeader(QNetworkRequest::ContentTypeHeader, mimeTypes.at(typeId));
		if (typeId == 0)
			this->m_file.setFileName(QString(":/gui/html/wave.html"));
		else
			this->m_file.setFileName(QString(":/gui/%1/%2.%3").arg(types.at(typeId), url.queryItemValue("name"), exts.at(typeId)));
	}

	if (this->m_file.exists()) {
		this->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
		this->setHeader(QNetworkRequest::ContentLengthHeader, this->m_file.size());
		this->m_file.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
	}
	else
		this->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);

	qApp->postEvent(this, new QEvent(QEvent::User));
	this->open(QIODevice::ReadOnly);
}

void ResourceReply::abort()
{
}

qint64 ResourceReply::bytesAvailable() const
{
	return this->m_file.bytesAvailable() + QNetworkReply::bytesAvailable();
}

bool ResourceReply::isSequential() const
{
	return true;
}

qint64 ResourceReply::readData(char * data, qint64 maxSize)
{
	QByteArray rdata = this->m_file.read(maxSize);
	memcpy(data, rdata.constData(), rdata.size());
	return rdata.size();
}

bool ResourceReply::event(QEvent * e)
{
	if (e->type() == QEvent::User) {
		if (this->m_file.exists())
			emit readyRead();
		emit finished();
		return true;
	}
	else
		return QNetworkReply::event(e);
}


GadgetReply::GadgetReply(QNetworkAccessManager * manager, const QString &hostName, const QUrl &url, QObject * parent) : QNetworkReply(parent)
{
	this->m_url.clear();
	this->m_url.setScheme("http");
	this->m_url.setHost(hostName);
	this->m_url.setPath("/gadgets/load/");
	QList<QPair<QString, QString> > query = url.queryItems();
	query.append(QPair<QString,QString>("wrapper_script", "wavelet:load?rc=gadget-wrapper"));
	query.append(QPair<QString,QString>("wave", "1"));
	this->m_url.setQueryItems(query);
	this->m_url.removeQueryItem("rc");
	qDebug("GadgetReply: Loading %s", this->m_url.toEncoded().constData());
	this->setHeader(QNetworkRequest::ContentTypeHeader, "application/xhtml+xml");
	QNetworkRequest request;
	request.setUrl(this->m_url);
	request.setRawHeader("User-Agent", "PyGoWaveDesktopClient/0.1");
	this->m_wrapped = manager->get(request);
	connect(this->m_wrapped, SIGNAL(metaDataChanged()), this, SIGNAL(metaDataChanged()));
	connect(this->m_wrapped, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
	connect(this->m_wrapped, SIGNAL(finished()), this, SIGNAL(finished()));
	connect(this->m_wrapped, SIGNAL(downloadProgress(qint64,qint64)), this, SIGNAL(downloadProgress(qint64,qint64)));
	this->open(QIODevice::ReadOnly);
}

void GadgetReply::abort()
{
}

qint64 GadgetReply::bytesAvailable() const
{
	return QNetworkReply::bytesAvailable() + this->m_wrapped->bytesAvailable();
}

bool GadgetReply::isSequential() const
{
	return true;
}

void GadgetReply::metaDataChanged()
{
	this->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, this->m_wrapped->attribute(QNetworkRequest::HttpStatusCodeAttribute));
	this->setHeader(QNetworkRequest::ContentLengthHeader, this->m_wrapped->header(QNetworkRequest::ContentLengthHeader));
}

qint64 GadgetReply::readData(char * data, qint64 maxSize)
{
	QByteArray rdata = this->m_wrapped->read(maxSize);
	memcpy(data, rdata.constData(), rdata.size());
	return rdata.size();
}
