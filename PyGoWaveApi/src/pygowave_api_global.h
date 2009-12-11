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

#ifndef PYGOWAVE_API_GLOBAL_H
#define PYGOWAVE_API_GLOBAL_H

#include <QtCore/qglobal.h>

#ifndef P_DECLARE_PRIVATE
#define P_DECLARE_PRIVATE(Class) \
	inline Class##Private* pd_func() { return reinterpret_cast<Class##Private *>(this->pd_ptr); } \
	inline const Class##Private* pd_func() const { return reinterpret_cast<const Class##Private *>(this->pd_ptr); } \
	friend class Class##Private;
#endif

#ifndef P_DECLARE_PUBLIC
#define P_DECLARE_PUBLIC(Class) \
	inline Class* pq_func() { return static_cast<Class *>(this->pq_ptr); } \
	inline const Class* pq_func() const { return static_cast<const Class *>(this->pq_ptr); } \
	friend class Class;
#endif

#ifndef P_D
#define P_D(Class) Class##Private * const d = this->pd_func()
#endif
#ifndef P_Q
#define P_Q(Class) Class * const q = this->pq_func()
#endif

#if defined(PYGOWAVE_API_LIBRARY)
#  define PYGOWAVE_API_SHARED_EXPORT Q_DECL_EXPORT
#  define PYGOWAVE_API_P_INCLUDE
#else
#  define PYGOWAVE_API_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PYGOWAVE_API_GLOBAL_H
