# Cave Story but with SDL Graphics

## Introduction
This is a DLL mod that completely rips out all of Cave Story's DirectDraw rendering system,
and replaces it with SDL calls (based on CSE2-portable backend code).
It is intended to fix issues such as sprites not drawing with transparency, or fullscreen
not working. It is *not* intended to be modding-friendly, although it may still be
compatible with mods that don't touch any of the rendering code.

**THIS IS NOT A GENERAL-PURPOSE DDRAW.DLL WRAPPER.** Don't try to use this for other games!

## Usage
Download a release and unzip next to Doukutsu.exe.

Note the following changes to the behavior of the DoConfig options:
* The "Full 24-bit" window mode will be treated as "960x720 Windowed"
* The "Full 32-bit" window mode will be treated as "1280x960 Windowed"
* Courier New is the only available font; changing the font name in Config.dat will have no effect.

## Compiling
The dependencies for this project are listed in `external/readme_dependencies.txt`.
(They are not included with this repo.)
A Visual Studio 2017 project and MSYS2 Makefile are provided for compiling. Note, if you
are using MSYS2 then you must use the MinGW 32-bit environment, since Cave Story is a
32-bit game and therefore the DLL must be built for a 32-bit architecture.
