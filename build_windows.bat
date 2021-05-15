cd build

cmake .. ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.1.0\msvc2019_64"^
  -DZLIB_ROOT="%~dp0\build\depends\lib"^
  -DImath_DIR="%~dp0\build\depends\lib\lib\cmake\Imath"^
  -DOpenEXR_DIR="%~dp0\build\depends\lib\lib\cmake\OpenEXR"^
  -DCMAKE_INSTALL_PREFIX="%~dp0\build\install"^
  -DCMAKE_BUILD_TYPE="Release"^
  -DCMAKE_CONFIGURATION_TYPES="Release"

cmake --build . --config Release

cpack