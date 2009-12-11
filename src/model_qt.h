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

#ifndef MODEL_QT_H
#define MODEL_QT_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QSet>
#include <QtGui/QPixmap>

namespace PyGoWave {
	class Participant;
	class WaveModel;
	class Controller;
}

class QSignalMapper;

class PyGoWaveListWatcher;

class PyGoWaveList : public QAbstractListModel
{
	Q_OBJECT

public:
	enum ItemDataRole {
		WaveIdRole = Qt::UserRole,
		RootWaveletIdRole,
		RootWaveletObjectRole
	};
	PyGoWaveList(PyGoWave::Controller * controller, QObject * parent = 0);
	~PyGoWaveList();

	int rowCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

public slots:
	void updateWave(const QByteArray &waveId);

private slots:
	void controller_waveAdded(const QByteArray &waveletId);
	void controller_waveAboutToBeRemoved(const QByteArray &waveletId);

private:
	QPixmap generateIcon(PyGoWave::WaveModel * wave);
	void updateRowIcon(int index);

	PyGoWave::Controller * m_controller;
	QList<QByteArray> m_waves_ordered;
	QList<QPixmap> m_icons;
	QList<PyGoWaveListWatcher*> m_watchers;
};

class PyGoWaveListWatcher : QObject
{
	Q_OBJECT

public:
	PyGoWaveListWatcher(PyGoWave::WaveModel * wave, PyGoWaveList * parent);

signals:
	void waveChanged(const QByteArray &waveId);

private slots:
	void avatarReady(const QString &url);
	void wavelet_participantsChanged();
	void participant_dataChanged();

private:
	PyGoWave::WaveModel * m_wave;
	QSignalMapper * m_participantMapper;
	QSet<QString> m_mappedParticipants;
};

class PyGoWaveParticipantsList : public QAbstractListModel
{
	Q_OBJECT

public:
	PyGoWaveParticipantsList(PyGoWave::Controller * controller, QObject * parent = 0);

	void sync(const QList<QByteArray> &ids);
	void add(const QByteArray &id);
	void remove(const QByteArray &id);

	int rowCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

private slots:
	void participant_dataChanged(const QString &s_id);
	void avatarReady(const QString &url);

private:
	PyGoWave::Controller * m_controller;
	QList<QByteArray> m_participants;
	QList<QPixmap> m_icons;

	QSignalMapper * m_participantMapper;
	QMap<QByteArray,int> m_mappedParticipants;

	QPixmap generateIcon(PyGoWave::Participant * p);
};

#endif // MODEL_QT_H
