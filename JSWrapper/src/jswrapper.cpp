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

#include "jswrapper.h"
#include "jswrapper_p.h"

#include <qjson/serializer.h>

#include <QtCore/QFile>
#include <QtWebKit/QWebFrame>

int JSWrapperFactoryPrivate::g_factoryId = 0;

JSWrapper::JSWrapper(QWebFrame * webFrame, QObject * object, const QString &objectId, JSWrapper * parent) : QObject(parent), d(new JSWrapperPrivate)
{
	d->m_webFrame = webFrame;
	d->m_object = object;
	d->m_objectClassName = object->metaObject()->className();
	d->m_parent = parent;
	this->setObjectId(objectId);
}

QObject * JSWrapper::object() const
{
	return d->m_object;
}

QString JSWrapper::objectId() const
{
	return d->m_objectId;
}

QString JSWrapper::objectGuid() const
{
	return d->m_objectGuid;
}

void JSWrapper::setObjectId(const QString &objectId)
{
	QString oldGuid = d->m_objectGuid;

	d->m_objectId = objectId;
	if (d->m_parent)
		d->m_objectGuid = QString("%1@%2").arg(objectId).arg(d->m_parent->objectGuid());
	else
		d->m_objectGuid = objectId;

	if (!oldGuid.isEmpty())
		d->m_webFrame->evaluateJavaScript(QString("jswrapper.objects.changeObjectGuid('%1', '%2', '%3');").arg(d->m_objectClassName, oldGuid, d->m_objectGuid));

	foreach (JSWrapper * child, this->findChildren<JSWrapper*>())
		child->setObjectId(child->objectId());
}

JSWrapper * JSWrapper::parent() const
{
	return d->m_parent;
}

void JSWrapper::forwardSignal(const QString &signalName, const QVariant &args)
{
	QJson::Serializer jserializer;
	QString js;
	if (!args.isValid()) {
		js = QString("jswrapper.objects.forwardSignal('%1', '%2', '%3');").arg(d->m_objectClassName, d->m_objectGuid, signalName);
		///qDebug("JSWrapper::forwardSignal: Forwarding signal '%s' for %s '%s'", qPrintable(signalName), qPrintable(d->m_objectClassName), qPrintable(d->m_objectGuid));
	}
	else {
		QString s_args = QString::fromUtf8(jserializer.serialize(args));
		js = QString("jswrapper.objects.forwardSignal('%1', '%2', '%3', %4);").arg(d->m_objectClassName, d->m_objectGuid, signalName, s_args);
		///qDebug("JSWrapper::forwardSignal: Forwarding signal '%s' for %s '%s' with args: %s", qPrintable(signalName), qPrintable(d->m_objectClassName), qPrintable(d->m_objectGuid), qPrintable(s_args));
	}
	d->m_webFrame->evaluateJavaScript(js);
}


JSWrapperFactory::JSWrapperFactory(QObject * parent) : QObject(parent), d(new JSWrapperFactoryPrivate)
{
	d->m_webFrame = NULL;
}

JSWrapperFactory::~JSWrapperFactory()
{
	this->javaScriptWindowObjectCleared();
}

void JSWrapperFactory::registerFactory(QWebFrame * webFrame)
{
	if (d->m_webFrame) {
		qWarning("JSWrapperFactory: Cannot register factory to more than one web frame.");
		return;
	}
	d->m_webFrame = webFrame;
	connect(webFrame, SIGNAL(javaScriptWindowObjectCleared()), SLOT(javaScriptWindowObjectCleared()));
	d->m_jsObjName = QString("__JSWrapperFactory%1").arg(JSWrapperFactoryPrivate::g_factoryId++);
	webFrame->addToJavaScriptWindowObject(d->m_jsObjName, this);
	webFrame->evaluateJavaScript(QString("jswrapper.factory.registerFactory(%1);").arg(d->m_jsObjName));
}

QString JSWrapperFactory::javaScriptObjectName() const
{
	return d->m_jsObjName;
}

QObject * JSWrapperFactory::createWrapper(const QString &className, const QString &objectId, QObject * parent)
{
	JSWrapper * wrapper = this->createWrapper(className, objectId, qobject_cast<JSWrapper*>(parent));
	if (!wrapper)
		return NULL;
	if (wrapper->parent() == NULL) // Top level wrappers
		d->m_tlWrappers.append(wrapper);
	return wrapper;
}

void JSWrapperFactory::javaScriptWindowObjectCleared()
{
	while (!d->m_tlWrappers.isEmpty()) // Delete top level wrappers
		delete d->m_tlWrappers.takeLast();
	d->m_webFrame = NULL;
}
