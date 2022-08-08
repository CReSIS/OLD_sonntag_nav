# sonntag_nav
Reads Novatel raw binary GPS stream (BESTPOSB, BESTVELB, TIMEB) from serial port and makes available via TCP server in Sonntag's navigation guidance system format.

The following standard Ubuntu packages must be installed (e.g. `sudo apt install PACKAGE`) to compile the source code (compiled on Ubuntu 20):

* libqwt-qt5-dev (and additional prompted packages)
* qt5-default (and additional prompted packages)

May be useful, but possibly not required:

* qtcreator (and additional prompted packages)
* libqt5serialport5-dev (and additional prompted packages)
* libqt5svg5-dev (and additional prompted packages)
