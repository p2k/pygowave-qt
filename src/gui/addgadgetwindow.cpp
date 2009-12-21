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

#include "addgadgetwindow.h"
#include "ui_addgadgetwindow.h"

#include <PyGoWaveApi/controller.h>

#include <QtGui/QPushButton>

AddGadgetWindow::AddGadgetWindow(PyGoWave::Controller * controller, QWidget * parent) : QDialog(parent), ui(new Ui::AddGadgetWindow)
{
	this->m_controller = controller;
	this->setWindowFlags((this->windowFlags() & ~Qt::Dialog) | Qt::Tool);
	this->ui->setupUi(this);

	connect(this->m_controller, SIGNAL(updateGadgetList(QList< QHash<QString,QString> >)), this, SLOT(updateGadgetList(QList< QHash<QString,QString> >)));
	this->ui->cmbGadgets->addItem(tr("Loading..."));
	this->ui->cmbGadgets->setEnabled(false);
	this->m_controller->refreshGadgetList();
}

AddGadgetWindow::~AddGadgetWindow()
{
	delete this->ui;
}

void AddGadgetWindow::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		this->ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void AddGadgetWindow::on_cmdReload_clicked()
{
	this->m_gadgetList.clear();
	this->ui->cmbGadgets->clear();
	this->ui->cmbGadgets->addItem(tr("Loading..."));
	this->ui->cmbGadgets->setEnabled(false);
	this->m_controller->refreshGadgetList(true);
}

void AddGadgetWindow::updateGadgetList(const QList< QHash<QString,QString> > &gadgetList)
{
	this->ui->cmbGadgets->clear();
	this->m_gadgetList = gadgetList;
	QStringList names;
	QHash<QString,QString> entry;
	foreach (entry, gadgetList)
		names.append(entry["name"]);
	this->ui->cmbGadgets->addItems(names);
	this->ui->cmbGadgets->setEnabled(true);
}

void AddGadgetWindow::on_cmbGadgets_currentIndexChanged(int index)
{
	if (index >= this->m_gadgetList.size() || index < 0) {
		this->ui->lblDescription->setText(tr("-"));
		this->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
	else {
		QHash<QString,QString> entry = this->m_gadgetList[index];
		this->ui->lblDescription->setText(QString("%1\n\n%2").arg(entry["descr"], tr("Uploaded by %1").arg(entry["uploaded_by"])));
		this->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
}

void AddGadgetWindow::on_buttonBox_accepted()
{
	int index = this->ui->cmbGadgets->currentIndex();
	if (index < this->m_gadgetList.size() || index >= 0) {
		QHash<QString,QString> entry = this->m_gadgetList[index];
		emit gadgetSelected(entry["url"]);
	}
}
