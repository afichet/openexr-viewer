OpenEXR Viewer
==============

A simple viewer for OpenEXR files with detailed metadata probing.

You can display various types of layers, automatically combines RGB, Luminance-Chroma and Y layers.

![screenshot](https://user-images.githubusercontent.com/7930348/119127837-b56bcb80-ba2c-11eb-8311-718c713fa3bc.png)


Disclaimer
==========

This is a very early version... expect crashes, don't expect polished piece of software ;-)


Installing
==========

There is prebuilt releases for Windows. You just need to download the installer i the release section and run it.

For Arch Linux, there is an AUR package available. To install, do:
```
yay -S openexr-viewer
```


Building
========

## Linux

To build this package, you need Qt5 or greater and OpenEXR 3.0.1 or greater.

If your package manager does come with an older version of OpenEXR, you will have to build it your own:

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

Then, you're ready to build the software:

```bash
git clone https://github.com/afichet/openexr-viewer.git
cd openexr-viewer
mkdir build
cd build
cmake ..
make
```

Currently, no install rule is there (yes, very early version ;-))

## Windows

To help for building on Windows, there is two scripts you can use:
- `build_dependencies.bat` to build the necessary Imath and OpenEXR libraries
- `build_windows.bat` to execute after the first to build the package.

It will generate in `build/Release` a shipable binary.

You will have to edit the script `build_windows.bat` to specify your Qt installation folder.


License
=======

It is licensed under the 3-clause BSD license.

This software uses the colormaps from https://bids.github.io/colormap/
by Nathaniel J. Smith, Stefan van der Walt, and (in the case of
viridis) Eric Firing. These are licensed under CC0 license
(http://creativecommons.org/publicdomain/zero/1.0/).
