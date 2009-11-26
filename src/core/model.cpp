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

#include "model.h"
#include "operations.h"

#include <QtCore/QCryptographicHash>

using namespace PyGoWave;

/*!
	\class PyGoWave::Participant
	\brief Models a participant to a Wavelet.

	Note that the implementation (i.e. the controller) should only create
	one participant object per participant because view objects are
	connected to only one participant instance. If the state of a participant
	changes they can be updated as appropriate.
 */

/*!
	\fn void Participant::onlineStateChanged(bool online)

	Fired if the participant's online state changes. The new state is
	given by \a online.

	\sa online
*/

/*!
	\fn void Participant::dataChanged()

	Fired if the participant's data changes.
*/

/*!
	Constructs a Participant with the given \a id (= address).
*/
Participant::Participant(const QByteArray & id)
{
	this->m_id = id;
	this->m_bot = false;
	this->m_online = false;
}

/*!
	Destroys the Participant
*/
Participant::~Participant() {}

/*!
	\property Participant::id
	\brief the ID (address) of the participant.
*/
QByteArray Participant::id() const
{
	return this->m_id;
}

/*!
	\property Participant::displayName
	\brief the participant's display name.
*/
QString Participant::displayName() const
{
	return this->m_displayName;
}
void Participant::setDisplayName(const QString & value)
{
	if (this->m_displayName != value) {
		this->m_displayName = value;
		emit dataChanged();
	}
}

/*!
	\property Participant::thumbnailUrl
	\brief the URL of the participant's avatar.
*/
QString Participant::thumbnailUrl() const
{
	return this->m_thumbnailUrl;
}
void Participant::setThumbnailUrl(const QString & value)
{
	if (this->m_thumbnailUrl != value) {
		this->m_thumbnailUrl = value;
		emit dataChanged();
	}
}

/*!
	\property Participant::profileUrl
	\brief the URL to the participant's profile.
*/
QString Participant::profileUrl() const
{
	return this->m_profileUrl;
}
void Participant::setProfileUrl(const QString & value)
{
	if (this->m_profileUrl != value) {
		this->m_profileUrl = value;
		emit dataChanged();
	}
}

/*!
	\property Participant::bot
	\brief weather the participant is a bot.
*/
bool Participant::isBot() const
{
	return this->m_bot;
}
void Participant::setBot(bool value)
{
	if (this->m_bot != value) {
		this->m_bot = value;
		emit dataChanged();
	}
}

/*!
	\property Participant::online
	\brief weather the participant is online.

	Setting this property fires onlineStateChanged() if changed.
*/
bool Participant::isOnline() const
{
	return this->m_online;
}
void Participant::setOnline(bool value)
{
	if (this->m_online != value) {
		this->m_online = value;
		emit onlineStateChanged(value);
	}
}

/*!
	Updates participant data from the JSON-serialized map/dict \a obj.
	The server name for local thumbnails is specified via the \a server
	parameter.

	Fires dataChanged()
*/
void Participant::updateData(const QVariantMap & obj, const QString &server)
{
	this->m_displayName = obj["displayName"].toString();
	this->m_thumbnailUrl = obj["thumbnailUrl"].toString();
	if (this->m_thumbnailUrl.startsWith("/"))
		this->m_thumbnailUrl.prepend("http://" + server);
	this->m_profileUrl = obj["profileUrl"].toString();
	this->m_bot = obj["isBot"].toBool();
	emit dataChanged();
}

/*!
	\property Participant::gadgetFormat
	\brief a ready-to-use map of this object in Gadget API format.
*/
QVariantMap Participant::toGadgetFormat() const
{
	QVariantMap map;
	map["id"] = this->m_id;
	map["displayName"] = this->m_displayName;
	map["thumbnailUrl"] = this->m_thumbnailUrl;
	return map;
}


/*!
	\class PyGoWave::WaveModel
	\brief Wave model class.
*/

/*!
	\fn WaveModel::waveletAdded(const QByteArray & waveletId, bool isRoot)

	Fired if a Wavelet has been added.
*/

/*!
	\fn WaveModel::waveletAboutToBeRemoved(const QByteArray & waveletId)

	Fired before a wavelet is removed.
*/

/*!
	Constructs a new WaveModel object.

	\a viewerId is the Participant ID of the wave viewer.
*/
WaveModel::WaveModel(const QByteArray & waveId, const QByteArray & viewerId)
{
	this->m_rootWavelet = NULL;
	this->m_waveId = waveId;
	this->m_viewerId = viewerId;
}

/*!
	Destroys the WaveModel and its child Wavelets. Emits
	waveletAboutToBeRemoved() once per Wavelet.
*/
WaveModel::~WaveModel()
{
	foreach (QByteArray id, this->m_wavelets.keys())
		this->removeWavelet(id);
}

/*!
	\property WaveModel::id
	\brief the ID of this Wave.
*/
QByteArray WaveModel::id() const
{
	return this->m_waveId;
}

/*!
	\property WaveModel::viewerId
	\brief the ID of the viewer.
*/
QByteArray WaveModel::viewerId() const
{
	return this->m_viewerId;
}

/*!
	Load the wave's contents from a JSON-serialized snapshot and a map of
	participant objects.
*/
void WaveModel::loadFromSnapshot(const QVariantMap & obj, const QMap<QByteArray, Participant*> & participants)
{
	QVariantMap rootWavelet = obj["wavelet"].toMap();
	QByteArray waveletId = rootWavelet["waveletId"].toByteArray();
	Wavelet * rootWaveletObj = this->createWavelet(
			waveletId,
			participants[rootWavelet["creator"].toByteArray()],
			rootWavelet["title"].toString(),
			true,
			QDateTime::fromTime_t(rootWavelet["creationTime"].toUInt()),
			QDateTime::fromTime_t(rootWavelet["lastModifiedTime"].toUInt()),
			rootWavelet["version"].toInt()
		);

	foreach (QVariant part_id, rootWavelet["participants"].toList()) {
		rootWaveletObj->addParticipant(participants[part_id.toByteArray()]);
	}

	QVariantMap blips = obj["blips"].toMap();
	rootWaveletObj->loadBlipsFromSnapshot(blips, rootWavelet["rootBlipId"].toByteArray(), participants);
}

/*!
	Create a Wavelet and add it to this Wave. For parameters see the
	\link Wavelet::Wavelet Wavelet constructor\endlink.

	Note: Fires waveletAdded()
*/
Wavelet * WaveModel::createWavelet(const QByteArray & id, Participant * creator, const QString & title, bool isRoot, const QDateTime & created, const QDateTime & lastModified, int version)
{
	Wavelet * w = new Wavelet(this, id, creator, title, isRoot, created, lastModified, version);
	this->m_wavelets[id] = w;
	emit waveletAdded(id, isRoot);
	return w;
}

/*!
	Return a Wavelet of this Wave by its \a id.
*/
Wavelet * WaveModel::wavelet(const QByteArray & id) const
{
	if (!this->m_wavelets.contains(id))
		return NULL;
	return this->m_wavelets[id];
}

/*!
	Return a list of all Wavelets on this Wave.
*/
QList<Wavelet*> WaveModel::allWavelets() const
{
	return this->m_wavelets.values();
}

/*!
	Returns the root Wavelet of this Wave.
*/
Wavelet * WaveModel::rootWavelet() const
{
	return this->m_rootWavelet;
}

/*!
	\internal
	Sets the root Wavelet of this Wave.
*/
void WaveModel::setRootWavelet(Wavelet * wavelet)
{
	this->m_rootWavelet = wavelet;
}

/*!
	Removes and deletes a wavelet by its id. Emits waveletAboutToBeRemoved() beforehand.
*/
void WaveModel::removeWavelet(const QByteArray & waveletId)
{
	if (!this->m_wavelets.contains(waveletId))
		return;

	emit waveletAboutToBeRemoved(waveletId);
	Wavelet * wavelet = this->m_wavelets.take(waveletId);
	if (wavelet == this->m_rootWavelet)
		this->m_rootWavelet = NULL;
	wavelet->deleteLater();
}


/*!
	\class PyGoWave::Wavelet
	\brief Models a Wavelet on a Wave.

	This class should not be instanitated directly. Please
	use \link WaveModel::createWavelet() \endlink.
*/

/*!
	\fn Wavelet::participantsChanged()

	Fired if a participant joins or leaves.
*/

/*!
	\fn Wavelet::blipInserted(int index, const QByteArray & blipId)

	Fired if a Blip was inserted.
*/

/*!
	\fn Wavelet::statusChange(const QByteArray & status)

	Fired when the Wavelet's status changed.
*/

/*!
	\fn Wavelet::titleChanged(const QString &title)

	Fired when the Wavelet's title changed
*/

/*!
	\fn Wavelet::lastModifiedChanged(const QDateTime &datetime)

	Fired when the Wavelet's last modification date/time changed.
*/

/*!
	Creates a new Wavelet object.

	\a wave - Parent WaveModel object

	\a id - Wavelet ID

	\a creator - Creator of this Wavelet

	\a title - Title of the Wavelet

	\a isRoot - True if this is the root Wavelet; if this value
	is set and the parent WaveModel does not have a root Wavelet yet,
	this Wavelet is set as the Wave's root Wavelet

	\a created - Date of creation

	\a lastModified - Date of last modification

	\a version - Version of the Wavelet
*/
Wavelet::Wavelet(WaveModel * wave, const QByteArray & id, Participant* creator, const QString & title, bool isRoot, const QDateTime & created, const QDateTime & lastModified, int version) : QObject(wave)
{
	this->m_wave = wave;
	this->m_id = id;
	this->m_creator = creator;
	this->m_title = title;
	this->m_root = isRoot;
	this->m_created = created;
	this->m_lastModified = lastModified;
	this->m_version = version;
	this->m_rootBlip = NULL;
	this->m_status = "clean";

	if (isRoot) {
		if (wave->rootWavelet() == NULL)
			wave->setRootWavelet(this);
		else
			this->m_root = false;
	}
}

/*!
	Destroys the Wavelet object.
*/
Wavelet::~Wavelet() {}

Participant * Wavelet::creator() const
{
	return this->m_creator;
}

/*!
	\property Wavelet::version
	\brief the version of the Wavelet.
*/
int Wavelet::version() const
{
	return this->m_version;
}
void Wavelet::setVersion(int value)
{
	this->m_version = value;
}


/*!
	\property Wavelet::title
	\brief the title of this Wavelet

	Emits titleChanged() on change.
*/
QString Wavelet::title() const
{
	return this->m_title;
}
void Wavelet::setTitle(const QString &title)
{
	if (this->m_title == title)
		return;
	this->m_title = title;
	emit titleChanged(title);
}

/*!
	\property Wavelet::root
	\brief weather this Wavelet is the Wave's root Wavelet.
*/
bool Wavelet::isRoot() const
{
	return this->m_root;
}

/*!
	\property Wavelet::id
	\brief the ID of this Wavelet.
*/
QByteArray Wavelet::id() const
{
	return this->m_id;
}

/*!
	\property Wavelet::waveId
	\brief the ID of this Wavelet's Wave.
*/
QByteArray Wavelet::waveId() const
{
	return this->m_wave->id();
}

/*!
	Returns the parent WaveModel object.
*/
WaveModel * Wavelet::waveModel() const
{
	return this->m_wave;
}

/*!
	Add a participant to this Wavelet.

	Note: Fires participantsChanged()
*/
void Wavelet::addParticipant(Participant * participant)
{
	if (!this->m_participants.contains(participant->id())) {
		this->m_participants[participant->id()] = participant;
		emit participantsChanged();
	}
}

/*!
	Removes a participant from this Wavelet.

	Note: Fires participantsChanged()
*/
void Wavelet::removeParticipant(const QByteArray & participantId)
{
	if (this->m_participants.contains(participantId)) {
		this->m_participants.remove(participantId);
		emit participantsChanged();
	}
}

/*!
	Returns the Participant object with the given id, if the participant
	resides on this Wavelet. Returns null otherwise.
*/
Participant * Wavelet::participant(const QByteArray & id) const
{
	if (this->m_participants.contains(id))
		return this->m_participants[id];
	else
		return NULL;
}

/*!
	\property Wavelet::participantCount
	\brief the number of Participants on this Wavelet
*/
int Wavelet::participantCount() const
{
	return this->m_participants.size();
}

/*!
	Returns a list of all participants on this Wavelet.
*/
QList<Participant*> Wavelet::allParticipants() const
{
	return this->m_participants.values();
}

/*!
	Returns a list of all IDs of the participants on this Wavelet.
*/
QList<QByteArray> Wavelet::allParticipantIDs() const
{
	return this->m_participants.keys();
}

/*!
	\property Wavelet::allParticipantsForGadget
	\brief a ready-to-use map of all participants on the Wavelet in Gadget API format.
*/
QVariantMap Wavelet::allParticipantsForGadget() const
{
	QVariantMap ret;
	foreach (QByteArray id, this->m_participants.keys())
		ret[id] = this->m_participants[id]->toGadgetFormat();
	return ret;
}

/*!
	Convenience function for inserting a new Blip at the end.

	For parameters see the \link Blip::Blip Blip constructor\endlink.

	Note: Fires blipInserted()
*/
Blip * Wavelet::appendBlip(const QByteArray & id, const QString & content, const QList<Element*> & elements, Participant * creator, bool isRoot, const QDateTime & lastModified, int version, bool submitted)
{
	return this->insertBlip(this->m_blips.size(), id, content, elements, creator, isRoot, lastModified, version, submitted);
}

/*!
	Insert a new Blip at the specified index.

	For parameters see the \link Blip::Blip Blip constructor\endlink.

	Note: Fires blipInserted()
*/
Blip * Wavelet::insertBlip(int index, const QByteArray & id, const QString & content, const QList<Element*> & elements, Participant * creator, bool isRoot, const QDateTime & lastModified, int version, bool submitted)
{
	Blip * blip = new Blip(this, id, content, elements, NULL, creator, isRoot, lastModified, version, submitted);
	this->m_blips.insert(index, blip);
	emit blipInserted(index, id);
	return blip;
}

/*!
	Returns the Blip object at the given \a index.
*/
Blip * Wavelet::blipByIndex(int index) const
{
	if (index < 0 || index >= this->m_blips.size())
		return NULL;
	return this->m_blips.at(index);
}

/*!
	Returns the Blip object with the given \a id, if the Blip resides on this
	Wavelet. Returns null otherwise.
*/
Blip * Wavelet::blipById(const QByteArray & id) const
{
	foreach (Blip * blip, this->m_blips) {
		if (blip->id() == id)
			return blip;
	}
	return NULL;
}

/*!
	Returns a list of all Blips on this Wavelet, starting with the root Blip.
*/
QList<Blip*> Wavelet::allBlips() const
{
	return this->m_blips;
}

/*!
	Returns a list of all IDs of the Blips on this Wavelet, starting with
	the root Blip.
*/
QList<QByteArray> Wavelet::allBlipIDs() const
{
	QList<QByteArray> ids;
	foreach (Blip * blip, this->m_blips)
		ids.append(blip->id());
	return ids;
}

/*!
	\internal
	Sets the root Blip of this Wavelet
*/
void Wavelet::setRootBlip(Blip * blip)
{
	this->m_rootBlip = blip;
}

/*!
	\property Wavelet::status
	\brief the status of this Wavelet

	Can be "clean", "dirty" and "invalid".
*/
QByteArray Wavelet::status() const
{
	return this->m_status;
}
void Wavelet::setStatus(const QByteArray & status)
{
	if (this->m_status != status) {
		this->m_status = status;
		emit statusChange(status);
	}
}

/*!
	\property Wavelet::created
	\brief the creation date/time of this Wavelet
*/
QDateTime Wavelet::created() const
{
	return this->m_created;
}

/*!
	\property Wavelet::lastModified
	\brief the date/time of the last modification of this Wavelet
*/
QDateTime Wavelet::lastModified() const
{
	return this->m_lastModified;
}
void Wavelet::setLastModified(const QDateTime &value)
{
	if (this->m_lastModified == value)
		return;
	this->m_lastModified = value;
	emit lastModifiedChanged(value);
}

/*!
	Calculate and compare checksums of all Blips to the given map.

	Fires statusChange() if the status changes.
*/
void Wavelet::checkSync(const QMap<QByteArray, QByteArray> & blipsums)
{
	bool valid = true;

	foreach (QByteArray blipId, blipsums.keys()) {
		Blip * blip = this->blipById(blipId);
		if (blip != NULL && !blip->checkSync(blipsums[blipId]))
			valid = false;
	}

	if (valid)
		this->setStatus("clean");
	else
		this->setStatus("invalid");
}

/*!
	Apply operations on the wavelet.
*/
void Wavelet::applyOperations(const QList<Operation*> & operations, const QMap<QByteArray, Participant*> & participants)
{
	foreach (Operation * op, operations) {
		QVariantMap property;
		if (op->blipId() != "") {
			Blip * blip = this->blipById(op->blipId());
			switch(op->type()) {
				case Operation::DOCUMENT_NOOP:
					break;
				case Operation::DOCUMENT_DELETE:
					blip->deleteText(op->index(), op->property().toInt());
					break;
				case Operation::DOCUMENT_INSERT:
					blip->insertText(op->index(), op->property().toString());
					break;
				case Operation::DOCUMENT_ELEMENT_DELETE:
					blip->deleteElement(op->index());
					break;
				case Operation::DOCUMENT_ELEMENT_INSERT:
					property = op->property().toMap();
					blip->insertElement(op->index(), (Element::Type) property["type"].toInt(), property["properties"].toMap());
					break;
				case Operation::DOCUMENT_ELEMENT_DELTA:
					blip->applyElementDelta(op->index(), op->property().toMap());
					break;
				case Operation::DOCUMENT_ELEMENT_SETPREF:
					property = op->property().toMap();
					blip->setElementUserpref(op->index(), property["key"].toString(), property["value"].toString());
					break;
				default:
					break;
			}
		}
		else {
			switch(op->type()) {
				case Operation::WAVELET_ADD_PARTICIPANT:
					this->addParticipant(participants[op->property().toByteArray()]);
					break;
				case Operation::WAVELET_REMOVE_PARTICIPANT:
					this->removeParticipant(op->property().toByteArray());
					break;
				default:
					break;
			}
		}
	}
}

/*!
	Load the blips from a snapshot.
*/
void Wavelet::loadBlipsFromSnapshot(const QVariantMap &blips, const QByteArray &rootBlipId, const QMap<QByteArray, Participant*> & participants)
{
	foreach (QString s_blip_id, blips.keys()) {
		QByteArray blip_id = s_blip_id.toAscii();
		QVariantMap blip = blips[blip_id].toMap();
		QList<Element*> blip_elements;

		foreach (QVariant element, blip["elements"].toList()) {
			QVariantMap melement = element.toMap();
			QVariantMap properties = melement["properties"].toMap();
			if (melement["type"].toInt() == Element::GADGET)
				blip_elements.append(new GadgetElement(
						NULL,
						melement["id"].toInt(),
						melement["index"].toInt(),
						properties
					));
			else
				blip_elements.append(new Element(
						NULL,
						melement["id"].toInt(),
						melement["index"].toInt(),
						(Element::Type) melement["type"].toInt(),
						properties
					));
		}

		this->appendBlip(
				blip_id,
				blip["content"].toString(),
				blip_elements,
				participants[blip["creator"].toByteArray()],
				blip_id == rootBlipId,
				QDateTime::fromTime_t(blip["lastModifiedTime"].toUInt()),
				blip["version"].toInt(),
				blip["submitted"].toBool()
			);
	}
}


/*!
	\class PyGoWave::Blip
	\brief Models a Blip in a Wavelet.

	This class should not be instanitated directly. Please
	use \link Wavelet::appendBlip() \endlink.
*/

/*!
	\fn Blip::insertedText(int index, const QString & text)

	Fired on text insertion.
*/

/*!
	\fn Blip::deletedText(int index, int length)

	Fired on text deletion.
*/

/*!
	\fn Blip::insertedElement(int index)

	Fired on element insertion.
*/

/*!
	\fn Blip::deletedElement(int index)

	Fired on element deletion.
*/

/*!
	\fn Blip::outOfSync()

	Fired if the Blip has gone out of sync with the server.
*/

/*!
	Creates a new Blip object.

	\a wavelet - Parent Wavelet object

	\a id - ID of the Blip

	\a content - Content of the Blip

	\a elements - Element objects which initially
	reside in this Blip

	\a parent - Parent Blip if this is a nested Blip

	\a creator - Creator of this Blip

	\a isRoot - True if this is the root Blip; if this value
	is set and the parent Wavelet does not have a root Blip yet,
	this Blip is set as the Wavelet's root Blip

	\a lastModified - Date/Time of last modification

	\a version - Version of the Blip

	\a submitted - True if this Blip is submitted
*/
Blip::Blip(Wavelet * wavelet, const QByteArray & id, const QString & content, const QList<Element*> & elements, Blip * parent, Participant * creator, bool isRoot, const QDateTime & lastModified, int version, bool submitted) : QObject(wavelet)
{
	this->m_wavelet = wavelet;
	this->m_id = id;
	this->m_parent = parent;
	this->m_content = content;
	this->m_elements = elements;
	foreach (Element * element, elements)
		element->setBlip(this);
	this->m_creator = creator;
	this->m_root = isRoot;
	this->m_lastModified = lastModified;
	this->m_version = version;
	this->m_submitted = submitted;
	this->m_outofsync = false;
}

/*!
	Destroys the Blip object.
*/
Blip::~Blip() {}

/*!
	\property Blip::id
	\brief the ID of this Blip.
*/
QByteArray Blip::id() const
{
	return this->m_id;
}

/*!
	Returns the parent Wavelet of this Blip.
*/
Wavelet * Blip::wavelet() const
{
	return this->m_wavelet;
}

/*!
	\property Blip::root
	\brief weather this Blip is the Wavelet's root Blip.
*/
bool Blip::isRoot() const
{
	return this->m_root;
}

/*!
	Returns an Element by its \a id.
*/
Element * Blip::elementById(int id) const
{
	foreach (Element * element, this->m_elements) {
		if (element->id() == id)
			return element;
	}
	return NULL;
}

/*!
	Returns the Element object at the given \a index or null.
*/
Element * Blip::elementAt(int index) const
{
	foreach (Element * element, this->m_elements) {
		if (element->position() == index)
			return element;
	}
	return NULL;
}

/*!
	Returns the Elements between the \a start and \a end index.
*/
QList<Element*> Blip::elementsWithin(int start, int end) const
{
	QList<Element*> lst;
	foreach (Element * element, this->m_elements) {
		if (element->position() >= start && element->position() < end)
			lst.append(element);
	}
	return lst;
}

/*!
	Returns all Elements of this Blip.
*/
QList<Element*> Blip::allElements() const
{
	return this->m_elements;
}

/*!
	Insert a \a text at the specified \a index. This moves annotations and
	elements as appropriate.

	Note: This sets the wavelet status to 'dirty'.
	Set \a noevent to true, to prevent the emission of a insertedText() signal.
*/
void Blip::insertText(int index, const QString & text, bool noevent)
{
	this->m_content.insert(index, text);

	int length = text.length();

	foreach (Element * element, this->m_elements) {
		if (element->position() >= index)
			element->setPosition(element->position() + length);
	}

	foreach (Annotation * anno, this->m_annotations) {
		if (anno->start() >= index) {
			anno->setStart(anno->start() + length);
			anno->setEnd(anno->end() + length);
		}
	}

	this->m_wavelet->setStatus("dirty");
	if (!noevent)
		emit insertedText(index, QString(text));
}

/*!
	Delete text at the specified \a index with the given \a length.
	This moves annotations and elements as appropriate.

	Note: This sets the wavelet status to 'dirty'.
	Set \a noevent to true, to prevent the emission of a deletedText() signal.
*/
void Blip::deleteText(int index, int length, bool noevent)
{
	this->m_content.remove(index, length);

	foreach (Element * element, this->m_elements) {
		if (element->position() >= index)
			element->setPosition(element->position() - length);
	}

	foreach (Annotation * anno, this->m_annotations) {
		if (anno->start() >= index) {
			anno->setStart(anno->start() - length);
			anno->setEnd(anno->end() - length);
		}
	}

	this->m_wavelet->setStatus("dirty");
	if (!noevent)
		emit deletedText(index, length);
}

/*!
	Insert an element at the specified \a index. This implicitly adds a
	protected newline character at the index.

	Note: This sets the wavelet status to 'dirty'.
	Set \a noevent to true, to prevent the emission of a insertedElement() signal.
*/
void Blip::insertElement(int index, Element::Type type, const QVariantMap & properties, bool noevent)
{
	this->insertText(index, "\n", true);

	Element * elt;
	if (type == Element::GADGET)
		elt = new GadgetElement(this, -1, index, properties);
	else
		elt = new Element(this, -1, index, type, properties);
	this->m_elements.append(elt);

	this->m_wavelet->setStatus("dirty");
	if (!noevent)
		emit insertedElement(index);
}

/*!
	Delete an element at the specified \a index. This implicitly deletes the
	protected newline character at the index.

	Note: This sets the wavelet status to 'dirty'.
	Set \a noevent to true, to prevent the emission of a deletedElement() signal.
*/
void Blip::deleteElement(int index, bool noevent)
{

	for (int i = 0; i < this->m_elements.length(); i++) {
		Element * elt = this->m_elements.at(i);
		if (elt->position() == index) {
			this->m_elements.takeAt(i);
			this->deleteText(index, 1, true);
			if (!noevent)
				emit deletedElement(index);
			delete elt;
			break;
		}
	}
}

/*!
	Apply an element delta on the element at the specified \a index.
	Currently only for gadget elements.

	Note: This action always emits \link GadgetElement::stateChange \endlink.
*/
void Blip::applyElementDelta(int index, const QVariantMap & delta)
{
	GadgetElement * elt = qobject_cast<GadgetElement*>(this->elementAt(index));
	if (elt != NULL)
		elt->applyDelta(delta);
}

/*!
	Set an UserPref of an element at the specified \a index.
	Currently only for gadget elements.

	Set \a noevent to true, to prevent the emission of a \link GadgetElement::userPrefSet \endlink signal.
*/
void Blip::setElementUserpref(int index, const QString & key, const QString & value, bool noevent)
{
	GadgetElement * elt = qobject_cast<GadgetElement*>(this->elementAt(index));
	if (elt != NULL)
		elt->setUserPref(key, value, noevent);
}

/*!
	\property Blip::content
	\brief the text content of this Blip.
*/
QString Blip::content() const
{
	return this->m_content;
}

/*!
	Calculate a checksum of this Blip and compare it against the given
	checksum. Fires outOfSync() if the checksum is wrong. Returns true if the checksum is ok.

	Note: Currently this only calculates the SHA-1 of the Blip's text. This
	is tentative and subject to change.
*/
bool Blip::checkSync(const QByteArray & sum) {
	QByteArray mysum = QCryptographicHash::hash(this->m_content.toUtf8(), QCryptographicHash::Sha1).toHex();
	if (sum != mysum) {
		emit outOfSync();
		this->m_outofsync = true;
		return false;
	}
	return true;
}


/*!
	\class PyGoWave::Annotation
	\brief Metadata that augments a range of text in a Blip's text.

	Example uses of annotations include styling text, supplying spelling
	corrections, and links to refer that area of text to another Blip or
	web site. The size of an annotation range must be positive and non-zero.
*/

/*!
	Constructs a new Annotation object.

	The annotation's parent Blip, a \a name, \a start and \a end indexes as
	well as the annotation's \a value must be provided.
*/
Annotation::Annotation(Blip * blip, const QString & name, int start, int end, const QString & value) : QObject(blip)
{
	this->m_blip = blip;
	this->m_name = name;
	this->m_start = start;
	this->m_end = end;
	this->m_value = value;
}

/*!
	Destroys the annotation.
*/
Annotation::~Annotation() {}

/*!
	Returns the Blip on which this element resides.
*/
Blip * Annotation::blip() const
{
	return this->m_blip;
}

/*!
	\property Annotation::name
	\brief the name of this annotation.
*/
QString Annotation::name() const
{
	return this->m_name;
}

/*!
	\property Annotation::start
	\brief the start index of this annotation.
*/
int Annotation::start() const
{
	return this->m_start;
}
void Annotation::setStart(int index)
{
	this->m_start = index;
}

/*!
	\property Annotation::end
	\brief the end index of this annotation.
*/
int Annotation::end() const
{
	return this->m_end;
}
void Annotation::setEnd(int index)
{
	this->m_end = index;
}

/*!
	\property Annotation::value
	\brief the value of this annotation.
*/
QString Annotation::value()
{
	return this->m_value;
}


/*!
	\class PyGoWave::Element
	\brief Element-objects are all the non-text elements in a Blip.

	An element has no physical presence in the text of a Blip, but it maintains
	an implicit protected newline character to keep positions distinct.

	Only special Wave Client elements are treated here.
	There are no HTML elements in any Blip. All markup is handled by Annotations.
*/

/*!
	\enum PyGoWave::Element::Type
	\brief This enum describes the type of the element.
*/
/*! \var PyGoWave::Element::Type Element::NOTHING
	Invalid element */
/*! \var PyGoWave::Element::Type Element::INLINE_BLIP
	Blip within a Blip */
/*! \var PyGoWave::Element::Type Element::GADGET
	GadgetElement */
/*! \var PyGoWave::Element::Type Element::INPUT
	FormElement (text input) */
/*! \var PyGoWave::Element::Type Element::CHECK
	FormElement (checkbox) */
/*! \var PyGoWave::Element::Type Element::LABEL
	FormElement (label) */
/*! \var PyGoWave::Element::Type Element::BUTTON
	FormElement (push button) */
/*! \var PyGoWave::Element::Type Element::RADIO_BUTTON
	FormElement (radio button) */
/*! \var PyGoWave::Element::Type Element::RADIO_BUTTON_GROUP
	FormElement (radio button group) */
/*! \var PyGoWave::Element::Type Element::PASSWORD
	FormElement (password input) */
/*! \var PyGoWave::Element::Type Element::IMAGE
	ImageElement */

/*!
	Constructs an Element object.

	If the \a id is set to -1, a new temporary ID will be assigned to the Element.
	The \a position is the character position within the text, an Element usually
	takes a newline character as placeholder.
*/
Element::Element(Blip * blip, int id, int position, Element::Type type, const QVariantMap & properties) : QObject(blip)
{
	this->m_blip = blip;
	if (id < 0)
		this->m_id = Element::newTempId();
	else
		this->m_id = id;
	this->m_pos = position;
	this->m_type = type;
	this->m_properties = properties;
}

/*!
	Destroys the Element.
*/
Element::~Element() {}

/*!
	Returns the Blip on which this element resides.

	\sa setBlip
*/
Blip * Element::blip() const
{
	return this->m_blip;
}

/*!
	Sets the Blip on which this element resides. Implies reparenting.
*/
void Element::setBlip(Blip * blip)
{
	this->setParent(blip);
	this->m_blip = blip;
}

/*!
	\property Element::id
	\brief the ID of the element.

	A negative ID represents a temporary ID,
	that is only valid for this session and will be replaced by a real ID
	on reload.
*/
int Element::id() const
{
	return this->m_id;
}

/*!
	\property Element::type
	\brief the type of the element.
*/
Element::Type Element::type() const
{
	return this->m_type;
}

/*!
	\property Element::position
	\brief the index where this element resides
*/
int Element::position() const
{
	return this->m_pos;
}
void Element::setPosition(int pos)
{
	this->m_pos = pos;
}

int Element::g_lastTempId = 0;

/*!
	Creates a new temporary ID.
*/
int Element::newTempId()
{
	Element::g_lastTempId--;
	return Element::g_lastTempId;
}

/*!
	\class PyGoWave::GadgetElement
	\brief A gadget element.
*/

/*!
	\fn void GadgetElement::stateChange()
	\brief Emitted on changes to the gadget state
*/

/*!
	\fn void GadgetElement::userPrefSet(const QString & key, const QString & value)
	\brief Emitted if a UserPref has been set
*/

/*!
	Constructs a new GadgetElement
*/
GadgetElement::GadgetElement(Blip * blip, int id, int position, const QVariantMap & properties)
		: Element(blip, id, position, Element::GADGET, properties) {}

/*!
	Destroys the GadgetElement
*/
GadgetElement::~GadgetElement() {}

/*!
	\property GadgetElement::fields
	\brief the gadget's state object i.e. the field map.
*/
QVariantMap GadgetElement::fields() const
{
	if (this->m_properties.contains("fields"))
		return this->m_properties["fields"].toMap();
	else
		return QVariantMap();
}

/*!
	\property GadgetElement::userPrefs
	\brief all UserPrefs.
*/
QVariantMap GadgetElement::userPrefs() const
{
	if (this->m_properties.contains("userprefs"))
		return this->m_properties["userprefs"].toMap();
	else
		return QVariantMap();
}

/*!
	\property GadgetElement::url
	\brief the gadget xml URL.
*/
QString GadgetElement::url() const
{
	if (this->m_properties.contains("url"))
		return this->m_properties["url"].toString();
	else
		return QString();
}

/*!
	Apply a delta to this gadget's state.

	Merges \a delta into the gadget state and emits stateChange()
*/
void GadgetElement::applyDelta(const QVariantMap & delta)
{
	QVariantMap fields = this->fields();

	foreach (QString key, delta.keys()) {
		QVariant data = delta[key];
		if (data.isNull()) {
			if (fields.contains(key))
				fields.remove(key);
		}
		else
			fields[key] = data;
	}

	this->m_properties["fields"] = fields;
	emit stateChange();
}

/*!
	Set a UserPref by \a key and \a value. Set \a noevent to true
	if userPrefSet() should not be emitted.
*/
void GadgetElement::setUserPref(const QString & key, const QString & value, bool noevent)
{
	QVariantMap userprefs = this->userPrefs();
	userprefs[key] = value;
	this->m_properties["userprefs"] = userprefs;
	if (!noevent)
		emit userPrefSet(key, value);
}
