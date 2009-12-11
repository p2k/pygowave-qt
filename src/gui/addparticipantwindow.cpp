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

#include "addparticipantwindow.h"
#include "ui_addparticipantwindow.h"

#include "PyGoWaveApi/controller.h"
#include "model_qt.h"

AddParticipantWindow::AddParticipantWindow(PyGoWave::Controller * controller, QWidget *parent) : QDialog(parent), ui(new Ui::AddParticipantWindow)
{
	this->m_controller = controller;
	this->setWindowFlags((this->windowFlags() & ~Qt::Dialog) | Qt::Tool);
	this->ui->setupUi(this);
	this->m_model = new PyGoWaveParticipantsList(controller, this);
	this->ui->searchResultsList->setModel(this->m_model);
	connect(controller, SIGNAL(participantSearchResults(int,QList<QByteArray>)), this, SLOT(controller_participantSearchResults(int,QList<QByteArray>)));
	connect(controller, SIGNAL(participantSearchResultsInvalid(int,int)), this, SLOT(controller_participantSearchResultsInvalid(int,int)));
}

AddParticipantWindow::~AddParticipantWindow()
{
	delete this->ui;
}

void AddParticipantWindow::changeEvent(QEvent *e)
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

void AddParticipantWindow::controller_participantSearchResults(int searchId, const QList<QByteArray> &ids)
{
	if (searchId != this->m_searchId)
		return;
	this->ui->searchResultsStack->setCurrentIndex(0);
	this->m_model->sync(ids);
}

void AddParticipantWindow::controller_participantSearchResultsInvalid(int searchId, int minimumLetters)
{
	if (searchId != this->m_searchId)
		return;
	this->ui->searchResultsStack->setCurrentIndex(1);
	this->ui->message->setText(tr("Please enter at least %1 letters.").arg(minimumLetters));
}

void AddParticipantWindow::on_searchBox_textChanged(const QString &text)
{
	this->m_searchId = this->m_controller->searchForParticipant(text);
}

void AddParticipantWindow::focusInEvent(QFocusEvent *e)
{
	QDialog::focusInEvent(e);
	this->ui->searchBox->setFocus();
}

void AddParticipantWindow::on_searchResultsList_doubleClicked(const QModelIndex &index)
{
	emit participantSelected(index.data(Qt::UserRole).toByteArray());
}
