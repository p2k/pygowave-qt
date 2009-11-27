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
SOURCES += src/jswrapper/jswrapper.cpp
HEADERS += src/jswrapper/jswrapper.h \
	src/jswrapper/jswrapper_p.h

target.path = $$[QT_INSTALL_LIBS]
dist_headers.path = $$[QT_INSTALL_HEADERS]/jswrapper
dist_headers.files = src/jswrapper/jswrapper.h
js_library.path = $$[QT_INSTALL_DATA]/jswrapper
js_library.path = src/jswrapper/jswrapper.js

VERSION = 0.2.0
INSTALLS += target dist_headers js_library
