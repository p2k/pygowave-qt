# -------------------------------------------------
# Project created by QtCreator 2009-09-04T09:11:55
# -------------------------------------------------
QT += network \
    webkit \
    xml
macx {
	LIBS += -framework PyGoWaveApi -framework JSWrapper
}
else:win32 {
	LIBS += -lpygowave_api0 -ljswrapper0
}
else {
	LIBS += -lpygowave_api -ljswrapper
}
LIBS += -lqjson
TARGET = PyGoWaveDesktopClient
TEMPLATE = app
DEPENDPATH += src
INCLUDEPATH += src
SOURCES += src/main.cpp \
    src/gui/mainwindow.cpp \
    src/model_qt.cpp \
    src/gui/connectdialog.cpp \
    src/gui/onlinestateindicator.cpp \
    src/avatarloader.cpp \
    src/gui/participantwidget.cpp \
    src/gui/waveletwidget.cpp \
    src/gui/addparticipantwindow.cpp \
    src/js_api.cpp \
    src/gui/preferencesdialog.cpp
HEADERS += src/gui/mainwindow.h \
    src/model_qt.h \
    src/gui/connectdialog.h \
    src/gui/onlinestateindicator.h \
    src/avatarloader.h \
    src/gui/participantwidget.h \
    src/gui/waveletwidget.h \
    src/gui/addparticipantwindow.h \
    src/js_api.h \
    src/gui/preferencesdialog.h
FORMS += src/gui/mainwindow.ui \
    src/gui/connectdialog.ui \
    src/gui/onlinestateindicator.ui \
    src/gui/participantwidget.ui \
    src/gui/waveletwidget.ui \
    src/gui/addparticipantwindow.ui \
    src/gui/preferencesdialog.ui
RESOURCES += src/main.qrc
VERSION = 0.2.0
target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
TRANSLATIONS += src/tr/pygowave_de.ts
win32:RC_FILE = src/gui/windows_icon.rc
macx:ICON = src/gui/icons/PyGoWave.icns
