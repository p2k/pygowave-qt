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

gadgets = {
	_urlParameters: null,
	_prefs: {},
	_country: "us",
	_lang: "en"
};

window.__MODULE_ID__ = null;

gadgets.rpc = {
	call: function (targetId, serviceName, callback, var_args) {
		if (window.__MODULE_ID__ != null)
			window.parent.gadget_rpc.call(window.__MODULE_ID__, targetId, serviceName, callback, var_args);
	},
	register: function (serviceName, handler) {
		if (window.__MODULE_ID__ != null)
			window.parent.gadget_rpc.register(window.__MODULE_ID__, serviceName, handler);
	}
};

gadgets.util = {
	getUrlParameters: function () {
		return gadgets._urlParameters;
	},
	registerOnLoadHandler: function(callback) {
		window.parent.gadget_rpc.registerOnLoadHandler(window.__MODULE_ID__, callback);
	}
};

gadgets.window = {
	adjustHeight: function (opt_height) {
		window.parent.gadget_rpc.adjustHeight(window.__MODULE_ID__, opt_height);
	},
	getViewportDimensions: function () {
		var w = 0, h = 0;
		try {
			w = window.innerWidth;
			h = window.innerHeight;
		}
		catch (e) {
			w = document.documentElement.clientWidth;
			h = document.documentElement.clientHeight;
		}
		return {width: w, height: h};
	},
	setTitle: function (title) {
		// Ignored in Waves
	}
};

gadgets.flash = {
	embedCachedFlash: function () {
		var args = Array.prototype.slice.call(arguments);
		return gadgets.flash.embedFlash.apply(this, args);
	},
	embedFlash: function (swfUrl, swfContainer, swfVersion, opt_params) {
		switch (typeof swfContainer) {
			case 'string':
				swfContainer = document.getElementById(swfContainer);
			case 'object':
				if (swfContainer && (typeof swfContainer.innerHTML == 'string'))
					break;
			default:
				return false;
		}
		switch (typeof opt_params) {
			case 'undefined':
				opt_params = {};
			case 'object':
				break;
			default:
				return false;
		}
		var ver = gadgets.flash.getMajorVersion();
		if (ver) {
			var swfVer = parseInt(swfVersion, 10);
			if (isNaN(swfVer))
				swfVer = 0;
			if (ver >= swfVer) {
				if (!opt_params.width)
					opt_params.width = '100%';
				if (!opt_params.height)
					opt_params.height = '100%';
				if (typeof opt_params.base != 'string')
					opt_params.base = swfUrl.match(/^[^?#]+\//)[0];
				if (typeof opt_params.wmode != 'string')
					opt_params.wmode = 'opaque';
				var html;
				if (navigator.plugins && navigator.mimeTypes && navigator.mimeTypes.length) {
					opt_params.type = 'application/x-shockwave-flash';
					opt_params.src = swfUrl;
					html = '<embed';
					for (var prop in opt_params) {
						if (!/^swf_/.test(prop))
							html += ' ' + prop + '="' + opt_params[prop] + '"';
					}
					html += ' /></embed>';
				}
				else {
					opt_params.movie = swfUrl;
					var attr = {
						width: opt_params.width,
						height: opt_params.height,
						classid: "clsid:D27CDB6E-AE6D-11CF-96B8-444553540000"
					};
					if (opt_params.id)
						attr.id = opt_params.id;
					html = '<object';
					for (var attrProp in attr)
						html += ' ' + attrProp + '="' + attr[attrProp] + '"';
					html += '>';
					for (var paramsProp in opt_params) {
						if (!/^swf_/.test(paramsProp) && !attr[paramsProp])
							html += '<param name="' + paramsProp + '" value="' + opt_params[paramsProp] + '" />';
					}
					html += '</object>';
				}
				swfContainer.innerHTML = html;
				return true;
			}
		}
		return false;
	},
	getMajorVersion: function () {
		var flashMajorVersion = 0;
		if (navigator.plugins && navigator.mimeTypes && navigator.mimeTypes.length) {
			var i = navigator.plugins["Shockwave Flash"];
			if (i && i.description)
				flashMajorVersion = parseInt(i.description.match(/[0-9]+/)[0], 10);
		}
		else {
			for (var version = 9; version > 0; version--) {
				try {
					new ActiveXObject("ShockwaveFlash.ShockwaveFlash." + version);
					return version;
				}
				catch (e) {}
			}
		}
		return flashMajorVersion;
	}
};

var _IG_GetFlashMajorVersion = gadgets.flash.getMajorVersion;
var _IG_EmbedFlash = function(swfUrl, swfContainer, opt_params) {
	return gadgets.flash.embedFlash(swfUrl, swfContainer, opt_params.swf_version, opt_params);
};
var _IG_EmbedCachedFlash = function(swfUrl, swfContainer, opt_params) {
	return gadgets.flash.embedCachedFlash(swfUrl, swfContainer, opt_params.swf_version, opt_params);
};

function _IG_Prefs(opt_moduleId) {
	if (opt_moduleId == undefined || opt_moduleId == window.__MODULE_ID__) {
		this._moduleId = window.__MODULE_ID__;
		this._prefs = gadgets._prefs;
	}
	else {
		this._moduleId = opt_moduleId;
		this._prefs = {};
	}
	
	this.getArray = function (key) {
		if (key in this._prefs) {
			var pref = this._prefs[key];
			if (pref.datatype == "list") {
				if ("value" in pref)
					return pref.value;
				else if ("default_value" in pref)
					return pref.default_value;
				else
					return [];
			}
			else {
				if ("value" in pref)
					return [pref.value];
				else if ("default_value" in pref)
					return [pref.default_value];
				else
					return [];
			}
		}
		else
			return [];
	};
	
	this.getBool = function (key) {
		if (key in this._prefs) {
			var pref = this._prefs[key];
			if ("value" in pref)
				return !!pref.value;
			else if ("default_value" in pref)
				return pref.default_value;
			else
				return false;
		}
		else
			return false;
	};
	
	this.getCountry = function () {
		return gadgets._country;
	};
	
	this.getLang = function () {
		return gadgets._lang;
	};
	
	this.getFloat = function (key) {
		if (key in this._prefs) {
			var pref = this._prefs[key];
			if ("value" in pref)
				return parseFloat(pref.value);
			else if ("default_value" in pref)
				return parseFloat(pref.default_value);
			else
				return 0;
		}
		else
			return 0;
	};
	
	this.getInt = function (key) {
		if (key in this._prefs) {
			var pref = this._prefs[key];
			if ("value" in pref)
				return parseInt(pref.value);
			else if ("default_value" in pref)
				return parseInt(pref.default_value);
			else
				return 0;
		}
		else
			return 0;
	};
	
	this.getModuleId = function () {
		return window.__MODULE_ID__;
	};
	
	this.getMsg = function (key) {
		return "";
	};
	
	this.getString = function (key) {
		if (key in this._prefs) {
			var pref = this._prefs[key];
			if ("value" in pref)
				return String(pref.value);
			else if ("default_value" in pref)
				return pref.default_value;
			else
				return "";
		}
		else
			return "";
	};
	
	// --- setprefs ---
	this.set = function (key, val) {
		if (key in this._prefs)
			this._prefs[key].value = val;
		else
			this._prefs[key] = {value: val};
		
		window.parent.gadget_rpc.set_pref(this._moduleId, key, val);
	};
	
	this.setArray = this.set;
}

gadgets.Prefs = _IG_Prefs;
