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
macx {
	LIBS += -framework QStomp
}
else:win32 {
	LIBS += -lqstomp0
}
else {
	LIBS += -lqstomp
}
LIBS += -lqjson
TARGET = pygowave_api
TEMPLATE = lib
DEFINES += PYGOWAVE_API_LIBRARY
DEPENDPATH += src
INCLUDEPATH += src
SOURCES += src/model.cpp \
    src/controller.cpp \
    src/operations.cpp
HEADERS += src/model.h \
	src/model_p.h \
    src/controller.h \
	src/operations.h \
	src/operations_p.h \
	src/controller_p.h \
	src/pygowave_api_global.h
target.path = $$[QT_INSTALL_LIBS]
dist_headers.path = $$[QT_INSTALL_HEADERS]/PyGoWaveApi
dist_headers.files = src/model.h \
    src/controller.h \
    src/operations.h \
	src/pygowave_api_global.h
VERSION = 0.3.0
INSTALLS += target \
    dist_headers
macx {
	CONFIG += lib_bundle
	FRAMEWORK_HEADERS.version = Versions
	FRAMEWORK_HEADERS.files = src/model.h src/controller.h src/operations.h src/pygowave_api_global.h
	FRAMEWORK_HEADERS.path = Headers
	QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
	TARGET = PyGoWaveApi
	INSTALLS -= dist_headers
}

