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

#ifndef ADDPARTICIPANTWINDOW_H
#define ADDPARTICIPANTWINDOW_H

#include <QtGui/QDialog>

namespace PyGoWave
{
	class Controller;
}

namespace Ui {
    class AddParticipantWindow;
}

class QModelIndex;
class PyGoWaveParticipantsList;

class AddParticipantWindow : public QDialog
{
    Q_OBJECT

public:
	AddParticipantWindow(PyGoWave::Controller * controller, QWidget *parent = 0);
    ~AddParticipantWindow();

signals:
	void participantSelected(const QByteArray &id);

private slots:
	void on_searchResultsList_doubleClicked(const QModelIndex &index);
	void on_searchBox_textChanged(const QString &text);
	void controller_participantSearchResults(int searchId, const QList<QByteArray> &);
	void controller_participantSearchResultsInvalid(int searchId, int minimumLetters);

protected:
    void changeEvent(QEvent *e);
	void focusInEvent(QFocusEvent *e);

private:
	Ui::AddParticipantWindow * ui;
	PyGoWave::Controller * m_controller;
	PyGoWaveParticipantsList * m_model;
	int m_searchId;
};

#endif // ADDPARTICIPANTWINDOW_H
