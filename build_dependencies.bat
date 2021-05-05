mkdir build
cd build

mkdir depends
cd depends
mkdir lib

mkdir src
cd src
git clone https://github.com/madler/zlib.git
git clone https://github.com/AcademySoftwareFoundation/Imath.git
git clone https://github.com/AcademySoftwareFoundation/openexr.git


cd zlib
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="../../../lib"
cmake --build . --target install --config Release

cd ../..

cd Imath
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="../../../lib"

cmake --build . --target install --config Release

cd ../..

cd openexr
mkdir build
cd build
cmake .. ^
  -DCMAKE_INSTALL_PREFIX="../../../lib"^
  -DZLIB_ROOT="../../../lib"^
  -DImath_DIR="../../../lib/cmake/Imath"

cmake --build . --target install --config Release