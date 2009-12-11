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

#include "operations.h"
#include "operations_p.h"

using namespace PyGoWave;

/*!
	\class PyGoWave::Operation
	\brief Represents a generic operation applied on the server.

	This operation class contains data that is filled in depending on the
	operation type.

	It can be used directly, but doing so will not result
	in local, transient reflection of state on the blips. In other words,
	creating a "delete blip" operation will not remove the blip from the local
	context for the duration of this session. It is better to use the OpBased
	model classes directly instead.
*/

/*!
	Constructs this operation with contextual data.

	\param op_type Type of operation
	\param waveId The id of the wave that this operation is to
	be applied.
	\param waveletId The id of the wavelet that this operation is
	to be applied.
	\param blipId The optional id of the Blip that this
	operation is to be applied.
	\param index Optional integer index for content-based
	operations.
	\param prop A weakly typed property object is based
	on the context of this operation.
*/
Operation::Operation(Operation::Type op_type, const QByteArray & waveId, const QByteArray & waveletId, const QByteArray & blipId, int index, QVariant prop) : pd_ptr(new OperationPrivate)
{
	P_D(Operation);
	d->m_type = op_type;
	d->m_waveId = waveId;
	d->m_waveletId = waveletId;
	d->m_blipId = blipId;
	d->m_index = index;
	d->m_property = prop;
}

/*!
	Create a copy of this operation.
*/
Operation::Operation(const Operation & other) : pd_ptr(new OperationPrivate)
{
	P_D(Operation);
	const OperationPrivate * other_pd = other.pd_func();
	d->m_type = other_pd->m_type;
	d->m_waveId = other_pd->m_waveId;
	d->m_waveletId = other_pd->m_waveletId;
	d->m_blipId = other_pd->m_blipId;
	d->m_index = other_pd->m_index;
	d->m_property = other_pd->m_property;
}

Operation::~Operation()
{
	delete this->pd_ptr;
}

Operation::Type Operation::type() const
{
	const P_D(Operation);
	return d->m_type;
}

QByteArray Operation::waveId() const
{
	const P_D(Operation);
	return d->m_waveId;
}

QByteArray Operation::waveletId() const
{
	const P_D(Operation);
	return d->m_waveletId;
}

QByteArray Operation::blipId() const
{
	const P_D(Operation);
	return d->m_blipId;
}

void Operation::setBlipId(const QByteArray &id)
{
	P_D(Operation);
	d->m_blipId = id;
}

int Operation::index() const
{
	const P_D(Operation);
	return d->m_index;
}
void Operation::setIndex(int value)
{
	P_D(Operation);
	d->m_index = value;
}

QVariant Operation::property() const
{
	const P_D(Operation);
	return d->m_property;
}
void Operation::setProperty(const QVariant & property)
{
	P_D(Operation);
	d->m_property = property;
}

Operation * Operation::clone() const
{
	return new Operation(*this);
}

/*!
	Return weather this operation is a null operation i.e. it does not
	change anything.
*/
bool Operation::isNull() const
{
	const P_D(Operation);
	if (d->m_type == Operation::DOCUMENT_INSERT)
		return this->length() == 0;
	else if (d->m_type == Operation::DOCUMENT_DELETE)
		return d->m_property.toInt() == 0;
	return false;
}

/*!
	Returns if this operation is compatible to another one i.e. it could potentially influence
	the other operation.
*/
bool Operation::isCompatibleTo(const Operation & other) const
{
	return this->isCompatibleTo(&other);
}

/*!
	\overload
*/
bool Operation::isCompatibleTo(const Operation * other) const
{
	const P_D(Operation);
	const OperationPrivate * other_pd = other->pd_func();
	if (d->m_waveId != other_pd->m_waveId || d->m_waveletId != other_pd->m_waveletId || d->m_blipId != other_pd->m_blipId)
		return false;
	return true;
}

/*!
	Returns true, if this op is an insertion operation.
*/
bool Operation::isInsert() const
{
	const P_D(Operation);
	return d->m_type == Operation::DOCUMENT_INSERT || d->m_type == Operation::DOCUMENT_ELEMENT_INSERT;
}

/*!
	Returns true, if this op is a deletion operation.
*/
bool Operation::isDelete() const
{
	const P_D(Operation);
	return d->m_type == Operation::DOCUMENT_DELETE || d->m_type == Operation::DOCUMENT_ELEMENT_DELETE;
}

/*!
	Returns true, if this op is an (attribute) change operation.
*/
bool Operation::isChange() const
{
	const P_D(Operation);
	return d->m_type == Operation::DOCUMENT_ELEMENT_DELTA || d->m_type == Operation::DOCUMENT_ELEMENT_SETPREF;
}

/*!
	Returns the length of this operation.
	This can be interpreted as the distance a concurrent operation's index
	must be moved to include the effects of this operation.
*/
int Operation::length() const
{
	const P_D(Operation);
	if (d->m_type == Operation::DOCUMENT_INSERT)
		return d->m_property.toString().length();
	else if (d->m_type == Operation::DOCUMENT_DELETE)
		return d->m_property.toInt();
	else if (d->m_type == Operation::DOCUMENT_ELEMENT_INSERT || d->m_type == Operation::DOCUMENT_ELEMENT_DELETE)
		return 1;
	return 0;
}

/*!
	Delete operations: Sets the amount of deleted characters/elements to
	\a value.

	Other operations: No effect.
*/
void Operation::resize(int value)
{
	P_D(Operation);
	if (d->m_type == Operation::DOCUMENT_DELETE)
		d->m_property = (value > 0 ? value : 0);
}

/*!
	DOCUMENT_INSERT: Inserts the string \a s into the property at
	the specified \a position.

	Other operations: No effect.
*/
void Operation::insertString(int pos, const QString & s)
{
	P_D(Operation);
	if (d->m_type == Operation::DOCUMENT_INSERT)
		d->m_property = d->m_property.toString().insert(pos, s);
}

/*!
	DOCUMENT_INSERT: Deletes a substring at \a pos with the specified
	\a length from the property.

	Other operations: No effect.
*/
void Operation::deleteString(int pos, int length)
{
	P_D(Operation);
	if (d->m_type == Operation::DOCUMENT_INSERT)
		d->m_property = d->m_property.toString().remove(pos, length);
}

/*!
	Serialize this operation into a dictionary. Official robots API format.
*/
QVariantMap Operation::serialize() const
{
	const P_D(Operation);
	QVariantMap ret;
	ret["type"] = OperationPrivate::typeToString(d->m_type);
	ret["waveId"] = QString::fromAscii(d->m_waveId);
	ret["waveletId"] = QString::fromAscii(d->m_waveletId);
	ret["blipId"] = QString::fromAscii(d->m_blipId);
	ret["index"] = d->m_index;
	if (d->m_property.isValid())
		ret["property"] = d->m_property;
	else
		ret["property"] = 0;
	return ret;
}

/*!
	Unserialize an operation from a dictionary.
*/
Operation * Operation::unserialize(const QVariantMap & obj)
{
	return new Operation(
			OperationPrivate::typeFromString(obj["type"].toString()),
			obj["waveId"].toByteArray(),
			obj["waveletId"].toByteArray(),
			obj["blipId"].toByteArray(),
			obj["index"].toInt(),
			obj["property"]
		);
}

QString OperationPrivate::typeToString(Operation::Type type)
{
	switch (type) {
		case Operation::DOCUMENT_NOOP:
			return "DOCUMENT_NOOP";
		case Operation::DOCUMENT_INSERT:
			return "DOCUMENT_INSERT";
		case Operation::DOCUMENT_DELETE:
			return "DOCUMENT_DELETE";
		case Operation::DOCUMENT_ELEMENT_INSERT:
			return "DOCUMENT_ELEMENT_INSERT";
		case Operation::DOCUMENT_ELEMENT_DELETE:
			return "DOCUMENT_ELEMENT_DELETE";
		case Operation::DOCUMENT_ELEMENT_DELTA:
			return "DOCUMENT_ELEMENT_DELTA";
		case Operation::DOCUMENT_ELEMENT_SETPREF:
			return "DOCUMENT_ELEMENT_SETPREF";
		case Operation::WAVELET_ADD_PARTICIPANT:
			return "WAVELET_ADD_PARTICIPANT";
		case Operation::WAVELET_REMOVE_PARTICIPANT:
			return "WAVELET_REMOVE_PARTICIPANT";
		case Operation::WAVELET_APPEND_BLIP:
			return "WAVELET_APPEND_BLIP";
		case Operation::BLIP_CREATE_CHILD:
			return "BLIP_CREATE_CHILD";
		case Operation::BLIP_DELETE:
			return "BLIP_DELETE";
	}
	return "DOCUMENT_NOOP";
}

Operation::Type OperationPrivate::typeFromString(const QString & type)
{
	if (type == "DOCUMENT_INSERT")
		return Operation::DOCUMENT_INSERT;
	else if (type == "DOCUMENT_DELETE")
		return Operation::DOCUMENT_DELETE;
	else if (type == "DOCUMENT_ELEMENT_INSERT")
		return Operation::DOCUMENT_ELEMENT_INSERT;
	else if (type == "DOCUMENT_ELEMENT_DELETE")
		return Operation::DOCUMENT_ELEMENT_DELETE;
	else if (type == "DOCUMENT_ELEMENT_DELTA")
		return Operation::DOCUMENT_ELEMENT_DELTA;
	else if (type == "DOCUMENT_ELEMENT_SETPREF")
		return Operation::DOCUMENT_ELEMENT_SETPREF;
	else if (type == "WAVELET_ADD_PARTICIPANT")
		return Operation::WAVELET_ADD_PARTICIPANT;
	else if (type == "WAVELET_REMOVE_PARTICIPANT")
		return Operation::WAVELET_REMOVE_PARTICIPANT;
	else if (type == "WAVELET_APPEND_BLIP")
		return Operation::WAVELET_APPEND_BLIP;
	else if (type == "BLIP_CREATE_CHILD")
		return Operation::BLIP_CREATE_CHILD;
	else if (type == "BLIP_DELETE")
		return Operation::BLIP_DELETE;
	return Operation::DOCUMENT_NOOP;
}

/*!
	\class PyGoWave::OpManager
	\brief The Operation Manager, core of PyGoWave's OT implementation
*/

/*!
	\fn void OpManager::operationChanged(int index)
	\brief Fired if an operation in this manager has been changed.

	\param index Index of the changed operation
*/

/*!
	\fn void OpManager::beforeOperationsRemoved(int start, int end)
	\brief Fired if one or more operations are about to be removed.

	\param start Start index of the removal.
	\param end End index of the removal.
*/

/*!
	\fn void OpManager::afterOperationsRemoved(int start, int end)
	\brief Fired if one or more operations have been removed.

	\param start Start index of the removal.
	\param end End index of the removal.
*/

/*!
	\fn void OpManager::beforeOperationsInserted(int start, int end)
	\brief Fired if one or more operations are about to be inserted.

	\param start Start index of the insertion.
	\param end End index of the insertion.
*/

/*!
	\fn void OpManager::afterOperationsInserted(int start, int end)
	\brief Fired if one or more operations have been inserted.

	\param start Start index of the insertion.
	\param end End index of the insertion.
*/

/*!
	Constructs the op manager with a \a waveId and \a waveletId.
*/
OpManager::OpManager(
		const QByteArray & waveId,
		const QByteArray & waveletId,
		const QByteArray & contributorId,
		QObject * parent
	) : QObject(parent), pd_ptr(new OpManagerPrivate(this))
{
	P_D(OpManager);
	d->m_waveId = waveId;
	d->m_waveletId = waveletId;
	d->m_contributorId = contributorId;
}

OpManager::~OpManager()
{
	P_D(OpManager);
	while (!d->m_operations.isEmpty())
		delete d->m_operations.takeLast();
	delete this->pd_ptr;
}

/*!
	Return true if this manager is not holding operations.
*/
bool OpManager::isEmpty() const
{
	const P_D(OpManager);
	return d->m_operations.isEmpty();
}

/*!
	Returns if the manager holds fetchable operations i.e. that are not
	locked.
*/
bool OpManager::canFetch() const
{
	const P_D(OpManager);
	foreach (Operation * op, d->m_operations) {
		if (!d->m_lockedBlips.contains(op->blipId()))
			return true;
	}
	return false;
}

QByteArray OpManager::waveId() const
{
	const P_D(OpManager);
	return d->m_waveId;
}

QByteArray OpManager::waveletId() const
{
	const P_D(OpManager);
	return d->m_waveletId;
}

/*!
	Transform the input operation on behalf of the manager's operations
	list. This will simultaneously transform the operations list on behalf
	of the input operation.

	This method returns a list of applicable operations. This list may be
	empty or it may contain any number of new operations (according to
	results of deletion, modification and splitting; i.e. the input
	operation is not modified by itself).
*/
QList<Operation*> OpManager::transform(Operation * input_op)
{
	P_D(OpManager);
	Operation * new_op = NULL;
	QList<Operation*> op_lst;
	op_lst.append(input_op->clone());
	int i = 0;
	while (i < d->m_operations.size()) {
		Operation * myop = d->m_operations[i];
		int j = 0;
		while (j < op_lst.size()) {
			Operation * op = op_lst[j];
			if (!op->isCompatibleTo(myop))
				continue;
			int end = 0;
			if (op->isDelete() && myop->isDelete()) {
				if (op->index() < myop->index()) {
					end = op->index() + op->length();
					if (end <= myop->index()) {
						myop->setIndex(myop->index() - op->length());
						emit operationChanged(i);
					}
					else if (end < (myop->index() + myop->length())) {
						op->resize(myop->index() - op->index());
						myop->resize(myop->length() - (end - myop->index()));
						myop->setIndex(op->index());
						emit operationChanged(i);
					}
					else {
						op->resize(op->length() - myop->length());
						myop = NULL;
						this->removeOperation(i);
						i--;
						break;
					}
				}
				else {
					end = myop->index() + myop->length();
					if (op->index() >= end)
						op->setIndex(op->index() - myop->length());
					else if (op->index() + op->length() <= end) {
						myop->resize(myop->length() - op->length());
						delete op_lst.takeAt(j);
						op = NULL;
						j--;
						if (myop->isNull()) {
							myop = NULL;
							this->removeOperation(i);
							i--;
							break;
						}
						else
							emit operationChanged(i);
					}
					else {
						myop->resize(myop->length() - (end - op->index()));
						emit operationChanged(i);
						op->resize(op->length() - (end - op->index()));
						op->setIndex(myop->index());
					}
				}
			}
			else if (op->isDelete() && myop->isInsert()) {
				if (op->index() < myop->index()) {
					if (op->index() + op->length() <= myop->index()) {
						myop->setIndex(myop->index() - op->length());
						emit operationChanged(i);
					}
					else {
						new_op = op->clone();
						op->resize(myop->index() - op->index());
						new_op->resize(new_op->length() - op->length());
						op_lst.insert(j + 1, new_op);
						myop->setIndex(myop->index() - op->length());
						emit operationChanged(i);
					}
				}
				else
					op->setIndex(op->index() + myop->length());
			}
			else if (op->isInsert() && myop->isDelete()) {
				if (op->index() <= myop->index()) {
					myop->setIndex(myop->index() + op->length());
					emit operationChanged(i);
				}
				else if (op->index() >= (myop->index() + myop->length()))
					op->setIndex(op->index() - myop->length());
				else {
					new_op = myop->clone();
					myop->resize(op->index() - myop->index());
					emit operationChanged(i);
					new_op->resize(new_op->length() - myop->length());
					this->insertOperation(i + 1, new_op);
					op->setIndex(myop->index());
				}
			}
			else if (op->isInsert() && myop->isInsert()) {
				if (op->index() <= myop->index()) {
					myop->setIndex(myop->index() + op->length());
					emit operationChanged(i);
				}
				else
					op->setIndex(op->index() + myop->length());
			}
			else if (op->isChange() && myop->isDelete()) {
				if (op->index() > myop->index()) {
					if (op->index() <= (myop->index() + myop->length()))
						op->setIndex(myop->index());
					else
						op->setIndex(op->index() - myop->length());
				}
			}
			else if (op->isChange() && myop->isInsert()) {
				if (op->index() >= myop->index())
					op->setIndex(op->index() + myop->length());
			}
			else if (op->isDelete() && myop->isChange()) {
				if (op->index() < myop->index()) {
					if (myop->index() <= (op->index() + op->length())) {
						myop->setIndex(op->index());
						emit operationChanged(i);
					}
					else {
						myop->setIndex(myop->index() - op->length());
						emit operationChanged(i);
					}
				}
			}
			else if (op->isInsert() && myop->isChange()) {
				if (op->index() <= myop->index()) {
					myop->setIndex(myop->index() + op->length());
					emit operationChanged(i);
				}
			}
			else if ((op->type() == Operation::WAVELET_ADD_PARTICIPANT && myop->type() == Operation::WAVELET_ADD_PARTICIPANT)
					|| (op->type() == Operation::WAVELET_REMOVE_PARTICIPANT && myop->type() == Operation::WAVELET_REMOVE_PARTICIPANT)) {
				if (op->property() == myop->property()) {
					myop = NULL;
					this->removeOperation(i);
					i--;
					break;
				}
			}
			else if (op->type() == Operation::BLIP_DELETE && !op->blipId().isEmpty() && !myop->blipId().isEmpty()) {
				myop = NULL;
				this->removeOperation(i);
				i--;
				break;
			}
			j++;
		}
		i++;
	}
	return op_lst;
}

/*!
	Returns the pending operations and removes them from this manager.
*/
QList<Operation*> OpManager::fetch()
{
	P_D(OpManager);
	QList<Operation*> ops;
	int i = 0, s = 0;
	while (i < d->m_operations.size()) {
		Operation * op = d->m_operations.at(i);
		if (d->m_lockedBlips.contains(op->blipId())) {
			if (i - s > 0) {
				d->removeOperations(s, i-1, false);
				i -= s+1;
			}
			s = i+1;
		}
		else
			ops.append(op);
		i++;
	}
	if (i - s > 0)
		d->removeOperations(s, i-1, false);

	return ops;
}

/*!
	Opposite of fetch. Inserts all given operations into this manager.

	\sa put operations
*/
void OpManager::put(const QList<Operation*> & ops)
{
	P_D(OpManager);
	if (ops.size() == 0)
		return;
	int start = d->m_operations.size();
	int end = start + ops.size() - 1;
	emit beforeOperationsInserted(start, end);
	d->m_operations.append(ops);
	emit afterOperationsInserted(start, end);
}

/*!
	Serialize this manager's operations into a list of dictionaries.
	Set fetch to true to also clear this manager.

	\sa unserialize
*/
QVariantList OpManager::serialize(bool fetch)
{
	P_D(OpManager);
	QList<Operation*> ops;
	if (fetch)
		ops = this->fetch();
	else
		ops = d->m_operations;

	QVariantList out;
	Operation * op;
	foreach (op, ops)
		out.append(op->serialize());

	if (fetch) {
		while (!ops.isEmpty())
			delete ops.takeLast();
	}
	return out;
}

/*!
	Unserialize a list of dictionaries to operations and add them to this
	manager.

	\sa serialize
*/
void OpManager::unserialize(const QVariantList & serial_ops) {
	QList<Operation*> ops;
	foreach (QVariant op, serial_ops)
		ops.append(Operation::unserialize(op.toMap()));
	this->put(ops);
}

/*!
	Returns all operations without deleting them from the manager.

	\sa fetch put
*/
QList<Operation*> OpManager::operations() const
{
	const P_D(OpManager);
	return d->m_operations;
}

/*!
	Requests to insert content into a document at a specific location.

	\param blipId The Blip id that this operation is applied to
	\param index The position insert the content at in ths document
	\param content The content to insert
*/
void OpManager::documentInsert(const QByteArray & blipId, int index, const QString & content)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::DOCUMENT_INSERT,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			index,
			content
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to delete content in a given range.

	\param blipId The Blip id that this operation is applied to
	\param start Start of the range
	\param end End of the range
*/
void OpManager::documentDelete(const QByteArray & blipId, int start, int end)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::DOCUMENT_DELETE,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			start,
			end-start
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to insert an element at the given position.

	\param blipId The Blip id that this operation is applied to
	\param index Position of the new element
	\param type Element type
	\param properties Element properties
*/
void OpManager::documentElementInsert(const QByteArray & blipId, int index, int type, const QVariantMap & properties)
{
	P_D(OpManager);
	QVariantMap property;
	property["type"] = type;
	property["properties"] = properties;
	Operation * op = new Operation(
			Operation::DOCUMENT_ELEMENT_INSERT,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			index,
			property
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to delete an element from the given position.

	\param blipId The Blip id that this operation is applied to
	\param index Position of the element to delete
*/
void OpManager::documentElementDelete(const QByteArray & blipId, int index)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::DOCUMENT_ELEMENT_DELETE,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			index,
			QVariant()
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to apply a delta to the element at the given position.

	\param blipId The Blip id that this operation is applied to
	\param index Position of the element
	\param delta Delta to apply to the element
*/
void OpManager::documentElementDelta(const QByteArray & blipId, int index, const QVariantMap & delta)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::DOCUMENT_ELEMENT_DELTA,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			index,
			delta
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to set a UserPref of the element at the given position.

	\param blipId The Blip id that this operation is applied to
	\param index Position of the element
	\param key Name of the UserPref
	\param value Value of the UserPref
*/
void OpManager::documentElementSetpref(const QByteArray & blipId, int index, const QString & key, const QString & value)
{
	P_D(OpManager);
	QVariantMap property;
	property["key"] = key;
	property["value"] = value;
	Operation * op = new Operation(
			Operation::DOCUMENT_ELEMENT_SETPREF,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			index,
			property
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to add a Participant to the Wavelet.

	\param id ID of the Participant to add
*/
void OpManager::waveletAddParticipant(const QByteArray &id)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::WAVELET_ADD_PARTICIPANT,
			d->m_waveId,
			d->m_waveletId,
			QByteArray(),
			-1,
			QString::fromAscii(id)
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to remove a Participant to the Wavelet.

	\param id ID of the Participant to remove
*/
void OpManager::waveletRemoveParticipant(const QByteArray &id)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::WAVELET_REMOVE_PARTICIPANT,
			d->m_waveId,
			d->m_waveletId,
			QByteArray(),
			-1,
			QString::fromAscii(id)
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to append a new Blip to the Wavelet.

	\param tempId Temporary Blip ID
*/
void OpManager::waveletAppendBlip(const QByteArray &tempId)
{
	P_D(OpManager);
	QVariantMap property;
	property["waveId"] = QString::fromAscii(d->m_waveId);
	property["waveletId"] = QString::fromAscii(d->m_waveletId);
	property["blipId"] = QString::fromAscii(tempId);
	Operation * op = new Operation(
			Operation::WAVELET_APPEND_BLIP,
			d->m_waveId,
			d->m_waveletId,
			QByteArray(),
			-1,
			property
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to delete a Blip.

	\param blipId The Blip id that this operation is applied to
*/
void OpManager::blipDelete(const QByteArray &blipId)
{
	P_D(OpManager);
	Operation * op = new Operation(
			Operation::BLIP_DELETE,
			d->m_waveId,
			d->m_waveletId,
			blipId
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	Requests to create a clild Blip.

	\param blipId The parent Blip
	\param tempId Temporary Blip ID
*/
void OpManager::blipCreateChild(const QByteArray &blipId, const QByteArray &tempId)
{
	P_D(OpManager);
	QVariantMap property;
	property["waveId"] = QString::fromAscii(d->m_waveId);
	property["waveletId"] = QString::fromAscii(d->m_waveletId);
	property["blipId"] = QString::fromAscii(tempId);
	Operation * op = new Operation(
			Operation::BLIP_CREATE_CHILD,
			d->m_waveId,
			d->m_waveletId,
			blipId,
			-1,
			property
		);
	if (!d->mergeInsert(op))
		delete op;
}

/*!
	\internal
	Inserts and probably merges an operation into the manager's
	operation list.
*/
bool OpManagerPrivate::mergeInsert(Operation * newop)
{
	P_Q(OpManager);
	Operation * op = NULL;
	int i = 0;
	if (newop->type() == Operation::DOCUMENT_ELEMENT_DELTA) {
		for (i = 0; i < this->m_operations.size(); i++) {
			op = this->m_operations[i];
			QVariantMap dmap = op->property().toMap();
			if (op->type() == Operation::DOCUMENT_ELEMENT_DELTA && newop->property().toMap()["id"] == dmap["id"]) {
				QVariantMap delta = dmap["delta"].toMap();
				QVariantMap newdelta = newop->property().toMap()["delta"].toMap();
				foreach (QString key, newdelta.keys())
					delta[key] = newdelta[key];
				dmap["delta"] = delta;
				op->setProperty(dmap);
				emit q->operationChanged(i);
				return false;
			}
		}
	}
	i = this->m_operations.size() - 1;
	if (i >= 0) {
		op = this->m_operations[i];
		if (newop->type() == Operation::DOCUMENT_INSERT && op->type() == Operation::DOCUMENT_INSERT) {
			if (newop->index() >= op->index() && newop->index() <= op->index()+op->length()) {
				op->insertString(newop->index() - op->index(), newop->property().toString());
				emit q->operationChanged(i);
				return false;
			}
		}
		else if (newop->type() == Operation::DOCUMENT_DELETE && op->type() == Operation::DOCUMENT_INSERT) {
			if (newop->index() >= op->index() && newop->index() < op->index()+op->length()) {
				int remain = op->length() - (newop->index() - op->index());
				if (remain > newop->length()) {
					op->deleteString(newop->index() - op->index(), newop->length());
					newop->resize(0);
				}
				else {
					op->deleteString(newop->index() - op->index(), remain);
					newop->resize(newop->length() - remain);
				}
				if (op->isNull()) {
					q->removeOperation(i);
					i--;
				}
				else
					emit q->operationChanged(i);
				if (newop->isNull())
					return false;
			}
			else if (newop->index() < op->index() && newop->index()+newop->length() > op->index()) {
				if (newop->index()+newop->length() >= op->index()+op->length()) {
					newop->resize(newop->length() - op->length());
					q->removeOperation(i);
					i--;
				}
				else {
					int dlength = newop->index()+newop->length() - op->index();
					newop->resize(newop->length() - dlength);
					op->deleteString(0, dlength);
					emit q->operationChanged(i);
				}
			}
		}
		else if (newop->type() == Operation::DOCUMENT_DELETE && op->type() == Operation::DOCUMENT_DELETE) {
			if (newop->index() == op->index()) {
				op->resize(op->length() + newop->length());
				emit q->operationChanged(i);
				return false;
			}
			if (newop->index() == (op->index() - newop->length())) {
				op->setIndex(op->index() - newop->length());
				op->resize(op->length() + newop->length());
				emit q->operationChanged(i);
				return false;
			}
		}
		else if ((newop->type() == Operation::WAVELET_ADD_PARTICIPANT && op->type() == Operation::WAVELET_ADD_PARTICIPANT)
				|| (newop->type() == Operation::WAVELET_REMOVE_PARTICIPANT && op->type() == Operation::WAVELET_REMOVE_PARTICIPANT)) {
			if (newop->property() == op->property())
				return false;
		}
	}
	q->insertOperation(i + 1, newop);
	return true;
}

/*!
	Inserts an operation at the specified index.
	Fires signals appropriately.

	\param index Position in operations list
	\param op Operation object to insert
*/
void OpManager::insertOperation(int index, Operation * op)
{
	P_D(OpManager);
	if (index > d->m_operations.size() || index < 0)
		return;
	emit beforeOperationsInserted(index, index);
	d->m_operations.insert(index, op);
	emit afterOperationsInserted(index, index);
}

/*!
	Removes the operation at the specified index.
	Fires signals appropriately.

	\param index Position in operations list
*/
void OpManager::removeOperation(int index)
{
	P_D(OpManager);
	if (index < 0 || index >= d->m_operations.size())
		return;
	emit beforeOperationsRemoved(index, index);
	delete d->m_operations.takeAt(index);
	emit afterOperationsRemoved(index, index);
}

/*!
	Removes operations between and including the given start and end
	indexes. Fires signals appropriately.
*/
void OpManager::removeOperations(int start, int end)
{
	P_D(OpManager);
	d->removeOperations(start, end, true);
}

void OpManagerPrivate::removeOperations(int start, int end, bool delete_obj)
{
	P_Q(OpManager);
	if (start < 0 || end < 0 || start > end || start >= this->m_operations.size() || end >= this->m_operations.size())
		return;
	emit q->beforeOperationsRemoved(start, end);
	for (int index = start; start <= end; start++) {
		Operation * op = this->m_operations.takeAt(index);
		if (delete_obj)
			delete op;
	}
	emit q->afterOperationsRemoved(start, end);
}

/*!
	Updates the ID of operations on temporary Blips.
*/
void OpManager::updateBlipId(const QByteArray &tempId, const QByteArray &blipId)
{
	P_D(OpManager);
	for (int i = 0; i < d->m_operations.size(); i++) {
		Operation * op = d->m_operations.at(i);
		if (op->blipId() == tempId) {
			op->setBlipId(blipId);
			emit operationChanged(i);
		}
	}
}

/*!
	Prevents the Operations on the Blip with the given ID from being
	handed over via fetch().
*/
void OpManager::lockBlipOps(const QByteArray &blipId)
{
	P_D(OpManager);
	if (!d->m_lockedBlips.contains(blipId))
		d->m_lockedBlips.append(blipId);
}

/*!
	Allows the Operations on the Blip with the given ID from being
	handed over via fetch().
*/
void OpManager::unlockBlipOps(const QByteArray &blipId)
{
	P_D(OpManager);
	d->m_lockedBlips.removeAll(blipId);
}
