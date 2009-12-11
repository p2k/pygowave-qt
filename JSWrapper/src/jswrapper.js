/*
 * This file is part of JSWrapper, a C++ to JavaScript wrapper on WebKit
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

// Note: You must have MooTools installed on your web page for this to work

window.jswrapper = $defined(window.jswrapper) ? window.jswrapper : new Hash();

(function () {
	var Objects = new Class({
		initialize: function() {
			this._all = new Hash();
		},
		
		forwardSignal: function (obj_class_name, obj_id, signalName, args) {
			if (!this._all.has(obj_class_name)) {
				window.console.error("JSWrapper::forwardSignal: No objects with class name '"+obj_class_name+"' could not be found!");
				return;
			}
			var objs = this._all.get(obj_class_name);
			if (!objs.has(obj_id)) {
				window.console.error("JSWrapper::forwardSignal: "+obj_class_name+" with ID '"+obj_id+"' could not be found!");
				return;
			}
			objs.get(obj_id).fireEvent(signalName, args);
		},

		// Note: obj_id may be a list
		get: function(obj_class, obj_id, parent) {
			if (!$defined(parent))
				parent = null;
			if ($type(obj_id) == 'array') {
				var lots = new Array();
				$each(obj_id, function (sub_id) {
					lots.push(jswrapper.objects.get(obj_class, sub_id, parent));
				});
				return lots;
			}
			
			if (obj_id == "")
				return null;

			var obj_class_name = obj_class.className;
			if (!this._all.has(obj_class_name))
				this._all.set(obj_class_name, new Hash());
			
			var objs = this._all.get(obj_class_name);
			if (!objs.has(obj_id)) {
				var wrapper = jswrapper.factory.createWrapper(obj_class, obj_id, parent);
				if (wrapper != null)
					objs.set(obj_id, wrapper);
				return wrapper;
			}
			return objs.get(obj_id);
		},

		changeObjectGuid: function (obj_class_name, old_id, new_id) {
			if (!this._all.has(obj_class_name))
				return;
			var objs = this._all.get(obj_class_name);
			if (!objs.has(old_id))
				return;
			var obj = objs.get(old_id);
			objs.erase(old_id);
			objs.set(new_id, obj);
			window.console.info("JSWrapper::changeObjectGuid: Changed GUID for '"+obj_class_name+"' from '"+old_id+"' to '"+new_id+"'");
		}
	});
	
	var Factory = new Class({
		initialize: function() {
			this._all = new Hash();
		},
		
		registerFactory: function (factory) {
			var self = this;
			$each(factory.classNameList, function (class_name) {
				self._all.set(class_name, factory);
			});
			window.console.info("JSWrapper::registerFactory: Registered factory for "+factory.classNameList);
		},
		
		createWrapper: function (obj_class, obj_id, parent) { // Parent is null or a CppWrapper instance
			var obj_class_name = obj_class.className;
			var obj = null;
			if (this._all.has(obj_class_name)) {
				var factory = this._all.get(obj_class_name);
				var wrapper = null;
				
				if (parent != null)
					wrapper = factory.createWrapper(obj_class_name, obj_id, parent._wrapper);
				else
					wrapper = factory.createWrapper(obj_class_name, obj_id, null);
				
				if (wrapper) {
					if (parent != null)
						obj = new obj_class(wrapper, parent);
					else
						obj = new obj_class(wrapper);
					obj.className = obj_class_name;
					window.console.info("JSWrapper::createWrapper: Created wrapper for '"+obj_class_name+"' with ID '"+obj_id+"'");
				}
				else
					window.console.warning("JSWrapper::createWrapper: Factory returned null for '"+obj_class_name+"' with ID '"+obj_id+"'!");
			}
			else
				window.console.error("JSWrapper::createWrapper: Could not find a factory for '"+obj_class_name+"' object!");
			return obj;
		}
	});
	
	var CppWrapper = new Class({
		Implements: Events,
		initialize: function (js_wrapper, parent) {
			if (!$defined(parent))
				parent = null;
			this._cpp = js_wrapper.object;
			this._parent = parent;
			this._wrapper = js_wrapper;
		},
		objectId: function () {
			return this._wrapper.objectId;
		},
		objectGuid: function () {
			return this._wrapper.objectGuid;
		},
		getChild: function (obj_class, obj_id) {
			if (obj_id == "")
				return null;
			else
				return jswrapper.objects.get(obj_class, obj_id+"@"+this.objectGuid(), this);
		},
		getChildren: function (obj_class, obj_ids) {
			if (obj_ids.length == 0)
				return [];
			var conv_ids = new Array();
			$each(obj_ids, function (obj_id) {
				conv_ids.push(obj_id+"@"+this.objectGuid());
			}, this);
			return jswrapper.objects.get(obj_class, conv_ids, this);
		},
		__repr__: function () {
			return this.className+"('"+this.objectGuid()+"')";
		}
	});
	
	jswrapper.extend({
		objects: new Objects(),
		factory: new Factory(),
		CppWrapper: CppWrapper
	});
})();
