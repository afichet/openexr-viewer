OpenEXR Viewer
==============

Simple Viewer for OpenEXR files with detailed metadata display.

You can display various types of layers, automatically combines RGB, Luminance-Chroma and Y layers.


Disclaimer
==========

This is a very early version... expect crashes, don't expect polished piece of software ;-)

It would require a lot of work, but I found it usefull for me so why not sharing it.



Building
========

## Linux

To build this package, you need Qt5 or greater and OpenEXR 3.0.1 or greater.

If your package manager does not come with an older version of OpenEXR, you can build it your own:
```bash
cd /tmp

# Imath dependency
git clone https://github.com/AcademySoftwareFoundation/Imath.git
cd Imath
mkdir build
cd build
cmake ..
sudo make install

cd /tmp

# OpenEXR
git clone https://github.com/AcademySoftwareFoundation/openexr.git
cd openexr
mkdir build
cd build
cmake ..
sudo make install
```

Then, you can build the software
```bash
git clone https://github.com/afichet/openexr-viewer.git
cd openexr-viewer
mkdir build
cd build
cmake ..
make
```

Currently, no install rule was created (very early version ;-))

## Windows

To help building on Windows, there is two scripts you can use:
- `build_dependencies.bat` to build the necessary Imath and OpenEXR library
- `build_windows.bat` to execute after the first to build the package.

It will generate in `build/Release` a shipable binary.

You will have to edit the script to specify the Qt installation folder.


License
=======

It is licensed under the 3-clause BSD license.

This software uses the colormaps from https://bids.github.io/colormap/
by Nathaniel J. Smith, Stefan van der Walt, and (in the case of
viridis) Eric Firing. These are licensed under CC0 license
(http://creativecommons.org/publicdomain/zero/1.0/).
