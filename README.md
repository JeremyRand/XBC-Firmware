XBC-Firmware
============

Xport Botball Controller Custom Firmware

Authors: Jeremy Rand and Farz Hemmati

Based on the official XBC firmware by KIPR and Charmed Labs.

Code written in 2006-2008, long before we used Git (so our changes aren't marked in the changelog), but our changes include:

* Storing multiple user programs (menu lets you select which one; downloading new programs automatically picks an open slot in flash)
* Playing .wav sounds on the robot speaker (including .wav mixing)
* NonPCode: A native C++ mode 86 times faster than the default p-code VM (2 years before KIPR added native C to the CBC)
* ICXView: Pixel-by-pixel graphics (1 year before KIPR added it)
* Scrolling menus
* Low-level sonar control (including jamming -- yes, this was legal in Botball, but we never used it)
* Low-level motor control (including motion recording/playback, detecting stalled motors, and current draw sensing)
* Differential base robot kinematics
* Setting palette colors
* Faster firmware update process (use Cport cable for downloads of a few seconds without needing to overwrite the serial bootloader).

Big thanks to Jorge Villatoro of KIPR and Rich LeGrand of Charmed Labs for their support.

Licensed under Mozilla Public License (like the original code).
