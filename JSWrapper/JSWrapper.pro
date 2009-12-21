#
# This file is part of JSWrapper, a C++ to JavaScript wrapper on WebKit
#
# Copyright (C) 2009 Patrick Schneider <patrick.p2k.schneider@googlemail.com>
#
# This library is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this library; see the file
# COPYING.LESSER.  If not, see <http://www.gnu.org/licenses/>.
#

QT += webkit
TARGET = jswrapper
TEMPLATE = lib
DEFINES += JSWRAPPER_LIBRARY
DEPENDPATH += src
INCLUDEPATH += src
LIBS += -lqjson
SOURCES += src/jswrapper.cpp
HEADERS += src/jswrapper.h \
	src/jswrapper_p.h \
	src/jswrapper_global.h

target.path = $$[QT_INSTALL_LIBS]
dist_headers.path = $$[QT_INSTALL_HEADERS]/JSWrapper
dist_headers.files = src/jswrapper.h src/jswrapper_global.h
js_library.path = $$[QT_INSTALL_DATA]/JSWrapper
js_library.path = src/jswrapper.js

VERSION = 0.2.0
INSTALLS += target dist_headers js_library
macx {
	CONFIG += lib_bundle
	FRAMEWORK_HEADERS.version = Versions
	FRAMEWORK_HEADERS.files = src/jswrapper.h src/jswrapper_global.h
	FRAMEWORK_HEADERS.path = Headers
	FRAMEWORK_RESOURCES.version = Versions
	FRAMEWORK_RESOURCES.files = src/jswrapper.js
	FRAMEWORK_RESOURCES.path = Resources
	QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS FRAMEWORK_RESOURCES
	TARGET = JSWrapper
	INSTALLS -= dist_headers js_library
}

