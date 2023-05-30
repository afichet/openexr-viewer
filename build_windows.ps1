$dir = $PWD -replace '\\', '/'

cd build

cmake .. `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.5.1/msvc2019_64" `
  -DZLIB_ROOT="$dir/build/depends/lib" `
  -DImath_DIR="$dir/build/depends/lib/lib/cmake/Imath" `
  -DOpenEXR_DIR="$dir/build/depends/lib/lib/cmake/OpenEXR" `
  -DCMAKE_INSTALL_PREFIX="$dir/build/install" `
  -DCMAKE_BUILD_TYPE="Release" `
  -DCMAKE_CONFIGURATION_TYPES="Release"

cmake --build . --config Release

cpack

cd $dir