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

	class Controller : public QObject
	{
		Q_OBJECT

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

		ClientState state();

		QAbstractItemModel * waveListModel();
		Participant * participant(const QByteArray &id);
		WaveModel * wave(const QByteArray &id);
		Wavelet * wavelet(const QByteArray &id);

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

	private slots:
		void on_conn_socketConnected();
		void on_conn_socketDisconnected();
		void on_conn_frameReceived();
		void on_conn_socketStateChanged(QAbstractSocket::SocketState);

		void on_pingTimer_timeout();
		void on_pendingTimer_timeout();

		void mcached_afterOperationsInserted(int start, int end);
		void wavelet_participantsChanged();

	private:
		Q_DISABLE_COPY(Controller)

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

		ClientState m_state;

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
	};

}

#endif // CONTROLLER_H
