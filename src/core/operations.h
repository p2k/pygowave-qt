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

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QList>

namespace PyGoWave {

	class Operation
	{
	public:
		enum Type {
			DOCUMENT_NOOP = 0,
			DOCUMENT_INSERT,
			DOCUMENT_DELETE,
			DOCUMENT_ELEMENT_INSERT,
			DOCUMENT_ELEMENT_DELETE,
			DOCUMENT_ELEMENT_DELTA,
			DOCUMENT_ELEMENT_SETPREF,
			WAVELET_ADD_PARTICIPANT,
			WAVELET_REMOVE_PARTICIPANT
		};

		Operation(Type op_type, const QByteArray & waveId, const QByteArray & waveletId, const QByteArray & blipId = QByteArray(), int index = -1, QVariant prop = QVariant());

		Operation(const Operation & other);

		Type type();
		QByteArray waveId();
		QByteArray waveletId();
		QByteArray blipId();

		int index();
		void setIndex(int value);
		QVariant property();
		void setProperty(const QVariant & property);

		Operation * clone() const;

		bool isNull() const;

		bool isCompatibleTo(const Operation & other) const;
		bool isCompatibleTo(const Operation * other) const;

		bool isInsert() const;
		bool isDelete() const;
		bool isChange() const;

		int length() const;

		void resize(int value);

		void insertString(int pos, const QString & s);
		void deleteString(int pos, int length);

		QVariantMap serialize() const;
		static Operation * unserialize(const QVariantMap & obj);

	private:
		Type m_type;
		QByteArray m_waveId;
		QByteArray m_waveletId;
		QByteArray m_blipId;
		int m_index;
		QVariant m_property;

		static QString typeToString(Type type);
		static Type typeFromString(const QString & type);
	};

	class OpManager : public QObject {
		Q_OBJECT

	public:
		OpManager(const QByteArray & waveId, const QByteArray & waveletId, QObject * parent = NULL);
		~OpManager();

		bool isEmpty() const;

		QByteArray waveId();

		QByteArray waveletId();

		QList<Operation*> transform(Operation * input_op);

		QList<Operation*> fetch();
		void put(const QList<Operation*> & ops);

		QVariantList serialize(bool fetch = false);
		void unserialize(const QVariantList & serial_ops);

		void documentInsert(const QByteArray & blipId, int index, const QString & content);
		void documentDelete(const QByteArray & blipId, int start, int end);

		void documentElementInsert(const QByteArray & blipId, int index, int type, const QVariantMap & properties);
		void documentElementDelete(const QByteArray & blipId, int index);

		void documentElementDelta(const QByteArray & blipId, int index, const QVariantMap & delta);
		void documentElementSetpref(const QByteArray & blipId, int index, const QString & key, const QString & value);

		void waveletAddParticipant(const QByteArray &id);
		void waveletRemoveParticipant(const QByteArray &id);

		QList<Operation*> operations();

		void insertOperation(int index, Operation * op);
		void removeOperation(int index);

	signals:
		void operationChanged(int index);
		void beforeOperationsRemoved(int start, int end);
		void afterOperationsRemoved(int start, int end);
		void beforeOperationsInserted(int start, int end);
		void afterOperationsInserted(int start, int end);

	private:
		QByteArray m_waveId;
		QByteArray m_waveletId;
		QList<Operation*> m_operations;

		bool __insert(Operation * newop);
	};
}

#endif // OPERATIONS_H
