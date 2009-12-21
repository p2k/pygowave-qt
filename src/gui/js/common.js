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

/**
 * Common classes and widgets.
 * 
 * @module pygowave.view.common
 */
pygowave.view = $defined(pygowave.view) ? pygowave.view : new Hash();
(function () {
	
	/**
	 * Base class for all view Widgets.
	 * 
	 * @class {private} pygowave.view.Widget
	 */
	var Widget = new Class({
		Implements: Events,
		/**
		 * Called on instantiation.
		 * @constructor {public} initialize
		 * @param {Element} parentElement Parent DOM element to insert the widget
		 * @param {Element} contentElement DOM element to be inserted into the
		 *        parent element
		 * @param {optional String} where Where to inject the contentElement
		 *        relative to the parent element. Can be 'top', 'bottom',
		 *        'after', or 'before'.
		 */
		initialize: function(parentElement, contentElement, where) {
			this.contentElement = contentElement;
			this.inject(parentElement, where);
		},
		
		/**
		 * Removes the widget from the DOM. Sets the parentElement to null.
		 * @function {public Widget} dispose
		 * @return Returns a reference to this widget.
		 */
		dispose: function () {
			this.contentElement.dispose();
			this.parentElement = null;
			return this;
		},
		
		/**
		 * Destroys the widget, its content element and all children of the
		 * content element.
		 */
		destroy: function () {
			this.dispose();
			this.contentElement.destroy();
		},
		
		/**
		 * Inject this widget somewhere else. Sets the parent element.<br/>
		 * Note: Called by initialize.
		 * @function {public Widget} inject
		 * @param {Element} parentElement New parent element
		 * @param {optional String} where Where to inject the contentElement
		 *        relative to the parent element. Can be 'top', 'bottom',
		 *        'after', or 'before'.
		 * @return Returns a reference to this widget.
		 */
		inject: function (parentElement, where) {
			this.parentElement = parentElement;
			this.contentElement.inject(this.parentElement, where);
			return this;
		}
	});

	pygowave.view.extend({
		Widget: Widget
	});
})();

// From PyCow //

str = function (obj) {
	if (!$defined(obj))
		return "null";
	else if ($defined(obj.__str__))
		return obj.__str__();
	else if ($type(obj) == "number")
		return String(obj);
	else if ($type(obj) == "string")
		return obj;
	else
		return repr(obj);
};

repr = function (obj) {
	if ($defined(obj.__repr__))
		return obj.__repr__();
	else if ($type(obj) == "boolean")
		return String(obj);
	else if ($type(obj) == "array")
		return "["+obj.map(repr).join(", ")+"]";
	else
		return JSON.encode(obj);
};

dbgprint = function () {
	var s = "";
	var first = true;

	for (var i = 0; i < arguments.length; i++) {
		if (first)
			first = false;
		else
			s += " ";
		s += str(arguments[i]);
	}

	if ($defined(window.console))
		window.console.info(s);
	else if (Browser.Engine.presto && $defined(opera.postError))
		opera.postError(s);
	else
		alert(s);
};

/*  
 *  Javascript sprintf
 *  http://www.webtoolkit.info/
 */

sprintfWrapper = {
	init : function () {
		if (!$defined(arguments))
			return null;
		if (arguments.length < 1)
			return null;
		if ($type(arguments[0]) != "string")
			return null;
		if (!$defined(RegExp))
			return null;
		
		var string = arguments[0];
		var exp = new RegExp(/(%([%]|(\-)?(\+|\x20)?(0)?(\d+)?(\.(\d)?)?([bcdfosxX])))/g);
		var matches = new Array();
		var strings = new Array();
		var convCount = 0;
		var stringPosStart = 0;
		var stringPosEnd = 0;
		var matchPosEnd = 0;
		var newString = '';
		var match = null;
		
		while ((match = exp.exec(string))) {
			if (match[9])
				convCount += 1;
				
			stringPosStart = matchPosEnd;
			stringPosEnd = exp.lastIndex - match[0].length;
			strings[strings.length] = string.substring(stringPosStart, stringPosEnd);
			
			matchPosEnd = exp.lastIndex;
			matches[matches.length] = {
				match: match[0],
				left: match[3] ? true : false,
				sign: match[4] || '',
				pad: match[5] || ' ',
				min: match[6] || 0,
				precision: match[8],
				code: match[9] || '%',
				negative: parseInt(arguments[convCount]) < 0 ? true : false,
				argument: String(arguments[convCount])
			};
		}
		strings[strings.length] = string.substring(matchPosEnd);

		if (matches.length == 0)
			return string;
		if ((arguments.length - 1) < convCount)
			return null;

		var code = null;
		var match = null;
		var i = null;

		for (i=0; i<matches.length; i++) {
			if (matches[i].code == '%') { substitution = '%' }
			else if (matches[i].code == 'b') {
				matches[i].argument = String(Math.abs(parseInt(matches[i].argument)).toString(2));
				substitution = sprintfWrapper.convert(matches[i], true);
			}
			else if (matches[i].code == 'c') {
				matches[i].argument = String(String.fromCharCode(parseInt(Math.abs(parseInt(matches[i].argument)))));
				substitution = sprintfWrapper.convert(matches[i], true);
			}
			else if (matches[i].code == 'd') {
				matches[i].argument = String(Math.abs(parseInt(matches[i].argument)));
				substitution = sprintfWrapper.convert(matches[i]);
			}
			else if (matches[i].code == 'f') {
				matches[i].argument = String(Math.abs(parseFloat(matches[i].argument)).toFixed(matches[i].precision ? matches[i].precision : 6));
				substitution = sprintfWrapper.convert(matches[i]);
			}
			else if (matches[i].code == 'o') {
				matches[i].argument = String(Math.abs(parseInt(matches[i].argument)).toString(8));
				substitution = sprintfWrapper.convert(matches[i]);
			}
			else if (matches[i].code == 's') {
				matches[i].argument = matches[i].argument.substring(0, matches[i].precision ? matches[i].precision : matches[i].argument.length)
				substitution = sprintfWrapper.convert(matches[i], true);
			}
			else if (matches[i].code == 'x') {
				matches[i].argument = String(Math.abs(parseInt(matches[i].argument)).toString(16));
				substitution = sprintfWrapper.convert(matches[i]);
			}
			else if (matches[i].code == 'X') {
				matches[i].argument = String(Math.abs(parseInt(matches[i].argument)).toString(16));
				substitution = sprintfWrapper.convert(matches[i]).toUpperCase();
			}
			else {
				substitution = matches[i].match;
			}
			
			newString += strings[i];
			newString += substitution;
		}
		
		newString += strings[i];
		return newString;
	},
	convert : function(match, nosign){
		if (nosign)
			match.sign = '';
		else
			match.sign = match.negative ? '-' : match.sign;
		
		var l = match.min - match.argument.length + 1 - match.sign.length;
		var pad = new Array(l < 0 ? 0 : l).join(match.pad);
		if (!match.left) {
			if (match.pad == "0" || nosign)
				return match.sign + pad + match.argument;
			else
				return pad + match.sign + match.argument;
		} else {
			if (match.pad == "0" || nosign)
				return match.sign + match.argument + pad.replace(/0/g, ' ');
			else
				return match.sign + match.argument + pad;
		}
	}
};

/*
 * Convert a unicode string to utf-8
 * http://www.webtoolkit.info/
 */
utf8encode = function (string) {
	string = string.replace(/\r\n/g,"\n");
	var utftext = "";
	
	for (var n = 0; n < string.length; n++) {
		var c = string.charCodeAt(n);
		if (c < 128) {
			utftext += String.fromCharCode(c);
		}
		else if((c > 127) && (c < 2048)) {
			utftext += String.fromCharCode((c >> 6) | 192);
			utftext += String.fromCharCode((c & 63) | 128);
		}
		else {
			utftext += String.fromCharCode((c >> 12) | 224);
			utftext += String.fromCharCode(((c >> 6) & 63) | 128);
			utftext += String.fromCharCode((c & 63) | 128);
		}
	}
	
	return utftext;
};

sprintf = sprintfWrapper.init;

String.implement({
	sprintf: function () {
		var args = $A(arguments);
		args.splice(0, 0, this);
		return sprintfWrapper.init.apply(this, args);
	},
	startswith: function (s) {
		return this.slice(0,s.length) == s;
	},
	endswith: function (s) {
		return this.slice(this.length-s.length) == s;
	},
	encode: function (encoding) {
		encoding = encoding.toLowerCase();
		if (encoding == "utf8" || encoding == "utf-8")
			return utf8encode(this);
		throw Error("Unknown encoding: " + encoding);
	}
});

String.alias("toLowerCase", "lower");
String.alias("toUpperCase", "upper");
Hash.alias("extend", "update");

Array.implement({
	insert: function (index, object) {
		this.splice(index, 0, object);
	},
	append: function (object) {
		this[this.length] = object;
	}
});

/**
 * Java-Style iterator class.
 */
_Iterator = new Class({
	initialize: function (object) {
		this.obj = object;
		this.pos = -1;
		if (object instanceof Array)
			this.elts = null;
		else {
			this.elts = new Array();
			for (var x in object) {
				if (object.hasOwnProperty(x))
					this.elts.push(x);
			}
		}
	},
	hasNext: function () {
		if (this.elts == null) {
			if (this.pos >= this.obj.length-1)
				return false;
		}
		else {
			if (this.pos >= this.elts.length-1)
				return false;
		}
		return true;
	},
	next: function () {
		if (this.elts == null) {
			if (this.pos >= this.obj.length-1)
				throw new IndexError();
			return this.obj[++this.pos];
		}
		else {
			if (this.pos >= this.elts.length-1)
				throw new IndexError();
			return this.obj[this.elts[++this.pos]];
		}
	},
	key: function () {
		if (this.elts == null)
			return this.pos;
		else {
			if (this.pos >= this.elts.length)
				throw new IndexError();
			return this.elts[this.pos];
		}
	}
});

// From Django //

var catalog = new Array();

function gettext(msgid) {
  var value = catalog[msgid];
  if (typeof(value) == 'undefined') {
	return msgid;
  } else {
	return (typeof(value) == 'string') ? value : value[0];
  }
}

function ngettext(singular, plural, count) {
  value = catalog[singular];
  if (typeof(value) == 'undefined') {
	return (count == 1) ? singular : plural;
  } else {
	return value[pluralidx(count)];
  }
}

function gettext_noop(msgid) { return msgid; }

function interpolate(fmt, obj, named) {
  if (named) {
	return fmt.replace(/%\(\w+\)s/g, function(match){return String(obj[match.slice(2,-2)])});
  } else {
	return fmt.replace(/%s/g, function(match){return String(obj.shift())});
  }
}
