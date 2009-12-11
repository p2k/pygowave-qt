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

/**@scope pygowave*/
window.pygowave = $defined(window.pygowave) ? window.pygowave : new Hash(); 

pygowave.view = $defined(pygowave.view) ? pygowave.view : new Hash();

/**
 * Widgets for displaying and adding participants.
 * 
 * @module pygowave.view.participants
 */
(function () {
	/* Imports */
	var Widget = pygowave.view.Widget;
	
	/**
	 * A widget to display a participant. This includes his/her avatar and a
	 * online indicator; may also display the participants nickname if rendered
	 * as searchbox widget.
	 *
	 * @class {private} pygowave.view.ParticipantWidget
	 * @extends pygowave.view.Widget
	 */
	var ParticipantWidget = new Class({
		Extends: Widget,
		
		// --- Event documentation ---
		/**
		 * Fired if clicked.
		 * @event onClick
		 * @param {ParticipantWidget} widget The clicked widget (for convenience)
		 */
		// ---------------------------
		
		/**
		 * Called on instantiation.
		 * @constructor {public} initialize
		 * @param {WaveView} view A reference back to the main view
		 * @param {Element} parentElement Parent DOM element to insert the widget
		 * @param {pygowave.model.Participant} participant Participant object
		 * @param {String} display_style Set the display style of the widget.
		 *        Possible values: 'normal', 'small', 'searchbox'.
		 * @param {String} where Where to inject the widget relative to the
		 *        parent element. Can be 'top', 'bottom', 'after', or 'before'.
		 */
		initialize: function (view, parentElement, participant, display_style, where) {
			this._view = view;
			this._participant = participant;
			var contentElement = new Element('div', {'class': 'wavelet_participant'});
			this._pImage = new Element('img', {
				'class': 'thumbnail',
				'src': '',
				'alt': '',
				'title': ''
			}).inject(contentElement);
			if (display_style == 'small') {
				contentElement.addClass('small');
				this._pNick = null;
			}
			else if (display_style == 'searchbox') {
				contentElement.addClass('searchbox');
				this._pNick = new Element('div', {
					'class': 'nicktext'
				}).inject(contentElement);
			}
			contentElement.addEvent('click', this._onClick.bind(this));
			this._onOnlineStateChanged = this._onOnlineStateChanged.bind(this);
			this._onDataChanged = this._onDataChanged.bind(this);
			this.parent(parentElement, contentElement, where);
		},
		
		/**
		 * Returns the displayed participant object.
		 * @function {public pygowave.model.Participant} participant
		 */
		participant: function () {
			return this._participant;
		},
		
		/**
		 * Overridden from {@link pygowave.view.Widget.dispose Widget.dispose}.<br/>
		 * Removes the widget from the DOM, sets the parentElement to null
		 * and removes event handlers.
		 * @function {public Widget} dispose
		 */
		dispose: function () {
			this.parent();
			this._participant.removeEvent('onlineStateChanged', this._onOnlineStateChanged);
			this._participant.removeEvent('dataChanged', this._onDataChanged);
			return this;
		},
		
		/**
		 * Overridden from {@link pygowave.view.Widget.inject Widget.inject}.<br/>
		 * Inject this widget somewhere else. Sets the parent element and adds
		 * event listeners to the (new) participant object.
		 * 
		 * @function {public Widget} inject
		 * @param {Element} parentElement New parent element
		 * @param {optional String} where Where to inject the contentElement
		 *        relative to the parent element. Can be 'top', 'bottom',
		 *        'after', or 'before'.
		 * @param {optional pygowave.model.Participant} participant New
		 *        Participant object to bind this element to.
		 * @return Returns a reference to this widget.
		 */
		inject: function (parentElement, where, participant) {
			this.parent(parentElement, where);
			
			if ($defined(participant))
				this._participant = participant;
			else
				participant = this._participant;
				
			participant.addEvent('onlineStateChanged', this._onOnlineStateChanged);
			participant.addEvent('dataChanged', this._onDataChanged);
			
			var display_name = participant.displayName();
			if (display_name == "") display_name = "...";
			
			var tn = participant.thumbnailUrl();
			if (tn == "") tn = this._view.defaultThumbnailUrl();
			
			this._pImage.set({
				'src': tn,
				'alt': display_name,
				'title': display_name
			});
			
			if (this._pNick != null)
				this._pNick.set('text', display_name);
			this._onOnlineStateChanged(participant.isOnline());
			return this;
		},
		
		/**
		 * Callback for {@link pygowave.model.Participant.onOnlineStateChanged
		 * onOnlineStateChanged}. Shows/hides the online indicator.<br/>
		 * Note: The behaviour depends on your stylesheet. This method toggles
		 * between the 'offline' and 'online' classes of the element.
		 * @function {private} _onOnlineStateChanged
		 * @param {Boolean} online True if the participant is online
		 */
		_onOnlineStateChanged: function (online) {
			if (!$defined(this._oImage))
				this._oImage = new Element('div', {'class': 'indicator'}).inject(this.contentElement, 'bottom');
			
			this._oImage.addClass(online ? 'online' : 'offline').removeClass(online ? 'offline' : 'online');
		},
		
		/**
		 * Callback for {@link pygowave.model.Participant.onDataChanged
		 * onDataChanged}. Updates the representation of the participant.
		 * @function {private} _onDataChanged
		 */
		_onDataChanged: function () {
			var pImage = this._pImage;
			var display_name = this._participant.displayName();
			if (display_name == "") display_name = "...";
			pImage.set({
				"alt": display_name,
				"title": display_name
			});
			if (this._pNick != null)
				this._pNick.set('text', display_name);
			
			var tn = this._participant.thumbnailUrl();
			if (tn == "") tn = this._view.defaultThumbnailUrl();
			
			if (pImage.get("src") != tn) { // Exchange thumbnail
				new Fx.Tween(pImage, {duration:200}).start("opacity", 1, 0).chain(function () {
					pImage.set("src", "");
					pImage.set.delay(100, pImage, ["src", tn]);
					this.start.delay(100, this ,["opacity", 0, 1]);
				});
			}
		},
		
		/**
		 * Callback on click. Forwards event to listeners.
		 * @function _onClick
		 */
		_onClick: function () {
			this.fireEvent('click', this);
		}
	});
	
	pygowave.view.extend({
		ParticipantWidget: ParticipantWidget
	});
})();
