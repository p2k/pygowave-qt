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

#ifndef MODEL_P_H
#define MODEL_P_H

namespace PyGoWave {

	class ParticipantPrivate
	{
	public:
		QByteArray m_id;
		QString m_displayName;
		QString m_thumbnailUrl;
		QString m_profileUrl;
		bool m_online;
		bool m_bot;
	};

	class AnnotationPrivate
	{
	public:
		Blip * m_blip;
		QString m_name;
		int m_start;
		int m_end;
		QString m_value;
	};

	class ElementPrivate
	{
	public:
		Blip * m_blip;
		int m_id;
		int m_pos;
		Element::Type m_type;

		QVariantMap m_properties;
		static int newTempId();

		static int g_lastTempId;
	};

	class GadgetElementPrivate : public ElementPrivate
	{
	};

	class WaveModelPrivate
	{
	public:
		Wavelet * m_rootWavelet;
		QByteArray m_waveId;
		QByteArray m_viewerId;
		QMap< QByteArray, Wavelet* > m_wavelets;
		IParticipantProvider * m_pp;
	};

	class WaveletPrivate
	{
	public:
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

	class BlipPrivate
	{
	public:
		Wavelet * m_wavelet;
		QByteArray m_id;
		QString m_content;
		QList<Element*> m_elements;
		Blip * m_parent;
		Participant * m_creator;
		QMap<QByteArray, Participant*> m_contributors;
		bool m_root;
		QDateTime m_lastModified;
		int m_version;
		bool m_submitted;
		bool m_outofsync;
		QList<Annotation*> m_annotations;

		static QByteArray newTempId();

		static int g_lastTempId;
	};
}

#endif // MODEL_P_H
