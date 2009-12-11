PyGoWave Desktop Client
=======================

Website: http://github.com/p2k/pygowave-qt

Project Lead/Maintainer:
  Patrick Schneider <patrick.p2k.schneider@googlemail.com>

Install
-------
For installation or compiling instructions, see the INSTALL file.

License
-------
The PyGoWave Desktop Client itself is licensed under the
GNU General Public License. However, its two sub-projects
JSWrapper and the PyGoWave Qt/C++ Client API are licensed
under the GNU Lesser General Public License.

Description
-----------
The PyGoWave Desktop Client allows you to connect to any
PyGoWave Server without using a browser. It requires less
resources and can run in your system tray all the time,
notifying you when you get new Waves.

Dependencies
------------
You will need Qt 4.5 or later to compile this.

QJson is required for all subprojects of this project.
See http://qjson.sourceforge.net/

You also have to compile and install QStomp first, see
http://github.com/p2k/QStomp

Notes for developers
--------------------
This project includes two sub-projects which can be used
seperately: JSWrapper, a C++ to JavaScript Object wrapper
framework for use with QtWebKit, and the PyGoWave Qt/C++
Client API which allows you to write 3rd party clients
and other software around PyGoWave.
