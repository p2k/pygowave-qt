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

#include "pygowave_api_global.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace PyGoWave {

	class ParticipantPrivate;
	class AnnotationPrivate;
	class ElementPrivate;
	class GadgetElementPrivate;
	class WaveModelPrivate;
	class WaveletPrivate;
	class BlipPrivate;

	class PYGOWAVE_API_SHARED_EXPORT Participant : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(Participant)
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
		ParticipantPrivate * const pd_ptr;
	};

	struct IParticipantProvider
	{
		explicit IParticipantProvider() {}
		virtual ~IParticipantProvider() {}
		virtual Participant * participant(const QByteArray & id) = 0;
	};

	class Wavelet;
	class Blip;
	class Operation;

	class PYGOWAVE_API_SHARED_EXPORT Annotation : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(Annotation)
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

		QString value() const;

		Blip * blip() const;

	private:
		AnnotationPrivate * const pd_ptr;
	};

	class PYGOWAVE_API_SHARED_EXPORT Element : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(Element)
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
		Element(Blip * blip, int id, int position, Type type, const QVariantMap & properties, ElementPrivate * d);
		ElementPrivate * const pd_ptr;
	};

	class PYGOWAVE_API_SHARED_EXPORT GadgetElement : public Element
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(GadgetElement)
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

	class PYGOWAVE_API_SHARED_EXPORT WaveModel : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(WaveModel)
		Q_PROPERTY(QByteArray id READ id)
		Q_PROPERTY(QByteArray viewerId READ viewerId)

	public:
		WaveModel(const QByteArray & waveId, const QByteArray & viewerId, IParticipantProvider * pp = NULL);
		~WaveModel();

		QByteArray id() const;

		QByteArray viewerId() const;

		void loadFromSnapshot(const QVariantMap & obj);

		Wavelet * createWavelet(
				const QByteArray & id,
				Participant * creator = NULL,
				const QString & title = QString(),
				bool isRoot = false,
				const QDateTime & created = QDateTime(),
				const QDateTime & lastModified = QDateTime(),
				int version = 0
			);

		Wavelet * wavelet(const QByteArray & id) const;
		QList<Wavelet*> allWavelets() const;
		Wavelet * rootWavelet() const;
		void setRootWavelet(Wavelet * wavelet);
		void removeWavelet(const QByteArray & waveletId);

		IParticipantProvider * participantProvider() const;
		void setParticipantProvider(IParticipantProvider *);

	signals:
		void waveletAdded(const QByteArray & waveletId, bool isRoot);
		void waveletAboutToBeRemoved(const QByteArray & waveletId);

	private:
		WaveModelPrivate * const pd_ptr;
	};

	class PYGOWAVE_API_SHARED_EXPORT Wavelet : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(Wavelet)
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
		Wavelet(
				WaveModel * wave,
				const QByteArray & id,
				Participant * creator = NULL,
				const QString & title = QString(),
				bool isRoot = false,
				const QDateTime & created = QDateTime(),
				const QDateTime & lastModified = QDateTime(),
				int version = 0
			);
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

		Blip * appendBlip(
				const QByteArray & id,
				const QString & content = QString(),
				const QList<Element*> & elements = QList<Element*>(),
				Participant * creator = NULL,
				const QList<Participant*> contributors = QList<Participant*>(),
				bool isRoot = false,
				const QDateTime & lastModified = QDateTime(),
				int version = 0,
				bool submitted = false
			);

		Blip * insertBlip(
				int index,
				const QByteArray & id,
				const QString & content = QString(),
				const QList<Element*> & elements = QList<Element*>(),
				Participant * creator = NULL,
				const QList<Participant*> contributors = QList<Participant*>(),
				bool isRoot = false,
				const QDateTime & lastModified = QDateTime(),
				int version = 0,
				bool submitted = false
			);

		void deleteBlip(const QByteArray & id);
		Blip * blipByIndex(int index) const;
		Blip * blipById(const QByteArray & id) const;
		QList<Blip*> allBlips() const;

		void checkSync(const QMap<QByteArray, QByteArray> & blipsums);

		void applyOperations(
				const QList<Operation*> & operations,
				const QDateTime &timestamp,
				const QByteArray &contributorId
			);

		void updateBlipId(const QByteArray &tempId, const QByteArray &blipId);

		void loadBlipsFromSnapshot(const QVariantMap &blips, const QByteArray &rootBlipId);

	signals:
		void participantsChanged();
		void blipInserted(int index, const QByteArray & blipId);
		void blipDeleted(const QByteArray & blipId);
		void statusChange(const QByteArray & status);
		void titleChanged(const QString &title);
		void lastModifiedChanged(const QDateTime &datetime);

	private:
		WaveletPrivate * const pd_ptr;
	};

	class PYGOWAVE_API_SHARED_EXPORT Blip : public QObject
	{
		Q_OBJECT
		P_DECLARE_PRIVATE(Blip)
		Q_PROPERTY(QByteArray id READ id)
		Q_PROPERTY(bool root READ isRoot)
		Q_PROPERTY(QString content READ content)
		Q_PROPERTY(QDateTime lastModified READ lastModified WRITE setLastModified)

	public:
		Blip(
				Wavelet * wavelet,
				const QByteArray & id,
				const QString & content = QString(),
				const QList<Element*> & elements = QList<Element*>(),
				Blip * parent = NULL,
				Participant * creator = NULL,
				const QList<Participant*> contributors = QList<Participant*>(),
				bool isRoot = false,
				const QDateTime & lastModified = QDateTime(),
				int version = 0,
				bool submitted = false
			);
		~Blip();

		QByteArray id() const;
		void setId(const QByteArray &id);

		bool isRoot() const;

		QString content() const;

		QDateTime lastModified() const;
		void setLastModified(const QDateTime &value);

		Participant * creator() const;

		QList<Participant*> allContributors() const;

		void addContributor(Participant * contributor);

		Wavelet * wavelet() const;
		Element * elementById(int id) const;
		Element * elementAt(int index) const;
		QList<Element*> elementsWithin(int start, int end) const;
		QList<Element*> allElements() const;

		void insertElement(int index, Element::Type type, const QVariantMap & properties, Participant * contributor, bool noevent = false);
		void deleteElement(int index, Participant * contributor, bool noevent = false);
		void insertText(int index, const QString & text, Participant * contributor, bool noevent = false);
		void deleteText(int index, int length, Participant * contributor, bool noevent = false);
		void applyElementDelta(int index, const QVariantMap & delta, Participant * contributor);
		void setElementUserpref(int index, const QString & key, const QString & value, Participant * contributor, bool noevent = false);

		bool checkSync(const QByteArray & sum);

	signals:
		void insertedText(int index, const QString & text);
		void deletedText(int index, int length);
		void insertedElement(int index);
		void deletedElement(int index);
		void outOfSync();
		void lastModifiedChanged(const QDateTime &datetime);
		void idChanged(const QByteArray &oldId, const QByteArray &newId);
		void contributorAdded(const QByteArray &id);

	private:
		BlipPrivate * const pd_ptr;
	};

	QDateTime PYGOWAVE_API_SHARED_EXPORT parseTimestamp(const QVariant &timestamp);
	QVariant PYGOWAVE_API_SHARED_EXPORT toTimestamp(const QDateTime &datetime);
}

#ifdef PYGOWAVE_API_P_INCLUDE
#  include "model_p.h"
#endif

#endif // MODEL_H
