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

#ifndef WAVELETNETWORKACCESSMANAGER_H
#define WAVELETNETWORKACCESSMANAGER_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QFile>

class QNetworkRequest;

class WaveletNetworkAccessManager : public QNetworkAccessManager
{
public:
	WaveletNetworkAccessManager(const QString &hostName, QObject * parent = 0);

protected:
	QNetworkReply * createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

private:
	QString m_hostName;
};

class AvatarReply : public QNetworkReply
{
	Q_OBJECT
public:
	AvatarReply(const QUrl &url, QObject * parent = 0);

	void abort();
	qint64 bytesAvailable() const;
	bool isSequential() const;

protected:
	qint64 readData(char * data, qint64 maxSize);
	bool event(QEvent * e);

private slots:
	void avatarReady(const QString & url, bool valid);

private:
	QByteArray m_data;
	QString m_url;
	qint64 m_offset;
};

class ResourceReply : public QNetworkReply
{
	Q_OBJECT
public:
	ResourceReply(const QUrl &url, QObject * parent = 0);

	void abort();
	qint64 bytesAvailable() const;
	bool isSequential() const;

protected:
	qint64 readData(char * data, qint64 maxSize);
	bool event(QEvent * e);

private:
	QFile m_file;
};

class GadgetReply : public QNetworkReply
{
	Q_OBJECT
public:
	GadgetReply(QNetworkAccessManager * manager, const QString &hostName, const QUrl &url, QObject * parent = 0);

	void abort();
	qint64 bytesAvailable() const;
	bool isSequential() const;

protected:
	qint64 readData(char * data, qint64 maxSize);

private slots:
	void metaDataChanged();

private:
	QUrl m_url;
	QNetworkReply * m_wrapped;
};

#endif // WAVELETNETWORKACCESSMANAGER_H
