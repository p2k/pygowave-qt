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
 * Classes and functions for handling gadgets.
 * 
 * @module pygowave.view.gadgets
 */
(function () {
	/* Imports */
	var Widget = pygowave.view.Widget;

	/**
	 * Widget for managing a GadgetElement. This also is a RPC handler.
	 *
	 * @class {private} pygowave.view.GadgetElementWidget
	 * @extends pygowave.view.Widget
	 */
	var GadgetElementWidget = new Class({
		Extends: Widget,
		
		/**
		 * Fired when the user clicks on the delete button.
		 * @event onDeleteClicked
		 * @param {int} index Index of the element
		 */
		
		/**
		 * Called on instantiation.
		 * @constructor {public} initialize
		 * @param {WaveView} view A reference back to the main view
		 * @param {pygowave.model.GadgetElement} gadgetElement GadgetElement to be rendered
		 * @param {Element} parentElement Parent DOM element to insert the widget
		 * @param {String} where Where to inject the widget relative to the
		 *        parent element. Can be 'top', 'bottom', 'after', or 'before'.
		 */
		initialize: function (view, gadgetElement, parentElement, where) {
			this._onParticipantsChanged = this._onParticipantsChanged.bind(this);
			this._onStateChange = this._onStateChange.bind(this);
			this._onSetUserPref = this._onSetUserPref.bind(this);
			
			this._view = view;
			this._gadgetElement = gadgetElement;
			var blip = gadgetElement.blip();
			this._wavelet = blip.wavelet();
			this._blipId = blip.id();
			this._viewerId = this._wavelet.waveModel().viewerId();
			
			this._callbacksOnLoad = new Array();
			this._callbacksRegistered = new Hash();
			
			window.gadget_rpc.registerModuleHandler(gadgetElement.id(), this);
			this._wavelet.addEvent('participantsChanged', this._onParticipantsChanged);
			this._gadgetElement.addEvents({
				'stateChange': this._onStateChange,
				'setUserPref': this._onSetUserPref
			});
			
			var contentElement = new Element('div', {'class': 'gadget_element'});
			contentElement.contentEditable = "false";
			
			this._deleteBox = new Element('div', {
				'class': 'delete_box' + (Browser.Engine.presto ? '_opera' : ''),
				'title': gettext("Delete Gadget")
			}).inject(contentElement);
			this._deleteBox.addEvent('click', function () {
				this.fireEvent('deleteClicked', this._gadgetElement.position());
			}.bind(this));
			
			var src = "%surl=%s&gadget_id=%d".sprintf(view.options.gadgetLoaderUrl, encodeURIComponent(gadgetElement.url()), gadgetElement.id());
			
			dbgprint(src);
			
			this._gadgetFrame = new Element('iframe', {
				'src': src
			}).inject(contentElement);
			this._ready = false;
			view.addEvent('loadFinished', this._onGadgetLoaded.bind(this));
			dbgprint("test1");
			
			this.parent(parentElement, contentElement, where);
		},
		/**
		 * Overridden from {@link pygowave.view.Widget.dispose Widget.dispose}.<br/>
		 * Removes the widget from the DOM, sets the parentElement to null and
		 * disconnects from the Wavelet and GadgetElement.
		 * 
		 * @function {public Widget} dispose
		 * @return Returns a reference to this widget.
		 */
		dispose: function () {
			this.parent();
			this._wavelet.removeEvent('participantsChanged', this._onParticipantsChanged);
			this._gadgetElement.removeEvents({
				'stateChange': this._onStateChange,
				'setUserPref': this._onSetUserPref
			});
			window.gadget_rpc.unregisterModuleHandler(this._gadgetElement.id());
			this._callbacksOnLoad.empty();
			this._callbacksRegistered.empty();
			return this;
		},
		/**
		 * Returns the GadgetElement which is rendered by this widget.
		 * 
		 * @function {public pygowave.model.GadgetElement} element
		 */
		element: function () {
			return this._gadgetElement;
		},
		/**
		 * Returns the logical position within the Blip text of the rendered
		 * GadgetElement.
		 * 
		 * @function {public int} position
		 */
		position: function () {
			return this._gadgetElement.position();
		},
		
		/**
		 * Callback from iframe on load. Invokes onLoad callbacks of the
		 * rendered Gadget.
		 *
		 * @function {private} _onGadgetLoaded
		 */
		_onGadgetLoaded: function () {
			dbgprint("test2");
			if (this._ready)
				return;
			
			dbgprint(this._gadgetFrame.contentDocument.innerHtml);
			
			for (var it = new _Iterator(this._callbacksOnLoad); it.hasNext(); ) {
				var callback = it.next();
				dbgprint("Calling back");
				callback();
			}
			this._callbacksOnLoad.empty();
			this._onParticipantsChanged();
			this._onStateChange();
			this.adjustHeight();
			this._ready = true;
		},
		/**
		 * Callback from wavelet model on participants change. Forwards to the
		 * rendered Gadget.
		 *
		 * @function {private} _onParticipantsChanged
		 */
		_onParticipantsChanged: function () {
			this.invokeCallbacks(
				"wave_participants",
				{
					myId: this._viewerId,
					authorId: 0,
					participants: this._wavelet.allParticipantsForGadget()
				}
			);
		},
		/**
		 * Callback from model on state change. Forwards to the rendered Gadget.
		 *
		 * @function {private} _onStateChange
		 */
		_onStateChange: function () {
			this.invokeCallbacks("wave_gadget_state", this._gadgetElement.fields());
		},
		/**
		 * Callback from model on userpref setting. Forwards to the rendered Gadget.
		 *
		 * @function {private} _onSetUserPref
		 * @param {String} key
		 * @param {String} value
		 */
		_onSetUserPref: function (key, value) {
			this.invokeCallbacks("set_pref", ["unknown", key, value]);
		},
		/**
		 * Invoke registered callbacks of the rendered Gadget.
		 *
		 * @function {public} invokeCallbacks
		 * @param {String} serviceName Name of the service to invoke
		 * @param {Object} var_args List of arguments to pass
		 */
		invokeCallbacks: function (serviceName, var_args) {
			if (!this._callbacksRegistered.has(serviceName))
				return;
			for (var it = new _Iterator(this._callbacksRegistered.get(serviceName)); it.hasNext(); ) {
				var callback = it.next();
				var takethis = {a: var_args};
				callback.call(takethis, var_args);
			}
		},
		
		/**
		 * Callback from GadgetRPCHandler. Reacts to "wave_gadget_state" and
		 * "wave_log" calls.
		 *
		 * @function {private} call
		 * @param {int} targetId ID of the target gadget or null for the parent
		 * @param {String} serviceName Name of the service to call
		 * @param {optional Function} callback Callback function if applicable
		 * @param {optional Object} var_args Arguments of the call
		 */
		call: function (targetId, serviceName, callback, var_args) {
			if (targetId == null) {
				switch (serviceName) {
					case "wave_gadget_state":
						this._view.fireEvent('elementDeltaSubmitted', [this._wavelet.id(), this._blipId, this._gadgetElement.position(), var_args]);
						break;
					case "wave_log":
						dbgprint(gettext("Gadget #%d at index %d says:").sprintf(this._gadgetElement.id(), this._gadgetElement.position()), var_args);
						break;
					case "wave_enable":
						//TODO?
						break;
				}
			}
		},
		/**
		 * Callback from GadgetRPCHandler. Registers an arbitrary service
		 * handler, but only "wave_participants", "wave_gadget_state" and
		 * "set_pref" will actually be called.
		 * 
		 * @function {private} register
		 * @param {String} serviceName Name of the service to register
		 * @param {Function} handler Function to be called for the service
		 */
		register: function (serviceName, handler) {
			if (!this._callbacksRegistered.has(serviceName))
				this._callbacksRegistered.set(serviceName, new Array());
			this._callbacksRegistered.get(serviceName).push(handler);
		},
		/**
		 * Callback from GadgetRPCHandler. Registers a callback to be invoked
		 * on Gadget load.
		 *
		 * @function {private} registerOnLoadHandler
		 * @param {Function} callback Function to be called on gadget load
		 */
		registerOnLoadHandler: function (callback) {
			this._callbacksOnLoad.push(callback);
		},
		/**
		 * Callback from GadgetRPCHandler. Adjusts the height of the container
		 * iframe.
		 *
		 * @function {private} adjustHeight
		 * @param {optional int} opt_height Height to be set. If missing tries
		 *        to calculate the optimal value from the document height.
		 */
		adjustHeight: function (opt_height) {
			if ($defined(opt_height))
				this._gadgetFrame.setStyle('height', opt_height);
			else {
				try { // On unexpected errors in the gadget loader, this may fail
					var doc = this._gadgetFrame.contentDocument.documentElement;
					this._gadgetFrame.setStyle('height', doc.documentElement.scrollHeight);
				}catch(e){}
			}
		},
		/**
		 * Callback from GadgetRPCHandler. Sets a UserPref value.
		 *
		 * @function {private} set_pref
		 * @param {int} moduleId ID of the module that invoked this callback
		 * @param {String} key
		 * @param {String} value
		 */
		set_pref: function (key, value) {
			this._view.fireEvent('elementSetUserpref', [this._wavelet.id(), this._blipId, this._gadgetElement.position(), key, value]);
		}
	});
	
	/**
	 * Singleton class for handling Gadget RPC calls.
	 * Acts as a proxy between the actual gadget in the iframe and the
	 * associated GadgetElementWidget.
	 *
	 * @class {private} GadgetRPCHandler
	 */
	var GadgetRPCHandler = new Class({
		Implements: Events,
		
		/**
		 * Called on instantiation.
		 * @constructor {public} initialize
		 */
		initialize: function () {
			this._modules = new Hash();
		},
		
		/**
		 * Registers a new module handler with the given ID. The handler must
		 * support all callback functions (call, register, registerOnLoadHandler,
		 * adjustHeight, set_pref) which will be invoked from the underlying
		 * Gadget.
		 *
		 * @function {public} registerModuleHandler
		 * @function {int} moduleId ID of the module to handle
		 * @function {Object} handler Module handler object which receives calls
		 */
		registerModuleHandler: function (moduleId, handler) {
			this._modules.set(moduleId, handler);
		},
		/**
		 * Unregisters an existing module handler with the given ID.
		 *
		 * @function {public} unregisterModuleHandler
		 * @param {int} moduleId ID of the module
		 */
		unregisterModuleHandler: function (moduleId) {
			this._modules.erase(moduleId);
		},
		
		/**
		 * Callback from modules (i.e. Gadgets) to trigger a specific function.
		 * Forwards to the associated module handler.
		 *
		 * @function {private} call
		 * @param {int} moduleId ID of the module that invoked this callback
		 * @param {int} targetId ID of the target gadget or null for the parent
		 * @param {String} serviceName Name of the service to call
		 * @param {optional Function} callback Callback function if applicable
		 * @param {optional Object} var_args Arguments of the call
		 */
		call: function (moduleId, targetId, serviceName, callback, var_args) {
			if (this._modules.has(moduleId))
				this._modules[moduleId].call(targetId, serviceName, callback, var_args);
		},
		/**
		 * Callback from modules (i.e. Gadgets) to register a service callback.
		 * Forwards to the associated module handler.
		 *
		 * @function {private} register
		 * @param {int} moduleId ID of the module that invoked this callback
		 * @param {String} serviceName Name of the service to register
		 * @param {Function} handler Function to be called for the service
		 */
		register: function (moduleId, serviceName, handler) {
			if (this._modules.has(moduleId))
				this._modules[moduleId].register(serviceName, handler);
		},
		/**
		 * Callback from modules (i.e. Gadgets) to register a callback which
		 * is invoked if the module was loaded.
		 * Forwards to the associated module handler.
		 *
		 * @function {private} registerOnLoadHandler
		 * @param {int} moduleId ID of the module that invoked this callback
		 * @param {Function} callback Function to be called on module load
		 */
		registerOnLoadHandler: function (moduleId, callback) {
			if (this._modules.has(moduleId))
				this._modules[moduleId].registerOnLoadHandler(callback);
		},
		/**
		 * Callback from modules (i.e. Gadgets) to issue an hight adjustment
		 * of the container frame.
		 * Forwards to the associated module handler.
		 *
		 * @function {private} adjustHeight
		 * @param {int} moduleId ID of the module that invoked this callback
		 * @param {optional int} opt_height Height to be set. If missing tries
		 *        to calculate the optimal value from the document height.
		 */
		adjustHeight: function (moduleId, opt_height) {
			if (this._modules.has(moduleId))
				this._modules[moduleId].adjustHeight(opt_height);
		},
		/**
		 * Callback from modules (i.e. Gadgets) to set a UserPref value.
		 * Forwards to the associated module handler.
		 *
		 * @function {private} set_pref
		 * @param {int} moduleId ID of the module that invoked this callback
		 * @param {String} key
		 * @param {String} value
		 */
		set_pref: function (moduleId, key, value) {
			if (this._modules.has(moduleId))
				this._modules[moduleId].set_pref(key, value);
		}
	});
	
	window.gadget_rpc = new GadgetRPCHandler();
	
	pygowave.view.extend({
		GadgetElementWidget: GadgetElementWidget
	});
})();
