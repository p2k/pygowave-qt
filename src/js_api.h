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

#ifndef JS_API_H
#define JS_API_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

#include <JSWrapper/jswrapper.h>

class QWebFrame;

namespace PyGoWave {

	class Controller;
	class WaveModel;
	class Wavelet;
	class Participant;
	class Blip;
	class GadgetElement;

	class JavaScriptAPI : public JSWrapperFactory
	{
		Q_OBJECT

	public:
		JavaScriptAPI(Controller * controller, const QByteArray &waveId, const QByteArray &waveletId, QObject * parent = NULL);
		QStringList classNameList() const;
		void install(QWebFrame * webFrame);

		bool checkOrAddNewline(int index);

	signals:
		void currentTextRangeChanged(int start, int end);
		void blipEditing(const QByteArray &blipId);

	public slots:
		void textInserted(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &content);
		void textDeleted(const QByteArray &waveletId, const QByteArray &blipId, int start, int end);
		void elementDelete(const QByteArray &waveletId, const QByteArray &blipId, int index);
		void elementDeltaSubmitted(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &s_delta);
		void elementSetUserpref(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &key, const QString &value);
		void deleteBlip(const QByteArray &waveletId, const QByteArray &blipId);
		void draftBlip(const QByteArray &waveletId, const QByteArray &blipId, bool enabled);

		void emitCurrentTextRangeChanged(int start, int end);
		void emitBlipEditing(const QString &blipId);

	protected:
		JSWrapper * createWrapper(const QString &className, const QString &objectId, JSWrapper * parent);

	private:
		Q_DISABLE_COPY(JavaScriptAPI)

		QWebFrame * m_webFrame;
		QByteArray m_waveId;
		QByteArray m_waveletId;

		Controller * m_controller;
	};


	class ParticipantWrapper : public JSWrapper
	{
		Q_OBJECT

	public:
		ParticipantWrapper(QWebFrame * webFrame, Participant * participant);

	private slots:
		void dataChanged();
		void onlineStateChanged(bool);

	private:
		Participant * m_participant;
	};


	class WaveModelWrapper : public JSWrapper
	{
		Q_OBJECT

	public:
		WaveModelWrapper(QWebFrame * webFrame, WaveModel * wave);

	public slots:
		QStringList allWavelets() const;
		QString rootWavelet() const;
		QString viewerId() const;

	private slots:
		void waveletAdded(const QByteArray &, bool);
		void waveletAboutToBeRemoved(const QByteArray &);

	private:
		WaveModel * m_wave;
	};


	class WaveletWrapper : public JSWrapper
	{
		Q_OBJECT

	public:
		WaveletWrapper(QWebFrame * webFrame, Wavelet * wavelet, JSWrapper * parent);

	public slots:
		bool hasParticipant(const QString &id) const;
		QStringList allParticipants() const;

		QString blipByIndex(int index) const;
		QStringList allBlips() const;

	private slots:
		void participantsChanged();
		void blipInserted(int, const QByteArray &);
		void blipDeleted(const QByteArray & blipId);
		void statusChange(const QByteArray &);
		void titleChanged(const QString &title);
		void lastModifiedChanged(const QDateTime &);

	private:
		Wavelet * m_wavelet;
	};


	class BlipWrapper : public JSWrapper
	{
		Q_OBJECT

	public:
		BlipWrapper(QWebFrame * webFrame, Blip * blip, JSWrapper * parent);

	public slots:
		QString elementAt(int index) const;
		QStringList elementsWithin(int start, int end) const;
		QStringList allElements() const;
		QStringList allContributors() const;
		QString creator() const;

	private slots:
		void deletedElement(int);
		void deletedText(int, int);
		void insertedElement(int);
		void insertedText(int, const QString &);
		void outOfSync();
		void lastModifiedChanged(const QDateTime &);
		void idChanged(const QByteArray &oldId, const QByteArray &newId);
		void contributorAdded(const QByteArray &id);

	private:
		Blip * m_blip;
	};


	class GadgetElementWrapper : public JSWrapper
	{
		Q_OBJECT

	public:
		GadgetElementWrapper(QWebFrame * webFrame, GadgetElement * gadget, JSWrapper * parent);

	public slots:
		QString fieldsJSON() const;

	private slots:
		void stateChange();
		void userPrefSet(const QString &, const QString &);

	private:
		GadgetElement * m_gadget;
	};
}

#endif // JS_API_H
