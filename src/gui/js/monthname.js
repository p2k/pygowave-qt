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

/**
 * Simple utility function to get a localized month name (three letters)
 * @function {public} monthname
 * @param {int} n Month number
 */
window.monthname = function (n) {
	if (!$defined(window.monthname.data)) {
		window.monthname.data = [
			gettext("Jan"),
			gettext("Feb"),
			gettext("Mar"),
			gettext("Apr"),
			gettext("May"),
			gettext("Jun"),
			gettext("Jul"),
			gettext("Aug"),
			gettext("Sep"),
			gettext("Oct"),
			gettext("Nov"),
			gettext("Dec")
		];
	}
	if (n < 1 || n > 12)
		return "???";
	return window.monthname.data[n-1];
}
