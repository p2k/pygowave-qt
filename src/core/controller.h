/*
 * This file is part of the PyGoWave Qt/C++ Client API
 *
 * Copyright (C) 2009 Patrick Schneider <patrick.p2k.schneider@googlemail.com>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; see the file
 * COPYING.LESSER.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "pygowave_api_global.h"

#include <QtCore/QVariant>
#include <QtNetwork/QAbstractSocket>

class QAbstractItemModel;
class QTimer;
class QStompClient;

namespace QJson {
	class Serializer;
	class Parser;
}

namespace PyGoWave {

	class WaveModel;
	class Wavelet;
	class WaveList;
	class Participant;
	class OpManager;

	class ControllerPrivate;

	class PYGOWAVE_API_SHARED_EXPORT Controller : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(Controller)

	public:
		enum ClientState {
			ClientDisconnected,
			ClientConnected,
			ClientOnline
		};

		Controller(QObject * parent = 0);
		~Controller();

		void connectToHost(const QString &stompServer, const QString & username, const QString & password, int stompPort = 61613, const QByteArray & stompUsername = "pygowave_client", const QByteArray & stompPassword = "pygowave_client");
		void reconnectToHost(const QString & username, const QString & password);
		void disconnectFromHost();
		QString hostName() const;

		ClientState state() const;

		QAbstractItemModel * waveListModel();
		Participant * participant(const QByteArray &id) const;
		WaveModel * wave(const QByteArray &id) const;
		Wavelet * wavelet(const QByteArray &id) const;

		int searchForParticipant(const QString &);

	signals:
		void stateChanged(int);
		void errorOccurred(const QByteArray &waveletId, const QString &tag, const QString &desc);
		void waveletOpened(const QByteArray &waveletId, bool isRoot);
		void participantSearchResults(int searchId, const QList<QByteArray> &ids);
		void participantSearchResultsInvalid(int searchId, int minimumLetters);

		void waveAdded(const QByteArray &waveId, bool created, bool initial);
		void waveAboutToBeRemoved(const QByteArray &waveId);

	public slots:
		void textInserted(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &content);
		void textDeleted(const QByteArray &waveletId, const QByteArray &blipId, int start, int end);
		void elementDelete(const QByteArray &waveletId, const QByteArray &blipId, int index);
		void elementDeltaSubmitted(const QByteArray &waveletId, const QByteArray &blipId, int index, const QVariantMap &delta);
		void elementSetUserpref(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &key, const QString &value);

		void openWavelet(const QByteArray &waveletId);
		void closeWavelet(const QByteArray &waveletId);
		void addParticipant(const QByteArray &waveletId, const QByteArray &id);
		void createNewWave(const QString &title);
		void createNewWavelet(const QByteArray &waveId, const QString &title);
		void leaveWavelet(const QByteArray &waveletId);

	private:
		Q_DISABLE_COPY(Controller)

		Q_PRIVATE_SLOT(pd_func(), void _q_conn_socketConnected())
		Q_PRIVATE_SLOT(pd_func(), void _q_conn_socketDisconnected())
		Q_PRIVATE_SLOT(pd_func(), void _q_conn_frameReceived())
		Q_PRIVATE_SLOT(pd_func(), void _q_conn_socketStateChanged(QAbstractSocket::SocketState))

		Q_PRIVATE_SLOT(pd_func(), void _q_pingTimer_timeout())
		Q_PRIVATE_SLOT(pd_func(), void _q_pendingTimer_timeout())

		Q_PRIVATE_SLOT(pd_func(), void _q_mcached_afterOperationsInserted(int start, int end))
		Q_PRIVATE_SLOT(pd_func(), void _q_wavelet_participantsChanged())

		ControllerPrivate * const pd_ptr;
	};

}

#ifdef PYGOWAVE_API_P_INCLUDE
#  include "controller_p.h"
#endif

#endif // CONTROLLER_H
