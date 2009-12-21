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

#ifndef ADDGADGETWINDOW_H
#define ADDGADGETWINDOW_H

#include <QtGui/QDialog>

namespace PyGoWave
{
	class Controller;
}

namespace Ui {
	class AddGadgetWindow;
}

class AddGadgetWindow : public QDialog
{
	Q_OBJECT
public:
	AddGadgetWindow(PyGoWave::Controller * controller, QWidget * parent = 0);
	~AddGadgetWindow();

protected:
	void changeEvent(QEvent *e);

signals:
	void gadgetSelected(const QString &url);

private:
	Ui::AddGadgetWindow * ui;
	PyGoWave::Controller * m_controller;
	QList< QHash<QString,QString> > m_gadgetList;

private slots:
	void on_buttonBox_accepted();
	void on_cmbGadgets_currentIndexChanged(int index);
	void on_cmdReload_clicked();

	void updateGadgetList(const QList< QHash<QString,QString> > &gadgetList);
};

#endif // ADDGADGETWINDOW_H
