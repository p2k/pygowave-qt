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

#include "model_qt.h"
#include "avatarloader.h"

#include <PyGoWaveApi/model.h>
#include <PyGoWaveApi/controller.h>

#include <QtCore/QSignalMapper>
#include <QtGui/QPainter>

PyGoWaveList::PyGoWaveList(PyGoWave::Controller * controller, QObject * parent) : QAbstractListModel(parent)
{
	this->m_controller = controller;
	connect(this->m_controller, SIGNAL(waveAdded(QByteArray,bool,bool)), this, SLOT(controller_waveAdded(QByteArray)));
	connect(this->m_controller, SIGNAL(waveAboutToBeRemoved(QByteArray)), this, SLOT(controller_waveAboutToBeRemoved(QByteArray)));
}

PyGoWaveList::~PyGoWaveList()
{
}

int PyGoWaveList::rowCount(const QModelIndex & /*parent*/) const
{
	return this->m_waves_ordered.size();
}

QVariant PyGoWaveList::data(const QModelIndex & index, int role) const
{
	QByteArray waveletId = this->m_waves_ordered[index.row()];
	PyGoWave::WaveModel * wave = this->m_controller->wave(waveletId);
	if (role == Qt::DisplayRole)
		return wave->rootWavelet()->title();
	if (role == Qt::DecorationRole)
		return this->m_icons[index.row()];

	PyGoWaveList::ItemDataRole irole = (PyGoWaveList::ItemDataRole) role;
	switch (irole) {
		case PyGoWaveList::WaveIdRole:
			return wave->id();
		case PyGoWaveList::RootWaveletIdRole:
			return wave->rootWavelet()->id();
		case PyGoWaveList::RootWaveletObjectRole:
			return QVariant::fromValue((QObject*) wave->rootWavelet());
	}
	return QVariant();
}

QPixmap PyGoWaveList::generateIcon(PyGoWave::WaveModel * wave)
{
	PyGoWave::Wavelet * wavelet = wave->rootWavelet();
	static const QSize pIconSize = QSize(27, 27);
	static const QPixmap onlineIndicator(":/gui/icons/user-online.png");

	QList<PyGoWave::Participant*> participants = wavelet->allParticipants();
	QImage icon(96, 35, QImage::Format_ARGB32);
	icon.fill(0);
	QPainter painter(&icon);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	// Put creator first
	int cidx = participants.indexOf(wavelet->creator());
	if (cidx > 0) {
		participants[cidx] = participants[0];
		participants[0] = wavelet->creator();
	}

	for (int i = 0; i < participants.size() && i < 3; i++) {
		PyGoWave::Participant * p = participants.at(i);
		QPixmap ava;
		if (p->thumbnailUrl() != "")
			ava = AvatarLoader::instance()->getPrepared(p->thumbnailUrl(), pIconSize);
		if (ava.isNull())
			ava = AvatarLoader::instance()->defaultAvatarPrepared(pIconSize);
		painter.drawPixmap(QPoint(i*32, 4), ava);
		if (p->isOnline())
			painter.drawPixmap(22+i*32, 25, 8, 8, onlineIndicator);
	}

	return QPixmap::fromImage(icon);
}

void PyGoWaveList::controller_waveAdded(const QByteArray &waveId)
{
	int index = this->m_waves_ordered.size();
	this->beginInsertRows(QModelIndex(), index, index);
	this->m_waves_ordered.append(waveId);
	PyGoWave::WaveModel * wave = this->m_controller->wave(waveId);
	this->m_icons.append(this->generateIcon(wave));
	this->m_watchers.append(new PyGoWaveListWatcher(wave, this));
	this->endInsertRows();
}

void PyGoWaveList::controller_waveAboutToBeRemoved(const QByteArray &waveId)
{
	int index = this->m_waves_ordered.indexOf(waveId);
	this->beginRemoveRows(QModelIndex(), index, index);
	this->m_waves_ordered.removeAt(index);
	this->m_icons.removeAt(index);
	delete this->m_watchers.takeAt(index);
	this->endRemoveRows();
}

void PyGoWaveList::updateWave(const QByteArray &waveId)
{
	int index = this->m_waves_ordered.indexOf(waveId);
	if (index == -1)
		return;
	this->updateRowIcon(index);
}

void PyGoWaveList::updateRowIcon(int index)
{
	this->m_icons[index] = this->generateIcon(this->m_controller->wave(this->m_waves_ordered[index]));
	emit dataChanged(this->index(index), this->index(index));
}


PyGoWaveListWatcher::PyGoWaveListWatcher(PyGoWave::WaveModel * wave, PyGoWaveList * parent) : QObject(parent)
{
	this->m_wave = wave;
	this->m_participantMapper = new QSignalMapper(this);
	connect(this->m_participantMapper, SIGNAL(mapped(QString)), this, SLOT(participant_dataChanged()));
	connect(this, SIGNAL(waveChanged(QByteArray)), parent, SLOT(updateWave(QByteArray)));
	connect(AvatarLoader::instance(), SIGNAL(avatarReady(QString,bool)), this, SLOT(avatarReady(QString)));
	connect(wave->rootWavelet(), SIGNAL(participantsChanged()), this, SLOT(wavelet_participantsChanged()));
	this->wavelet_participantsChanged();
}

void PyGoWaveListWatcher::avatarReady(const QString &url)
{
	foreach(PyGoWave::Participant * p, this->m_wave->rootWavelet()->allParticipants()) {
		if (p->thumbnailUrl() == url) {
			emit waveChanged(this->m_wave->id());
			break;
		}
	}
}

void PyGoWaveListWatcher::wavelet_participantsChanged()
{
	QSet<QString> newSet;
	foreach (PyGoWave::Participant * p, this->m_wave->rootWavelet()->allParticipants()) {
		QString s_id = QString::fromAscii(p->id());
		newSet.insert(s_id);
		if (!this->m_mappedParticipants.contains(s_id)) {
			this->m_participantMapper->setMapping(p, s_id);
			this->m_mappedParticipants.insert(s_id);
			connect(p, SIGNAL(dataChanged()), this->m_participantMapper, SLOT(map()));
			connect(p, SIGNAL(onlineStateChanged(bool)), this->m_participantMapper, SLOT(map()));
		}
	}

	QSet<QString> oldSet = this->m_mappedParticipants;
	foreach (QString s_id, oldSet.subtract(newSet)) {
		QObject * p = this->m_participantMapper->mapping(s_id);
		this->m_participantMapper->removeMappings(p);
		p->disconnect(this->m_participantMapper);
		this->m_mappedParticipants.remove(s_id);
	}

	emit waveChanged(this->m_wave->id());
}

void PyGoWaveListWatcher::participant_dataChanged()
{
	emit waveChanged(this->m_wave->id());
}


PyGoWaveParticipantsList::PyGoWaveParticipantsList(PyGoWave::Controller * controller, QObject * parent) : QAbstractListModel(parent)
{
	this->m_controller = controller;
	this->m_participantMapper = new QSignalMapper(this);
	connect(AvatarLoader::instance(), SIGNAL(avatarReady(QString,bool)), this, SLOT(avatarReady(QString)));
	connect(this->m_participantMapper, SIGNAL(mapped(QString)), this, SLOT(participant_dataChanged(QString)));
}

void PyGoWaveParticipantsList::sync(const QList<QByteArray> &ids)
{
	QSet<QByteArray> newSet;
	foreach (QByteArray id, ids) {
		newSet.insert(id);
		if (!this->m_participants.contains(id))
			this->add(id);
	}

	QSet<QByteArray> oldSet = QSet<QByteArray>::fromList(this->m_participants);
	foreach (QByteArray id, oldSet.subtract(newSet))
		this->remove(id);
}

void PyGoWaveParticipantsList::add(const QByteArray &id)
{
	if (this->m_participants.contains(id))
		return;
	int index = this->m_participants.size();
	this->beginInsertRows(QModelIndex(), index, index);
	this->m_participants.append(id);
	PyGoWave::Participant * p = this->m_controller->participant(id);
	this->m_icons.append(this->generateIcon(p));
	this->m_participantMapper->setMapping(p, QString::fromAscii(id));
	connect(p, SIGNAL(dataChanged()), this->m_participantMapper, SLOT(map()));
	this->endInsertRows();
}

void PyGoWaveParticipantsList::remove(const QByteArray &id)
{
	int index = this->m_participants.indexOf(id);
	if (index == -1)
		return;
	this->beginRemoveRows(QModelIndex(), index, index);
	PyGoWave::Participant * p = this->m_controller->participant(id);
	this->m_participantMapper->removeMappings(p);
	p->disconnect(this->m_participantMapper);
	this->m_participants.removeAt(index);
	this->m_icons.removeAt(index);
	this->endRemoveRows();
}

void PyGoWaveParticipantsList::participant_dataChanged(const QString &s_id)
{
	QByteArray id = s_id.toAscii();
	int index = this->m_participants.indexOf(id);
	if (index == -1)
		return;
	PyGoWave::Participant * p = this->m_controller->participant(id);
	this->m_icons[index] = this->generateIcon(p);
	emit dataChanged(this->index(index), this->index(index));
}

void PyGoWaveParticipantsList::avatarReady(const QString &url)
{
	for (int index = 0; index < this->m_participants.size(); index++) {
		QByteArray id = this->m_participants[index];
		PyGoWave::Participant * p = this->m_controller->participant(id);
		if (p->thumbnailUrl() == url) {
			this->m_icons[index] = this->generateIcon(p);
			emit dataChanged(this->index(index), this->index(index));
			break;
		}
	}
}

QPixmap PyGoWaveParticipantsList::generateIcon(PyGoWave::Participant * p)
{
	static const QSize pIconSize = QSize(27, 27);
	static const QPixmap onlineIndicator(":/gui/icons/user-online.png");

	QImage icon(32, 35, QImage::Format_ARGB32);
	icon.fill(0);
	QPainter painter(&icon);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	QPixmap ava;
	if (p->thumbnailUrl() != "")
		ava = AvatarLoader::instance()->getPrepared(p->thumbnailUrl(), pIconSize);
	if (ava.isNull())
		ava = AvatarLoader::instance()->defaultAvatarPrepared(pIconSize);
	painter.drawPixmap(QPoint(0, 4), ava);
	if (p->isOnline())
		painter.drawPixmap(22, 25, 8, 8, onlineIndicator);

	return QPixmap::fromImage(icon);
}

int PyGoWaveParticipantsList::rowCount(const QModelIndex &) const
{
	return this->m_participants.size();
}

QVariant PyGoWaveParticipantsList::data(const QModelIndex & index, int role) const
{
	switch (role) {
		case Qt::DecorationRole:
			return this->m_icons[index.row()];
		case Qt::UserRole:
			return this->m_participants[index.row()];
		case Qt::DisplayRole:
			return this->m_controller->participant(this->m_participants[index.row()])->displayName();
		default:
			return QVariant();
	}
}
