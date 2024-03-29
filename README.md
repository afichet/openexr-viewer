OpenEXR Viewer
==============

[![openexr-viewer](https://snapcraft.io/openexr-viewer/badge.svg)](https://snapcraft.io/openexr-viewer)

A simple viewer for OpenEXR files with detailed metadata probing.

You can display various types of layers, automatically combines RGB,
Luminance-Chroma and Y layers.

![Screenshot from 2021-09-11 01-58-27](https://user-images.githubusercontent.com/7930348/132928984-fd31c2c3-c66f-43c9-b63b-2f1836a09fe8.png)

Disclaimer
==========

This is a very early version... expect crashes, don't expect polished
piece of software ;-)


Installing
==========

Windows
-------

You can install OpenEXR Viewer from winget. Run in a terminal:

```
winget install openexr-viewer
```

Alternatively, there is an installer you can download from the release
section: https://github.com/afichet/openexr-viewer/releases


macOS
-----

There is prebuilt releases for macOS. You just need to download the
installer in the releases section and run it.

Get the releases here: https://github.com/afichet/openexr-viewer/releases


Linux
-----

For Arch Linux, you can use the AUR repository. To install, do:

```
yay -S openexr-viewer
```

For other distribution, there is a snap package available:

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-black.svg)](https://snapcraft.io/openexr-viewer)


```
sudo snap install openexr-viewer
```


Building
========

To build this package, you need Qt5 or greater and OpenEXR 3.0.1 or greater.

Linux
-----

If your package manager does come with an older version of OpenEXR,
you will have to build it your own:

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

Windows
-------

To help for building on Windows, there is two scripts you can use:
- `build_dependencies.bat` to build the necessary Imath and OpenEXR libraries
- `build_windows.bat` to execute after the first to build the package.

It will generate in `build/Release` a shipable binary.

You will have to edit the script `build_windows.bat` to specify your
Qt installation folder.

macOS
-----

You can install Qt from the official website and get a recent OpenEXR
release from homebrew.

```bash
brew install openexr
```

Then, you need to specify the of Qt's install when running CMake. For example:

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=~/Qt/6.1.0/clang64
```

Then, you can build the package:

```bash
make
```


License
=======

It is licensed under the 3-clause BSD license.

This software uses the colormaps:

- from https://bids.github.io/colormap/ by Nathaniel J. Smith, Stefan
van der Walt, and (in the case of viridis) Eric Firing. These are
licensed under CC0 license
(http://creativecommons.org/publicdomain/zero/1.0/).
- Turbo from https://gist.github.com/mikhailov-work/ee72ba4191942acecc03fe6da94fc73f by Anton Mikhailov licensed under Apache-2.0.
- Most icons made by DinosoftLabs from www.flaticon.com 
