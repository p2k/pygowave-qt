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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QtCore/QSettings>
#include <QtGui/QPushButton>

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog)
{
	this->ui->setupUi(this);

	this->ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	QSettings settings;

	this->ui->rbSDI->setChecked(settings.value("UseTabs", false).toBool());
	this->ui->chkTray->setChecked(settings.value("Tray", true).toBool());
	this->ui->chkStartHidden->setChecked(settings.value("StartHidden", false).toBool());
	this->ui->cmbInitialStatus->setCurrentIndex(settings.value("InitialStatus", 0).toInt());

	settings.beginGroup("Connection");
	this->ui->txtHostname->setText(settings.value("Hostname", "pygowave.net").toString());
	this->ui->spbPort->setValue(settings.value("Port", 61613).toInt());
	this->ui->txtUsername->setText(settings.value("Username", "").toString());
	int pwLength = QString::fromUtf8(QByteArray::fromBase64(settings.value("Password", "").toByteArray())).length();
	if (pwLength > 0) {
		this->ui->chkSavePassword->setChecked(true);
		this->ui->txtPassword->setText(QString(pwLength, '*')); // Minimal password protection
	}
	settings.endGroup();

	settings.beginGroup("Events");
	this->ui->chkAnimateTray->setChecked(settings.value("AnimateTray", true).toBool());
	this->ui->chkPopups->setChecked(settings.value("ShowPopups", true).toBool());
	settings.endGroup();

	this->m_passwordChanged = false;
	connect(this->ui->chkTray, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
	connect(this->ui->chkStartHidden, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
	connect(this->ui->cmbInitialStatus, SIGNAL(currentIndexChanged(int)), this, SLOT(somethingChanged()));
	connect(this->ui->txtHostname, SIGNAL(textChanged(QString)), this, SLOT(somethingChanged()));
	connect(this->ui->spbPort, SIGNAL(valueChanged(int)), this, SLOT(somethingChanged()));
	connect(this->ui->txtUsername, SIGNAL(textChanged(QString)), this, SLOT(somethingChanged()));
	connect(this->ui->txtPassword, SIGNAL(textChanged(QString)), this, SLOT(somethingChanged()));
	connect(this->ui->chkAnimateTray, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
	connect(this->ui->chkPopups, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
	connect(this->ui->rbMDI, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
	connect(this->ui->rbSDI, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
}

void PreferencesDialog::somethingChanged()
{
	this->ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

PreferencesDialog::~PreferencesDialog()
{
	delete this->ui;
}

void PreferencesDialog::changeEvent(QEvent *e)
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

void PreferencesDialog::on_buttonBox_clicked(QAbstractButton* button)
{
	if (button == this->ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
		this->ui->rbMDI->setChecked(true);

		this->ui->chkTray->setChecked(true);
		this->ui->chkStartHidden->setChecked(false);
		this->ui->cmbInitialStatus->setCurrentIndex(0);

		this->ui->txtHostname->setText("pygowave.net");
		this->ui->spbPort->setValue(61613);
		this->ui->txtUsername->setText("");

		this->ui->chkSavePassword->setEnabled(false);
		this->ui->txtPassword->setText("");

		this->ui->chkAnimateTray->setChecked(true);
		this->ui->chkPopups->setChecked(true);
	}
	if (button == this->ui->buttonBox->button(QDialogButtonBox::Apply) || button == this->ui->buttonBox->button(QDialogButtonBox::Ok)) {
		QSettings settings;

		settings.setValue("UseTabs", this->ui->rbSDI->isChecked());

		settings.setValue("Tray", this->ui->chkTray->isChecked());
		settings.setValue("StartHidden", this->ui->chkStartHidden->isChecked());
		settings.setValue("InitialStatus", this->ui->cmbInitialStatus->currentIndex());

		settings.beginGroup("Connection");
		settings.setValue("Hostname", this->ui->txtHostname->text());
		settings.setValue("Port", this->ui->spbPort->value());
		settings.setValue("Username", this->ui->txtUsername->text());
		if (!this->ui->chkSavePassword->isChecked())
			settings.remove("Password");
		else if (this->m_passwordChanged)
			settings.setValue("Password", this->ui->txtPassword->text().toUtf8().toBase64());
		settings.endGroup();

		settings.beginGroup("Events");
		settings.setValue("AnimateTray", this->ui->chkAnimateTray->isChecked());
		settings.setValue("ShowPopups", this->ui->chkPopups->isChecked());
		settings.endGroup();

		this->ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	}
}

void PreferencesDialog::on_txtPassword_textChanged(const QString &)
{
	this->m_passwordChanged = true;
}
