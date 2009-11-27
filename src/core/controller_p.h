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

#ifndef CONTROLLER_P_H
#define CONTROLLER_P_H

#include "pygowave_api_global.h"

namespace PyGoWave {

	class ControllerPrivate
	{
		P_DECLARE_PUBLIC(Controller)
		public:
			ControllerPrivate(Controller * q);

			QStompClient * conn;
			QJson::Serializer * jserializer;
			QJson::Parser * jparser;
			QTimer * pingTimer;
			QTimer * pendingTimer;

			QString m_stompServer;
			int m_stompPort;
			QByteArray m_stompUsername;
			QByteArray m_stompPassword;

			QString m_username;
			QString m_password;

			QByteArray m_waveAccessKeyTx;
			QByteArray m_waveAccessKeyRx;
			QByteArray m_viewerId;

			Controller::ClientState m_state;

			QMap<QByteArray,WaveModel*> m_allWaves;
			QMap<QByteArray,Wavelet*> m_allWavelets;
			QMap<QByteArray,Participant*> m_allParticipants;

			QMap<QByteArray,OpManager*> mcached;
			QMap<QByteArray,OpManager*> mpending;
			QMap<QByteArray,bool> ispending;

			int m_lastSearchId;
			QByteArray m_createdWaveId;

			void addWave(WaveModel * wave, bool initial);
			void removeWave(const QByteArray &id, bool deleteObjects);
			void clearWaves(bool deleteObjects);

			void sendJson(const QByteArray & dest, const QString &type, const QVariant &property = QVariant());
			void subscribeWavelet(const QByteArray &id, bool open = true);
			void unsubscribeWavelet(const QByteArray &id, bool close = true);
			void processMessage(const QByteArray &waveletId, const QString &type, const QVariant &property = QVariant());

			Wavelet * newWaveletByDict(WaveModel * wave, const QByteArray &waveletId, const QVariantMap &waveletDict);
			void updateWaveletByDict(Wavelet * wavelet, const QVariantMap &waveletDict);
			void collateParticipants(const QList<QByteArray> & participants);
			void collateParticipants(const QByteArray & participant);
			quint64 timestamp();
			bool hasPendingOperations(const QByteArray &waveletId);
			void transferOperations(const QByteArray &waveletId);

			void queueMessageBundle(Wavelet * wavelet, const QVariant &serial_ops, int version, const QVariantMap &blipsums);
			void processMessageBundle(Wavelet * wavelet, const QVariant &serial_ops, int version, const QVariantMap &blipsums);

			void _q_conn_socketConnected();
			void _q_conn_socketDisconnected();
			void _q_conn_frameReceived();
			void _q_conn_socketStateChanged(QAbstractSocket::SocketState);
			void _q_pingTimer_timeout();
			void _q_pendingTimer_timeout();
			void _q_mcached_afterOperationsInserted(int start, int end);
			void _q_wavelet_participantsChanged();

		private:
			Controller * const pq_ptr;
	};
}

#endif // CONTROLLER_P_H
