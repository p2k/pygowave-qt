#
# This file is part of the PyGoWave Qt/C++ Client API
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

QT += network
QT -= gui
LIBS += -lqstomp
TARGET = pygowave_api
TEMPLATE = lib
DEFINES += PYGOWAVE_API_LIBRARY
DEPENDPATH += src
INCLUDEPATH += src
SOURCES += src/core/model.cpp \
    src/core/controller.cpp \
    src/core/operations.cpp
HEADERS += src/core/model.h \
	src/core/model_p.h \
    src/core/controller.h \
	src/core/operations.h \
	src/core/operations_p.h \
	src/core/controller_p.h \
	src/core/pygowave_api_global.h
target.path = $$[QT_INSTALL_LIBS]
dist_headers.path = $$[QT_INSTALL_HEADERS]/pygowave_api
dist_headers.files = src/core/model.h \
    src/core/controller.h \
    src/core/operations.h
VERSION = 0.2.0
INSTALLS += target \
    dist_headers
