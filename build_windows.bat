cd build

cmake .. ^
  -DCMAKE_PREFIX_PATH="C:\Qt\6.0.1\msvc2019_64"^
  -DZLIB_ROOT="%~dp0\build\depends\lib"^
  -DImath_DIR="%~dp0\build\depends\lib\lib\cmake\Imath"^
  -DOpenEXR_DIR="%~dp0\build\depends\lib\lib\cmake\OpenEXR"^
  -DCMAKE_INSTALL_PREFIX="%~dp0\build\install"

cmake --build . --target install --config Release

cp .\depends\lib\bin\*.dll Release
C:\Qt\6.0.1\msvc2019_64\bin\windeployqt Release/openexr-viewer.exe