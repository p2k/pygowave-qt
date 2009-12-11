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

pygowave.view = $defined(pygowave.view) ? pygowave.view : new Hash();

(function () {
	/* Imports */
	var Widget = pygowave.view.Widget;
	var BlipEditorWidget = pygowave.view.BlipEditorWidget;
	
	var BlipContainerWidget = new Class({
		Extends: Widget,

		initialize: function (view, parentElement) {
			this._view = view;
			var contentElement = new Element('div', {'class': 'blip_container_widget'});
			this.parent(parentElement, contentElement);
			this._blips = [];
			this._rootBlip = null;
		},

		insertBlip: function (index, blip) {
			var where = 'bottom';
			var parentElement = this.contentElement;
			if (index != this._blips.length) {
				where = 'before';
				parentElement = this._blips[index];
			}
			
			var blipwgt = new BlipEditorWidget(this._view, blip, parentElement, where);
			this._blips.insert(index, blipwgt);
			
			if (blip.isRoot())
				this._rootBlip = blipwgt;
			
			return blipwgt;
		},

		deleteBlip: function (id) {
			for (var i = 0; i < this._blips.length; i++) {
				var blipwgt = this._blips[i];
				if (blipwgt.blip().id() == id) {
					this._blips.pop(i);
					blipwgt.destroy();
					break;
				}
			}
		}
	});
	
	pygowave.view.extend({
		BlipContainerWidget: BlipContainerWidget
	});
})();
