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
 * Browser independent selection objects.
 * 
 * @module pygowave.view.selection
 */
(function () {
	
	/**
	 * Browser independent selection class which wraps W3C and MS selection
	 * and range objects.<br/>
	 * Note: This is at no rate a complete implementation for selections and
	 * ranges, it is used for special purposes only.
	 * 
	 * @class {public} pygowave.view.Selection
	 */
	var Selection = new Class({
		/**
		 * Called on instantiation.
		 * @constructor {public} initialize
		 * @param {optional Node} startNode Start node
		 * @param {optional int} startOffset Start offset
		 * @param {optional Node} endNode End node
		 * @param {optional int} endOffset End offset
		 * @param {optional Document} ownerDocument Owner document for the selection.
		 */
		initialize: function (startNode, startOffset, endNode, endOffset, ownerDocument) {
			if (!$defined(ownerDocument))
				this._doc = window.document;
			else
				this._doc = ownerDocument;
			this._startNode = ($defined(startNode) ? startNode : null);
			this._endNode = ($defined(endNode) ? endNode : null);
			this._startOffset = ($defined(startOffset) ? startOffset : 0);
			this._endOffset = ($defined(endOffset) ? endOffset : 0);
			this._atBottom = false;
		},
		/**
		 * Return the start node.
		 * @function {public Node} startNode
		 */
		startNode: function () {
			return this._startNode;
		},
		/**
		 * Return the end node.
		 * @function {public Node} endNode
		 */
		endNode: function () {
			return this._endNode;
		},
		/**
		 * Return the start offset.
		 * @function {public Node} startOffset
		 */
		startOffset: function () {
			return this._startOffset;
		},
		/**
		 * Return the end offset.
		 * @function {public Node} endOffset
		 */
		endOffset: function () {
			return this._endOffset;
		},
		/**
		 * Set the start node and offset.
		 * @function {public} setStart
		 * @param {Node} node Start node
		 * @param {int} offset Start offset
		 */
		setStart: function (node, offset) {
			this._startNode = node;
			this._startOffset = offset;
			this._atBottom = false;
		},
		/**
		 * Set the end node and offset.
		 * @function {public} setEnd
		 * @param {Node} node End node
		 * @param {int} offset End offset
		 */
		setEnd: function (node, offset) {
			this._endNode = node;
			this._endOffset = offset;
			this._atBottom = false;
		},
		/**
		 * Returns true if the selection is valid.
		 * @function {public Boolean} isValid
		 */
		isValid: function () {
			return (this._startNode != null && this._endNode != null);
		},
		/**
		 * Returns true if the selection is collapsed (i.e. empty).
		 * @function {public Boolean} isCollapsed
		 */
		isCollapsed: function () {
			return (this._startNode == this._endNode && this._startOffset == this._endOffset);
		},
		/**
		 * Move the start and end point to the lowest node (usually a text
		 * node) if possible.
		 * @function {public} moveDown
		 */
		moveDown: function () {
			if (!this.isValid() || this._atBottom)
				return;
			
			var prev;
			while ($type(this._startNode) == "element" && $defined(this._startNode.firstChild)) {
				prev = this._startNode;
				this._startNode = this._startNode.firstChild;
				while ($defined(this._startNode) && this._startOffset > 0) {
					prev = this._startNode;
					this._startNode = this._startNode.nextSibling;
					this._startOffset--;
				}
				if (!$defined(this._startNode))
					this._startNode = prev;
			}
			
			while ($type(this._endNode) == "element" && $defined(this._endNode.firstChild)) {
				prev = this._endNode;
				this._endNode = this._endNode.firstChild;
				while ($defined(this._endNode) && this._endOffset > 0) {
					prev = this._endNode;
					this._endNode = this._endNode.nextSibling;
					this._endOffset--;
				}
				if (!$defined(this._endNode))
					this._endNode = prev;
			}
			
			this._atBottom = true;
		},
		/**
		 * Return a native text range object (either W3C or MS).
		 * @function {public Object} toNativeRange
		 */
		toNativeRange: function () {
			if (!this.isValid)
				return null;
			
			var rng;
			rng = this._doc.createRange();
			rng.setStart(this._startNode, this._startOffset);
			rng.setEnd(this._endNode, this._endOffset);
			return rng;
		},
		/**
		 * Set the user's current selection to this selection.
		 * @function {public} select
		 */
		select: function () {
			var range = this.toNativeRange();
			if (range.select){
				$try(function(){
					range.select();
				});
			} else {
				var win = this._doc.window;
				var s = $defined(win.getSelection) ? win.getSelection() : this._doc.selection;
				if (s.addRange) {
					s.removeAllRanges();
					s.addRange(range);
				}
			}
		}
	});

	/**
	 * Returns the current selection of the user.
	 * 
	 * @function {static public Selection} currentSelection
	 * @param {optional Element} scope Limit the scope to this element, i.e.
	 *        return an invalid selection if it is not inside this element.
	 * @param {optional Element} target Target from the event object to check
	 *        against the scope
	 * @param {optional Document} ownerDocument Owner document for the selection.
	 *        Defaults to the current window's document.
	 */
	Selection.currentSelection = function (scope, target, ownerDocument) {
		if (!$defined(ownerDocument))
			ownerDocument = window.document;
		if (!$defined(scope))
			scope = ownerDocument.body;
		
		var win = ownerDocument.window;
		var sel = $defined(win.getSelection) ? win.getSelection() : ownerDocument.selection;
		if (!$defined(sel))
			return new Selection();
		var rng = (sel.rangeCount > 0 ? sel.getRangeAt(0) : (sel.createRange ? sel.createRange() : null));
		if (!$defined(rng))
			return new Selection();
			
		if (scope != ownerDocument.body) {
			// Check selection scope
			var elt = rng.startContainer;
			while ($type(elt) != "element" || (elt != scope && elt.get('tag') != "html"))
				elt = elt.parentNode;
			if (elt != scope)
				return new Selection();
			elt = rng.endContainer;
			while ($type(elt) != "element" || (elt != scope && elt.get('tag') != "html"))
				elt = elt.parentNode;
			if (elt != scope)
				return new Selection();
			
			// Also check target
			if ($defined(target)) {
				var elt = target;
				while ($type(elt) != "element" || (elt != scope && elt.get('tag') != "html"))
					elt = elt.parentNode;
				if (elt != scope)
					return new Selection();
				elt = rng.endContainer;
				while ($type(elt) != "element" || (elt != scope && elt.get('tag') != "html"))
					elt = elt.parentNode;
				if (elt != scope)
					return new Selection();
			}
		}
		return new Selection(
			rng.startContainer,
			rng.startOffset,
			rng.endContainer,
			rng.endOffset,
			ownerDocument
		);
	};
	
	pygowave.view.extend({
		Selection: Selection
	});
})();
