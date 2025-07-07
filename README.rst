WARP ESP32 Ethernet 2.0 Co Bricklet
===================================

**This Bricklet is currently in development.**

This repository contains the firmware source code for the co processor
of the WARP ESP32 Ethernet 2.0 Brick.

Repository Content
------------------

software/:
 * examples/: Examples for all supported languages
 * build/: Compiled files
 * src/: Source code of firmware
 * Makefile: Makefile to build project

Software
--------

If you want to do your own Brick/Bricklet firmware development we highly
recommend that you use our build environment setup script and read the
tutorial (https://www.tinkerforge.com/en/doc/Tutorials/Tutorial_Build_Environment/Tutorial.html).

To compile the C code we recommend you to install the newest GNU Arm Embedded
Toolchain (https://launchpad.net/gcc-arm-embedded/+download).
You also need to install bricklib2 (https://github.com/Tinkerforge/bricklib2).

You can either clone it directly in software/src/ or clone it in a
separate folder and symlink it into software/src/
(ln -s bricklib_path/bricklib2 project_path/software/src/). Finally make sure to
have CMake installed (http://www.cmake.org/cmake/resources/software.html).

After that you can build the firmware by invoking make in software/.
The firmware (.zbin) can then be found in software/build/ and uploaded
with brickv (click button "Flashing" on start screen).
