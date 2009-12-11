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

#include "js_api.h"

#include "PyGoWaveApi/controller.h"
#include "PyGoWaveApi/model.h"

#include <qjson/serializer.h>

#include <QtWebKit/QWebFrame>

using namespace PyGoWave;

JavaScriptAPI::JavaScriptAPI(Controller * controller, const QByteArray &waveId, const QByteArray &waveletId, QObject * parent) : JSWrapperFactory(parent)
{
	this->m_controller = controller;
	this->m_waveId = waveId;
	this->m_waveletId = waveletId;
}

void JavaScriptAPI::install(QWebFrame * webFrame)
{
	QJson::Serializer jserializer;
	this->m_webFrame = webFrame;

	QStringList tr_catalog = QStringList()
		<< "Unfortunately, this Blip has gone out of sync with the server. You have to reload the page to be able to use it again."
		<< tr("Unfortunately, this Blip has gone out of sync with the server. You have to reload the page to be able to use it again.")
		<< "Sorry, but this Blip Editor has gone out of sync with the internal representation. Please click resync."
		<< tr("Sorry, but this Blip Editor has gone out of sync with the internal representation. Please click resync.")
		<< "The text content could not be rendered correctly. This may be a bug."
		<< tr("The text content could not be rendered correctly. This may be a bug.")
		<< "Reload"
		<< tr("Reload")
		<< "Resync"
		<< tr("Resync")
		<< "Dismiss"
		<< tr("Dismiss");

	QString tr_catalog_js;
	for (int i = 0; i < tr_catalog.size(); i += 2)
		tr_catalog_js.append(QString("catalog[%1] = %2;\n").arg(QString::fromUtf8(jserializer.serialize(tr_catalog[0])), QString::fromUtf8(jserializer.serialize(tr_catalog[1]))));
	this->m_webFrame->evaluateJavaScript(tr_catalog_js);

	this->registerFactory(webFrame);

	QVariantMap options;
	options["gadgetLoaderUrl"] = QString("http://%1/gadgets/load/?wave=1&").arg(this->m_controller->hostName());
	options["defaultThumbnailUrl"] = QString("qrc:/gui/gfx/default-avatar.png");
	this->m_webFrame->evaluateJavaScript(QString("pygowave.api.setup(%1, '%2', '%3', %4);").arg(this->javaScriptObjectName(), QString::fromAscii(this->m_waveId), QString::fromAscii(this->m_waveletId), QString::fromUtf8(jserializer.serialize(options))));
}

QStringList JavaScriptAPI::classNameList() const
{
	static const QStringList classes = QStringList() << "PyGoWave::Participant" << "PyGoWave::WaveModel" << "PyGoWave::Wavelet" << "PyGoWave::Blip" << "PyGoWave::GadgetElement";
	return classes;
}

JSWrapper * JavaScriptAPI::createWrapper(const QString &className, const QString &objectId, JSWrapper * parent)
{
	if (className == "PyGoWave::Participant") {
		Participant * p = this->m_controller->participant(objectId.toAscii());
		if (p)
			return new ParticipantWrapper(this->m_webFrame, p);
	}
	else if (className == "PyGoWave::WaveModel") {
		WaveModel * w = this->m_controller->wave(objectId.toAscii());
		if (w)
			return new WaveModelWrapper(this->m_webFrame, w);
	}
	else if (className == "PyGoWave::Wavelet") {
		QStringList ids = objectId.split('@');
		Q_ASSERT(ids.size() == 2);
		Wavelet * w = this->m_controller->wavelet(ids[0].toAscii());
		if (w)
			return new WaveletWrapper(this->m_webFrame, w, parent);
	}
	else if (className == "PyGoWave::Blip") {
		QStringList ids = objectId.split('@');
		Q_ASSERT(ids.size() == 3);
		Wavelet * w = this->m_controller->wavelet(ids[1].toAscii());
		if (w) {
			Blip * b = w->blipById(ids[0].toAscii());
			if (b)
				return new BlipWrapper(this->m_webFrame, b, parent);
		}
	}
	else if (className == "PyGoWave::GadgetElement") {
		QStringList ids = objectId.split('@');
		Q_ASSERT(ids.size() == 4);
		Wavelet * w = this->m_controller->wavelet(ids[2].toAscii());
		if (w) {
			Blip * b = w->blipById(ids[1].toAscii());
			if (b) {
				GadgetElement * g = qobject_cast<GadgetElement*>(b->elementById(ids[0].toInt()));
				if (g)
					return new GadgetElementWrapper(this->m_webFrame, g, parent);
			}
		}
	}
	return NULL;
}

void JavaScriptAPI::emitCurrentTextRangeChanged(int start, int end)
{
	emit currentTextRangeChanged(start, end);
}

void JavaScriptAPI::emitBlipEditing(bool enabled)
{
	emit blipEditing(enabled);
}

// Forwarding to controller
void JavaScriptAPI::textInserted(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &content)
{
	this->m_controller->textInserted(waveletId, blipId, index, content);
}
void JavaScriptAPI::textDeleted(const QByteArray &waveletId, const QByteArray &blipId, int start, int end)
{
	this->m_controller->textDeleted(waveletId, blipId, start, end);
}
void JavaScriptAPI::elementDelete(const QByteArray &waveletId, const QByteArray &blipId, int index)
{
	this->m_controller->elementDelete(waveletId, blipId, index);
}
void JavaScriptAPI::elementDeltaSubmitted(const QByteArray &waveletId, const QByteArray &blipId, int index, const QVariantMap &delta)
{
	this->m_controller->elementDeltaSubmitted(waveletId, blipId, index, delta);
}
void JavaScriptAPI::elementSetUserpref(const QByteArray &waveletId, const QByteArray &blipId, int index, const QString &key, const QString &value)
{
	this->m_controller->elementSetUserpref(waveletId, blipId, index, key, value);
}
void JavaScriptAPI::deleteBlip(const QByteArray &waveletId, const QByteArray &blipId)
{
	this->m_controller->deleteBlip(waveletId, blipId);
}
void JavaScriptAPI::draftBlip(const QByteArray &waveletId, const QByteArray &blipId, bool enabled)
{
	this->m_controller->draftBlip(waveletId, blipId, enabled);
}


ParticipantWrapper::ParticipantWrapper(QWebFrame * webFrame, Participant * participant) : JSWrapper(webFrame, participant, QString::fromAscii(participant->id()))
{
	this->m_participant = participant;
	connect(participant, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(participant, SIGNAL(onlineStateChanged(bool)), this, SLOT(onlineStateChanged(bool)));
}

void ParticipantWrapper::dataChanged()
{
	this->forwardSignal("dataChanged");
}

void ParticipantWrapper::onlineStateChanged(bool online)
{
	this->forwardSignal("onlineStateChanged", online);
}


WaveModelWrapper::WaveModelWrapper(QWebFrame * webFrame, WaveModel * wave) : JSWrapper(webFrame, wave, QString::fromAscii(wave->id()))
{
	this->m_wave = wave;
	connect(wave, SIGNAL(waveletAdded(QByteArray,bool)), this, SLOT(waveletAdded(QByteArray,bool)));
	connect(wave, SIGNAL(waveletAboutToBeRemoved(QByteArray)), this, SLOT(waveletAboutToBeRemoved(QByteArray)));
}

QStringList WaveModelWrapper::allWavelets() const
{
	QStringList ids;
	foreach (Wavelet * w, this->m_wave->allWavelets())
		ids << QString::fromAscii(w->id());
	return ids;
}

QString WaveModelWrapper::rootWavelet() const
{
	if (this->m_wave->rootWavelet())
		return this->m_wave->rootWavelet()->id();
	else
		return QString();
}

QString WaveModelWrapper::viewerId() const
{
	return QString::fromAscii(this->m_wave->viewerId());
}

void WaveModelWrapper::waveletAdded(const QByteArray & waveletId, bool isRoot)
{
	QVariantList args;
	args << QString::fromAscii(waveletId) << isRoot;
	this->forwardSignal("waveletAdded", args);
}

void WaveModelWrapper::waveletAboutToBeRemoved(const QByteArray & waveletId)
{
	this->forwardSignal("waveletAboutToBeRemoved", QString::fromAscii(waveletId));
}


WaveletWrapper::WaveletWrapper(QWebFrame * webFrame, Wavelet * wavelet, JSWrapper * parent) : JSWrapper(webFrame, wavelet, QString::fromAscii(wavelet->id()), parent)
{
	this->m_wavelet = wavelet;
	connect(wavelet, SIGNAL(participantsChanged()), this, SLOT(participantsChanged()));
	connect(wavelet, SIGNAL(blipInserted(int,QByteArray)), this, SLOT(blipInserted(int,QByteArray)));
	connect(wavelet, SIGNAL(blipDeleted(QByteArray)), this, SLOT(blipDeleted(QByteArray)));
	connect(wavelet, SIGNAL(statusChange(QByteArray)), this, SLOT(statusChange(QByteArray)));
	connect(wavelet, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged(QString)));
	connect(wavelet, SIGNAL(lastModifiedChanged(QDateTime)), this, SLOT(lastModifiedChanged(QDateTime)));
}

bool WaveletWrapper::hasParticipant(const QString &id) const
{
	return this->m_wavelet->participant(id.toAscii()) != NULL;
}

QStringList WaveletWrapper::allParticipants() const
{
	QStringList ids;
	foreach (Participant * p, this->m_wavelet->allParticipants())
		ids << QString::fromAscii(p->id());
	return ids;
}

QString WaveletWrapper::blipByIndex(int index) const
{
	Blip * b = this->m_wavelet->blipByIndex(index);
	if (b)
		return b->id();
	else
		return QString();
}

QStringList WaveletWrapper::allBlips() const
{
	QStringList ids;
	foreach (Blip * b, this->m_wavelet->allBlips())
		ids << QString::fromAscii(b->id());
	return ids;
}

void WaveletWrapper::participantsChanged()
{
	this->forwardSignal("participantsChanged");
}

void WaveletWrapper::blipInserted(int index, const QByteArray & blipId)
{
	QVariantList args;
	args << index << QString::fromAscii(blipId);
	this->forwardSignal("blipInserted", args);
}

void WaveletWrapper::blipDeleted(const QByteArray & blipId)
{
	this->forwardSignal("blipDeleted", QString::fromAscii(blipId));
}

void WaveletWrapper::statusChange(const QByteArray & status)
{
	this->forwardSignal("statusChange", QString::fromAscii(status));
}

void WaveletWrapper::titleChanged(const QString &title)
{
	this->forwardSignal("titleChanged", title);
}

void WaveletWrapper::lastModifiedChanged(const QDateTime &datetime)
{
	this->forwardSignal("lastModifiedChanged", toTimestamp(datetime));
}


BlipWrapper::BlipWrapper(QWebFrame * webFrame, Blip * blip, JSWrapper * parent) : JSWrapper(webFrame, blip, QString::fromAscii(blip->id()), parent)
{
	this->m_blip = blip;
	connect(blip, SIGNAL(deletedElement(int)), this, SLOT(deletedElement(int)));
	connect(blip, SIGNAL(deletedText(int,int)), this, SLOT(deletedText(int,int)));
	connect(blip, SIGNAL(insertedElement(int)), this, SLOT(insertedElement(int)));
	connect(blip, SIGNAL(insertedText(int,QString)), this, SLOT(insertedText(int,QString)));
	connect(blip, SIGNAL(outOfSync()), this, SLOT(outOfSync()));
	connect(blip, SIGNAL(lastModifiedChanged(QDateTime)), this, SLOT(lastModifiedChanged(QDateTime)));
	connect(blip, SIGNAL(idChanged(QByteArray,QByteArray)), this, SLOT(idChanged(QByteArray,QByteArray)));
	connect(blip, SIGNAL(contributorAdded(QByteArray)), this, SLOT(contributorAdded(QByteArray)));
}

QString BlipWrapper::elementAt(int index) const
{
	Element * e = this->m_blip->elementAt(index);
	if (e)
		return QString::number(e->id());
	else
		return QString();
}

QStringList BlipWrapper::elementsWithin(int start, int end) const
{
	QStringList ids;
	foreach (Element * e, this->m_blip->elementsWithin(start, end))
		ids << QString::number(e->id());
	return ids;
}

QStringList BlipWrapper::allElements() const
{
	QStringList ids;
	foreach (Element * e, this->m_blip->allElements())
		ids << QString::number(e->id());
	return ids;
}

QStringList BlipWrapper::allContributors() const
{
	QStringList ids;
	foreach (Participant * c, this->m_blip->allContributors())
		ids << QString::fromAscii(c->id());
	return ids;
}

QString BlipWrapper::creator() const
{
	return QString::fromAscii(this->m_blip->creator()->id());
}

void BlipWrapper::deletedElement(int index)
{
	this->forwardSignal("deletedElement", index);
}

void BlipWrapper::deletedText(int index, int length)
{
	QVariantList args;
	args << index << length;
	this->forwardSignal("deletedText", args);
}

void BlipWrapper::insertedElement(int index)
{
	this->forwardSignal("insertedElement", index);
}

void BlipWrapper::insertedText(int index, const QString &text)
{
	QVariantList args;
	args << index << text;
	this->forwardSignal("insertedText", args);
}

void BlipWrapper::outOfSync()
{
	this->forwardSignal("outOfSync");
}

void BlipWrapper::lastModifiedChanged(const QDateTime &datetime)
{
	this->forwardSignal("lastModifiedChanged", toTimestamp(datetime));
}

void BlipWrapper::idChanged(const QByteArray &oldId, const QByteArray &newId)
{
	QVariantList args;
	args << QString::fromAscii(oldId) << QString::fromAscii(newId);
	this->setObjectId(QString::fromAscii(newId));
	this->forwardSignal("idChanged", args);
}

void BlipWrapper::contributorAdded(const QByteArray &id)
{
	this->forwardSignal("contributorAdded", QString::fromAscii(id));
}


GadgetElementWrapper::GadgetElementWrapper(QWebFrame * webFrame, GadgetElement * gadget, JSWrapper * parent) : JSWrapper(webFrame, gadget, QString::number(gadget->id()), parent)
{
	this->m_gadget = gadget;
}

void GadgetElementWrapper::stateChange()
{
	this->forwardSignal("stateChange");
}

void GadgetElementWrapper::userPrefSet(const QString &key, const QString &value)
{
	QVariantList args;
	args << key << value;
	this->forwardSignal("userPrefSet", args);
}
