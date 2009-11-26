/*
 * This file is part of JSWrapper, a C++ to JavaScript wrapper on WebKit
 *
 * Copyright (C) 2009 Patrick Schneider <patrick.p2k.schneider@googlemail.com>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; see the file
 * COPYING.LESSER.  If not, see <http://www.gnu.org/licenses/>.
 */

// Note: You must have MooTools installed on your web page for this to work.
//       Also, do not forget to load the library on the JavaScript side first.

#ifndef JSWRAPPER_H
#define JSWRAPPER_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

class QWebFrame;

class JSWrapper : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString objectId READ objectId)
	Q_PROPERTY(QObject* object READ object)

public:
	QObject * object() const;
	QString objectId() const;
	JSWrapper * parent() const;

protected:
	JSWrapper(QWebFrame * webFrame, QObject * object, const QString &objectId, JSWrapper * parent = 0);
	void forwardSignal(const QString &signalName, const QVariant &args = QVariant());

private:
	Q_DISABLE_COPY(JSWrapper)

	QString m_objectClassName;
	QObject * m_object;
	QString m_objectId;
	QWebFrame * m_webFrame;
	JSWrapper * m_parent;
};

class JSWrapperFactory : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QStringList classNameList READ classNameList)

public:
	JSWrapperFactory(QObject * parent = 0);
	virtual ~JSWrapperFactory();

	virtual QStringList classNameList() const = 0;

	void registerFactory(QWebFrame * webFrame);

	QString javaScriptObjectName() const;

protected:
	virtual JSWrapper * createWrapper(const QString &className, const QString &objectId, JSWrapper * parent) = 0;

public slots:
	QObject * createWrapper(const QString &className, const QString &objectId, QObject * parent);

private slots:
	void javaScriptWindowObjectCleared();

private:
	Q_DISABLE_COPY(JSWrapperFactory)

	QList<JSWrapper*> m_tlWrappers;
	QWebFrame * m_webFrame;
	QString m_jsObjName;

	static int g_factoryId;
};

#endif // JSWRAPPER_H
