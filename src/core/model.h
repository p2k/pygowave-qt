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

#ifndef MODEL_H
#define MODEL_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace PyGoWave {

	class Participant : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(QByteArray id READ id)
		Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName)
		Q_PROPERTY(QString thumbnailUrl READ thumbnailUrl WRITE setThumbnailUrl)
		Q_PROPERTY(QString profileUrl READ profileUrl WRITE setProfileUrl)
		Q_PROPERTY(bool online READ isOnline WRITE setOnline)
		Q_PROPERTY(bool bot READ isBot WRITE setBot)
		Q_PROPERTY(QVariantMap gadgetFormat READ toGadgetFormat)

	public:
		Participant(const QByteArray & id);
		~Participant();

		QByteArray id() const;

		QString displayName() const;
		void setDisplayName(const QString & value);

		QString thumbnailUrl() const;
		void setThumbnailUrl(const QString & value);

		QString profileUrl() const;
		void setProfileUrl(const QString & value);

		bool isOnline() const;
		void setOnline(bool value);

		bool isBot() const;
		void setBot(bool value);

		QVariantMap toGadgetFormat() const;

		void updateData(const QVariantMap & obj, const QString &server);

	signals:
		void onlineStateChanged(bool online);
		void dataChanged();

	private:
		QByteArray m_id;
		QString m_displayName;
		QString m_thumbnailUrl;
		QString m_profileUrl;
		bool m_online;
		bool m_bot;
	};

	class Wavelet;
	class Blip;
	class Operation;

	class Annotation : public QObject {
		Q_OBJECT
		Q_PROPERTY(QString name READ name)
		Q_PROPERTY(int start READ start WRITE setStart)
		Q_PROPERTY(int end READ end WRITE setEnd)
		Q_PROPERTY(QString value READ value)

	public:
		Annotation(Blip * blip, const QString & name, int start, int end, const QString & value);
		~Annotation();

		QString name() const;

		int start() const;
		void setStart(int index);

		int end() const;
		void setEnd(int index);

		QString value();

		Blip * blip() const;

	private:
		Blip * m_blip;
		QString m_name;
		int m_start;
		int m_end;
		QString m_value;
	};

	class Element : public QObject {
		Q_OBJECT
		Q_ENUMS(Type)
		Q_PROPERTY(int id READ id)
		Q_PROPERTY(int type READ type)
		Q_PROPERTY(int position READ position WRITE setPosition)

	public:
		enum Type {
			NOTHING = 0,
			INLINE_BLIP = 1,
			GADGET = 2,
			INPUT = 3,
			CHECK = 4,
			LABEL = 5,
			BUTTON = 6,
			RADIO_BUTTON = 7,
			RADIO_BUTTON_GROUP = 8,
			PASSWORD = 9,
			IMAGE = 10
		};

		Element(Blip * blip, int id, int position, Type type, const QVariantMap & properties);
		virtual ~Element();

		Blip * blip() const;
		void setBlip(Blip * blip);

		int id() const;

		Type type() const;

		int position() const;
		void setPosition(int pos);

	protected:
		QVariantMap m_properties;
		static int newTempId();

	private:
		Blip * m_blip;
		int m_id;
		int m_pos;
		Type m_type;

		static int g_lastTempId;
	};

	class GadgetElement : public Element {
		Q_OBJECT
		Q_PROPERTY(QVariantMap fields READ fields)
		Q_PROPERTY(QVariantMap userPrefs READ userPrefs)
		Q_PROPERTY(QString url READ url)

	public:
		GadgetElement(Blip * blip, int id, int position, const QVariantMap & properties);
		~GadgetElement();

		QVariantMap fields() const;

		QVariantMap userPrefs() const;

		QString url() const;

	public slots:
		void applyDelta(const QVariantMap & delta);
		void setUserPref(const QString & key, const QString & value, bool noevent = false);

	signals:
		void stateChange();
		void userPrefSet(const QString & key, const QString & value);

	};

	class WaveModel : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(QByteArray id READ id)
		Q_PROPERTY(QByteArray viewerId READ viewerId)

	public:
		WaveModel(const QByteArray & waveId, const QByteArray & viewerId);
		~WaveModel();

		QByteArray id() const;

		QByteArray viewerId() const;

		void loadFromSnapshot(const QVariantMap & obj, const QMap<QByteArray, Participant*> & participants);
		Wavelet * createWavelet(const QByteArray & id, Participant * creator = NULL, const QString & title = QString(), bool isRoot = false, const QDateTime & created = QDateTime(), const QDateTime & lastModified = QDateTime(), int version = 0);
		Wavelet * wavelet(const QByteArray & id) const;
		QList<Wavelet*> allWavelets() const;
		Wavelet * rootWavelet() const;
		void setRootWavelet(Wavelet * wavelet);
		void removeWavelet(const QByteArray & waveletId);

	signals:
		void waveletAdded(const QByteArray & waveletId, bool isRoot);
		void waveletAboutToBeRemoved(const QByteArray & waveletId);

	private:
		Wavelet * m_rootWavelet;
		QByteArray m_waveId;
		QByteArray m_viewerId;
		QMap< QByteArray, Wavelet* > m_wavelets;

	};

	class Wavelet : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(int version READ version WRITE setVersion)
		Q_PROPERTY(bool root READ isRoot)
		Q_PROPERTY(QByteArray id READ id)
		Q_PROPERTY(QByteArray waveId READ waveId)
		Q_PROPERTY(QString title READ title WRITE setTitle)
		Q_PROPERTY(int participantCount READ participantCount)
		Q_PROPERTY(QByteArray status READ status WRITE setStatus)
		Q_PROPERTY(QDateTime created READ created)
		Q_PROPERTY(QDateTime lastModified READ lastModified WRITE setLastModified)
		Q_PROPERTY(QVariantMap allParticipantsForGadget READ allParticipantsForGadget)

	public:
		Wavelet(WaveModel * wave, const QByteArray & id, Participant * creator = NULL, const QString & title = QString(), bool isRoot = false, const QDateTime & created = QDateTime(), const QDateTime & lastModified = QDateTime(), int version = 0);
		~Wavelet();

		int version() const;
		void setVersion(int value);

		QString title() const;
		void setTitle(const QString &title);

		bool isRoot() const;

		QByteArray id() const;

		QByteArray waveId() const;

		int participantCount() const;

		QList<QByteArray> allParticipantIDs() const;

		QVariantMap allParticipantsForGadget() const;

		QList<QByteArray> allBlipIDs() const;

		void setStatus(const QByteArray & status);
		QByteArray status() const;

		QDateTime created() const;

		QDateTime lastModified() const;
		void setLastModified(const QDateTime &value);

		Participant * creator() const;
		WaveModel * waveModel() const;

		void addParticipant(Participant * participant);
		void removeParticipant(const QByteArray & participantId);

		Participant * participant(const QByteArray & id) const;
		QList<Participant*> allParticipants() const;

		Blip * appendBlip(const QByteArray & id, const QString & content = QString(), const QList<Element*> & elements = QList<Element*>(), Participant * creator = NULL, bool isRoot = false, const QDateTime & lastModified = QDateTime(), int version = 0, bool submitted = false);
		Blip * insertBlip(int index, const QByteArray & id, const QString & content = QString(), const QList<Element*> & elements = QList<Element*>(), Participant * creator = NULL, bool isRoot = false, const QDateTime & lastModified = QDateTime(), int version = 0, bool submitted = false);
		Blip * blipByIndex(int index) const;
		Blip * blipById(const QByteArray & id) const;
		QList<Blip*> allBlips() const;

		void checkSync(const QMap<QByteArray, QByteArray> & blipsums);

		void applyOperations(const QList<Operation*> & operations, const QMap<QByteArray, Participant*> & participants);

		void loadBlipsFromSnapshot(const QVariantMap &blips, const QByteArray &rootBlipId, const QMap<QByteArray, Participant*> & participants);

	signals:
		void participantsChanged();
		void blipInserted(int index, const QByteArray & blipId);
		void statusChange(const QByteArray & status);
		void titleChanged(const QString &title);
		void lastModifiedChanged(const QDateTime &datetime);

	private:
		void setRootBlip(Blip * blip);

		WaveModel * m_wave;

		QByteArray m_id;
		Participant * m_creator;
		QString m_title;
		bool m_root;
		QDateTime m_created;
		QDateTime m_lastModified;
		int m_version;

		QMap<QByteArray, Participant*> m_participants;
		QList<Blip*> m_blips;
		Blip * m_rootBlip;
		QByteArray m_status;
	};

	class Blip : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(QByteArray id READ id)
		Q_PROPERTY(bool root READ isRoot)
		Q_PROPERTY(QString content READ content)

	public:
		Blip(Wavelet * wavelet, const QByteArray & id, const QString & content = QString(), const QList<Element*> & elements = QList<Element*>(), Blip * parent = NULL, Participant * creator = NULL, bool isRoot = false, const QDateTime & lastModified = QDateTime(), int version = 0, bool submitted = false);
		~Blip();

		QByteArray id() const;

		bool isRoot() const;

		QString content() const;

		Wavelet * wavelet() const;
		Element * elementById(int id) const;
		Element * elementAt(int index) const;
		QList<Element*> elementsWithin(int start, int end) const;
		QList<Element*> allElements() const;

		void insertElement(int index, Element::Type type, const QVariantMap & properties, bool noevent = false);
		void deleteElement(int index, bool noevent = false);

		bool checkSync(const QByteArray & sum);

	public slots:
		void insertText(int index, const QString & text, bool noevent = false);
		void deleteText(int index, int length, bool noevent = false);
		void applyElementDelta(int index, const QVariantMap & delta);
		void setElementUserpref(int index, const QString & key, const QString & value, bool noevent = false);

	signals:
		void insertedText(int index, const QString & text);
		void deletedText(int index, int length);
		void insertedElement(int index);
		void deletedElement(int index);
		void outOfSync();

	private:
		Wavelet * m_wavelet;
		QByteArray m_id;
		QString m_content;
		QList<Element*> m_elements;
		Blip * m_parent;
		Participant * m_creator;
		bool m_root;
		QDateTime m_lastModified;
		int m_version;
		bool m_submitted;
		bool m_outofsync;
		QList<Annotation*> m_annotations;

	};
}

#endif // MODEL_H
