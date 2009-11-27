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

#ifndef JSWRAPPER_P_H
#define JSWRAPPER_P_H

#include <QtCore/QString>

class JSWrapperPrivate
{
public:
	QString m_objectClassName;
	QObject * m_object;
	QString m_objectId;
	QWebFrame * m_webFrame;
	JSWrapper * m_parent;
};

class JSWrapperFactoryPrivate
{
public:
	QList<JSWrapper*> m_tlWrappers;
	QWebFrame * m_webFrame;
	QString m_jsObjName;

	static int g_factoryId;
};

#endif // JSWRAPPER_P_H
