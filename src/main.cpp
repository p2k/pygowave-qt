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

#include <QtGui/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include "gui/mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	app.setOrganizationDomain("pygowave.org");
	app.setOrganizationName("PyGoWave");
	app.setApplicationName("PyGoWaveDesktopClient");
	app.setApplicationVersion("0.1");

	QTranslator qtTranslator;
	if (!qtTranslator.load(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + QLocale::system().name()))
		qtTranslator.load("qt_" + QLocale::system().name());
	if (!qtTranslator.isEmpty())
		app.installTranslator(&qtTranslator);

	QTranslator pygoTranslator;
	if (pygoTranslator.load(":/tr/pygowave_" + QLocale::system().name()))
		app.installTranslator(&pygoTranslator);

    MainWindow w;
	return app.exec();
}
