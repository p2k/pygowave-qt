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

#include <qjson/serializer.h>

#include <QtCore/QFile>
#include <QtWebKit/QWebFrame>

int JSWrapperFactory::g_factoryId = 0;

JSWrapper::JSWrapper(QWebFrame * webFrame, QObject * object, const QString &objectId, JSWrapper * parent) : QObject(parent)
{
	this->m_webFrame = webFrame;
	this->m_object = object;
	this->m_objectClassName = object->metaObject()->className();
	this->m_parent = parent;
	if (parent)
		this->m_objectId = QString("%1@%2").arg(objectId).arg(parent->objectId());
	else
		this->m_objectId = objectId;
}

QObject * JSWrapper::object() const
{
	return this->m_object;
}

QString JSWrapper::objectId() const
{
	return this->m_objectId;
}

JSWrapper * JSWrapper::parent() const
{
	return this->m_parent;
}

void JSWrapper::forwardSignal(const QString &signalName, const QVariant &args)
{
	QJson::Serializer jserializer;
	QString js;
	if (!args.isValid()) {
		js = QString("jswrapper.objects.forwardSignal('%1', '%2', '%3');").arg(this->m_objectClassName, this->objectId(), signalName);
		///qDebug("JSWrapper::forwardSignal: Forwarding signal '%s' for %s '%s'", qPrintable(signalName), qPrintable(this->m_objectClassName), qPrintable(this->objectId()));
	}
	else {
		QString s_args = QString::fromUtf8(jserializer.serialize(args));
		js = QString("jswrapper.objects.forwardSignal('%1', '%2', '%3', %4);").arg(this->m_objectClassName, this->objectId(), signalName, s_args);
		///qDebug("JSWrapper::forwardSignal: Forwarding signal '%s' for %s '%s' with args: %s", qPrintable(signalName), qPrintable(this->m_objectClassName), qPrintable(this->objectId()), qPrintable(s_args));
	}
	this->m_webFrame->evaluateJavaScript(js);
}


JSWrapperFactory::JSWrapperFactory(QObject * parent) : QObject(parent)
{
	this->m_webFrame = NULL;
}

JSWrapperFactory::~JSWrapperFactory()
{
	this->javaScriptWindowObjectCleared();
}

void JSWrapperFactory::registerFactory(QWebFrame * webFrame)
{
	if (this->m_webFrame) {
		qWarning("JSWrapperFactory: Cannot register factory to more than one web frame.");
		return;
	}
	this->m_webFrame = webFrame;
	connect(webFrame, SIGNAL(javaScriptWindowObjectCleared()), SLOT(javaScriptWindowObjectCleared()));
	this->m_jsObjName = QString("__JSWrapperFactory%1").arg(JSWrapperFactory::g_factoryId++);
	webFrame->addToJavaScriptWindowObject(this->m_jsObjName, this);
	webFrame->evaluateJavaScript(QString("jswrapper.factory.registerFactory(%1);").arg(this->m_jsObjName));
}

QString JSWrapperFactory::javaScriptObjectName() const
{
	return this->m_jsObjName;
}

QObject * JSWrapperFactory::createWrapper(const QString &className, const QString &objectId, QObject * parent)
{
	JSWrapper * wrapper = this->createWrapper(className, objectId, qobject_cast<JSWrapper*>(parent));
	if (!wrapper)
		return NULL;
	if (wrapper->parent() == NULL) // Top level wrappers
		this->m_tlWrappers.append(wrapper);
	return wrapper;
}

void JSWrapperFactory::javaScriptWindowObjectCleared()
{
	while (!this->m_tlWrappers.isEmpty()) // Delete top level wrappers
		delete this->m_tlWrappers.takeLast();
	this->m_webFrame = NULL;
}
