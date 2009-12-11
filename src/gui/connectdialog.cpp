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

#include "connectdialog.h"
#include "ui_connectdialog.h"

#include <QtCore/QSettings>
#include <QtGui/QMessageBox>

ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConnectDialog)
{
	this->ui->setupUi(this);
	QSettings settings;
	this->ui->txtUsername->setText(settings.value("Connection/Username", "").toString());
	if (this->ui->txtUsername->text().isEmpty())
		this->ui->txtUsername->setFocus();
}

ConnectDialog::~ConnectDialog()
{
	delete this->ui;
}

void ConnectDialog::changeEvent(QEvent *e)
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

QString ConnectDialog::username()
{
	return this->ui->txtUsername->text();
}

QString ConnectDialog::password()
{
	return this->ui->txtPassword->text();
}

void ConnectDialog::accept()
{
	if (this->ui->txtUsername->text().isEmpty()) {
		QMessageBox::warning(this, tr("No username"), tr("You must enter a username to connect!"));
		this->ui->txtUsername->setFocus();
		return;
	}

	QSettings settings;
	settings.beginGroup("Connection");
	settings.setValue("Username", this->ui->txtUsername->text());
	if (this->ui->chkSavePassword->isChecked())
		settings.setValue("Password", this->ui->txtPassword->text().toUtf8().toBase64()); // At least don't make it directly readable
	QDialog::accept();
}
