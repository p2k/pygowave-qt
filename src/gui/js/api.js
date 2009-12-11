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

window.pygowave = $defined(window.pygowave) ? window.pygowave : new Hash();

pygowave.api = $defined(pygowave.api) ? pygowave.api : new Hash();
(function () {
	/* Imports */
	var BlipContainerWidget = pygowave.view.BlipContainerWidget;
	var CppWrapper = jswrapper.CppWrapper;

	var Participant = new Class({
		Extends: CppWrapper,
		
		id: function () {
			return this._wrapper.objectId;
		},
		displayName: function () {
			return this._cpp.displayName;
		},
		thumbnailUrl: function () {
			return this._cpp.thumbnailUrl;
		},
		profileUrl: function () {
			return this._cpp.profileUrl;
		},
		isBot: function () {
			return this._cpp.bot;
		},
		isOnline: function () {
			return this._cpp.online;
		},
		toGadgetFormat: function () {
			return this._cpp.gadgetFormat;
		}
	});
	Participant.className = "PyGoWave::Participant";
	Participant.get = function (id) {
		return jswrapper.objects.get(Participant, id);
	};

	var WaveModel = new Class({
		Extends: CppWrapper,
		
		id: function () {
			return this._wrapper.objectId;
		},
		viewerId: function () {
			return this._wrapper.viewerId();
		},
		createWavelet: function (id, options) {
			// Unsupported
		},
		wavelet: function (waveletId) {
			return this.getChild(Wavelet, waveletId);
		},
		allWavelets: function () {
			return this.getChildren(Wavelet, this._wrapper.allWavelets());
		},
		rootWavelet: function () {
			return this.getChild(Wavelet, this._wrapper.rootWavelet());
		}
	});
	WaveModel.className = "PyGoWave::WaveModel";

	var Wavelet = new Class({
		Extends: CppWrapper,
		
		title: function () {
			return this._cpp.title;
		},
		isRoot: function () {
			return this._cpp.root;
		},
		id: function () {
			return this._wrapper.objectId;
		},
		waveId: function () {
			return this._parent.id();
		},
		waveModel: function () {
			return this._parent;
		},
		participant: function (id) {
			if (this._wrapper.hasParticipant(id))
				return Participant.get(id);
			else
				return null;
		},
		allParticipants: function () {
			return Participant.get(this._wrapper.allParticipants());
		},
		allParticipantIDs: function () {
			return this._wrapper.allParticipants();
		},
		allParticipantsForGadget: function () {
			return this._cpp.allParticipantsForGadget;
		},
		//TODO
		appendBlip: function (id, options, content, elements) {

		},
		//TODO
		insertBlip: function (index, id, options, content, elements) {

		},
		blipByIndex: function (index) {
			return this.getChild(Blip, this._wrapper.blipByIndex(index));
		},
		blipById: function (id) {
			return this.getChild(Blip, id);
		},
		allBlips: function () {
			this.getChildren(Blip, this._wrapper.allBlips());
		},
		allBlipIDs: function () {
			return this._wrapper.allBlips();
		},
		created: function () {
			return this._cpp.created;
		},
		lastModified: function () {
			return this._cpp.lastModified;
		},
		setLastModified: function (value) {
			this._cpp.lastModified = value;
		},
	});
	Wavelet.className = "PyGoWave::Wavelet";

	var Blip = new Class({
		Extends: CppWrapper,
		
		id: function () {
			return this._wrapper.objectId;
		},
		wavelet: function () {
			return this._parent;
		},
		isRoot: function () {
			return this._cpp.root;
		},
		creator: function () {
			return Participant.get(this._wrapper.creator());
		},
		elementAt: function (index) {
			return this.getChild(GadgetElement, this._wrapper.elementAt(index));
		},
		elementsWithin: function (start, end) {
			return this.getChildren(GadgetElement, this._wrapper.elementsWithin(start, end));
		},
		allElements: function () {
			return this.getChildren(GadgetElement, this._wrapper.allElements());
		},
		allContributors: function () {
			var ret = Participant.get(this._wrapper.allContributors());
			return ret;
		},
		//TODO?
		insertText: function (index, text, noevent) {

		},
		//TODO?
		deleteText: function (index, length, noevent) {

		},
		//TODO?
		insertElement: function (index, type, properties, noevent) {

		},
		//TODO?
		deleteElement: function (index, noevent) {

		},
		//TODO?
		applyElementDelta: function (index, delta) {

		},
		//TODO?
		setElementUserpref: function (index, key, value, noevent) {

		},
		content: function () {
			return this._cpp.content;
		},
		lastModified: function () {
			return this._cpp.lastModified;
		},
		setLastModified: function (value) {
			this._cpp.lastModified = value;
		}
	});
	Blip.className = "PyGoWave::Blip";

	var Element = new Class({
		Extends: CppWrapper,
		
		blip: function () {
			return this._parent;
		},
		setBlip: function (blip) {
			// Unsupported
		},
		id: function () {
			return this._cpp.id;
		},
		type: function ()  {
			return this._cpp.type;
		},
		position: function () {
			return this._cpp.position;
		}
	});

	var GadgetElement = new Class({
		Extends: Element,
		
		fields: function () {
			return this._cpp.fields;
		},
		userPrefs: function () {
			return this._cpp.userPrefs;
		},
		url: function () {
			return this._cpp.url;
		},
		applyDelta: function (delta) {
			this._cpp.applyDelta(delta);
		},
		setUserPref: function (key, value, noevent) {
			this._cpp.setUserPref(key, value, noevent);
		}
	});
	GadgetElement.className = "PyGoWave::GadgetElement";

	var WaveView = new Class({
		Implements: [Options, Events],
		options: {
			gadgetLoaderUrl: "about:blank",
			defaultThumbnailUrl: "about:blank",
			rtl: false
		},

		initialize: function (container, waveId, waveletId, options) {
			this.setOptions(options);
			this.container = container;
			this._busyCounter = 0;

			// Connect view
			this.addEvent("textInserted", pygowave.api.cpp.textInserted);
			this.addEvent("textDeleted", pygowave.api.cpp.textDeleted);
			this.addEvent("elementDelete", pygowave.api.cpp.elementDelete);
			this.addEvent("elementDeltaSubmitted", pygowave.api.cpp.elementDeltaSubmitted);
			this.addEvent("elementSetUserpref", pygowave.api.cpp.elementSetUserpref);
			this.addEvent("deleteBlip", pygowave.api.cpp.deleteBlip);
			this.addEvent("draftBlip", pygowave.api.cpp.draftBlip);

			this._wave = jswrapper.objects.get(WaveModel, waveId);
			this._wavelet = this._wave.wavelet(waveletId);

			this._wavelet.addEvent('blipInserted', this._onBlipInserted.bind(this));
			this._wavelet.addEvent('blipDeleted', this._onBlipDeleted.bind(this));

			this._blipContainerWidget = new BlipContainerWidget(this, container);

			this._activeBlipEditor = null;
			this._blipEditors = new Hash();
			this._onCurrentTextRangeChanged = this._onCurrentTextRangeChanged.bind(this);
			this._onBlipEditing = this._onBlipEditing.bind(this);
			this._onBlipDone = this._onBlipDone.bind(this);
			this._onBlipIdChanged = this._onBlipIdChanged.bind(this);

			var blipIds = this._wavelet.allBlipIDs();
			for (var i = 0; i < blipIds.length; i++)
				this._onBlipInserted(i, blipIds[i]);
		},

		_onBlipInserted: function (index, blip_id) {
			var blip = this._wavelet.blipByIndex(index);
			var editor = this._blipContainerWidget.insertBlip(index, blip);
			this._blipEditors.set(blip_id, editor);
			editor.addEvents({
				blipEditing: this._onBlipEditing,
				blipDone: this._onBlipDone,
				deleteBlip: this._onDeleteBlip
			});
			blip.addEvent('idChanged', this._onBlipIdChanged);
			if (blip.id().startswith("TBD_"))
				editor.editBlip();
		},
		_onBlipDeleted: function (blipId) {
			var editor = this._blipEditors.get(blipId);
			if (!$defined(editor))
				return;
			if (editor == this._activeBlipEditor)
				this._activeBlipEditor.finishBlip();
			this._blipEditors.erase(blipId);
			editor.removeEvents({
				blipEditing: this._onBlipEditing,
				blipDone: this._onBlipDone,
				deleteBlip: this._onDeleteBlip
			});
			editor.blip().removeEvent('idChanged', this._onBlipIdChanged);
			this._blipContainerWidget.deleteBlip(blipId);
		},
		_onBlipIdChanged: function (oldId, newId) {
			var editor = this._blipEditors.get(oldId);
			if (!$defined(editor))
				return;
			this._blipEditors.erase(oldId);
			this._blipEditors.set(newId, editor);
		},
		_onBlipEditing: function (blipId) {
			var editor = this._blipEditors.get(blipId);
			if (this._activeBlipEditor == editor)
				return;
			if (this._activeBlipEditor != null)
				this._activeBlipEditor.finishBlip();
			this._activeBlipEditor = editor;
			editor.addEvent("currentTextRangeChanged", this._onCurrentTextRangeChanged);
			pygowave.api.cpp.emitBlipEditing(true);
		},
		_onBlipDone: function (blipId) {
			var editor = this._blipEditors.get(blipId);
			this._activeBlipEditor = null;
			pygowave.api.cpp.emitBlipEditing(false);
			this._onCurrentTextRangeChanged(-1, -1);
			editor.removeEvent("currentTextRangeChanged", this._onCurrentTextRangeChanged);
		},
		_onCurrentTextRangeChanged: function (start, end) {
			if (start == null)
				start = -1;
			if (end == null)
				end = -1;
			pygowave.api.cpp.emitCurrentTextRangeChanged(start, end);
		},

		defaultThumbnailUrl: function () {
			return this.options.defaultThumbnailUrl;
		},

		showMessage: function (message, title, buttonLabel) {
			alert("WaveView::showMessage - not implemented");
		},

		showQuestion: function (message, title, yesCallback, noCallback, yesLabel, noLabel) {
			alert("WaveView::showQuestion - not implemented");
		},
		//TODO
		insertGadgetAtCursor: function (waveletId, url) {
			return false;
		},

		isBusy: function () {
			return (this._busyCounter != 0);
		},

		setBusy: function () {
			this._busyCounter++;
		},

		unsetBusy: function () {
			this._busyCounter--;
			if (this._busyCounter < 0)
				this._busyCounter = 0;
			if (this._busyCounter == 0)
				this.fireEvent('ready');
		}
	});

	var setup = function (cpp_obj, waveId, waveletId, options) {
		window.console.info("PyGoWaveJSAPI: Setup started...");
		pygowave.api.extend({
			cpp: cpp_obj
		});
		window.__wave_view__ = new WaveView(document.body, waveId, waveletId, options);
		window.console.info("PyGoWaveJSAPI: Setup completed");
	};

	pygowave.api.extend({
		setup: setup
	});
})();
