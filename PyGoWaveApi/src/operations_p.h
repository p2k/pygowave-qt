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

#ifndef OPERATIONS_P_H
#define OPERATIONS_P_H

#include "pygowave_api_global.h"

namespace PyGoWave {

	class OperationPrivate
	{
	public:
		Operation::Type m_type;
		QByteArray m_waveId;
		QByteArray m_waveletId;
		QByteArray m_blipId;
		int m_index;
		QVariant m_property;

		static QString typeToString(Operation::Type type);
		static Operation::Type typeFromString(const QString & type);
	};

	class OpManagerPrivate
	{
		P_DECLARE_PUBLIC(OpManager)
	public:
		OpManagerPrivate(OpManager * q) : pq_ptr(q) {}

		QByteArray m_waveId;
		QByteArray m_waveletId;
		QByteArray m_contributorId;
		QList<Operation*> m_operations;
		QList<QByteArray> m_lockedBlips;

		bool mergeInsert(Operation * newop);
		void removeOperations(int start, int end, bool delete_obj);

	private:
		OpManager * const pq_ptr;
	};
}

#endif // OPERATIONS_P_H
