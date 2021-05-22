cmake_minimum_required(VERSION 3.5)

project(openexr-viewer 
    VERSION 0.2
    DESCRIPTION "Simple Viewer for OpenEXR files with detailed metadata display"
    HOMEPAGE_URL "https://github.com/afichet/openexr-viewer"
    LANGUAGES CXX 
    )

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

cmake_policy(SET CMP0074 NEW)

include(GNUInstallDirs)

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(OpenMP)

# Those are provided by Imath and OpenEXR 3.0
find_package(Imath REQUIRED)
find_package(OpenEXR REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# ----------------------------------------------------------------------------
# Source
# ----------------------------------------------------------------------------

set(PROJECT_SOURCES
    src/main.cpp
    src/view/mainwindow.cpp
    src/view/mainwindow.h
    src/view/mainwindow.ui

    src/view/GraphicsScene.cpp
    src/view/GraphicsScene.h

    src/view/GraphicsView.cpp
    src/view/GraphicsView.h

    src/view/RGBFramebufferWidget.cpp
    src/view/RGBFramebufferWidget.h
    src/view/RGBFramebufferWidget.ui

    src/view/FramebufferWidget.cpp
    src/view/FramebufferWidget.h
    src/view/FramebufferWidget.ui

    src/view/ScaleWidget.cpp
    src/view/ScaleWidget.h

    src/model/OpenEXRImage.cpp
    src/model/OpenEXRImage.h
    src/model/OpenEXRLayerItem.cpp
    src/model/OpenEXRLayerItem.h
    src/model/OpenEXRHeaderItem.cpp
    src/model/OpenEXRHeaderItem.h

    src/model/ImageModel.cpp
    src/model/ImageModel.h
    src/model/FramebufferModel.cpp
    src/model/FramebufferModel.h
    src/model/RGBFramebufferModel.cpp
    src/model/RGBFramebufferModel.h

    src/model/HeaderModel.cpp
    src/model/HeaderModel.h

    src/util/Colormap.cpp
    src/util/Colormap.h
    src/util/TabulatedColormap.cpp
    src/util/TabulatedColormap.h
    src/util/BBGRColormap.cpp
    src/util/BBGRColormap.h
    src/util/YColormap.cpp
    src/util/YColormap.h
    src/util/ColormapModule.h
    src/util/ColormapModule.cpp

    openexr-viewer.rc
    assets/themes/dark_flat.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if (WIN32)
        qt_add_executable(openexr-viewer WIN32
            ${PROJECT_SOURCES}
        )
    elseif(APPLE)
        qt_add_executable(openexr-viewer MACOSX_BUNDLE
            ${PROJECT_SOURCES}
        )
    else()
        qt_add_executable(openexr-viewer
            ${PROJECT_SOURCES}
        )
    endif()
else()
    if(WIN32)
        add_executable(openexr-viewer WIN32
            ${PROJECT_SOURCES}
        )
    elseif(APPLE)
        add_executable(openexr-viewer MACOSX_BUNDLE
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(openexr-viewer
            ${PROJECT_SOURCES}
        )    
    endif()
endif()

target_include_directories(openexr-viewer PRIVATE src)
target_include_directories(openexr-viewer PRIVATE 3rdparty/data)

target_link_libraries(openexr-viewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(openexr-viewer PRIVATE Imath::Imath OpenEXR::OpenEXR)

if((OpenMP_CXX_FOUND) OR (OpenMP_FOUND))
    target_link_libraries(openexr-viewer PRIVATE OpenMP::OpenMP_CXX)
    # target_link_libraries(openexr-viewer PRIVATE OpenMP::OpenMP_C)
endif()


if (WIN32)
    add_custom_command(
        TARGET openexr-viewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Widgets>
        $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Core>
        $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Gui>
        $<TARGET_FILE_DIR:openexr-viewer>
        )
endif(WIN32)


# ----------------------------------------------------------------------------
# Install Rules
# ----------------------------------------------------------------------------

if(UNIX AND NOT APPLE)
    install(TARGETS openexr-viewer)

    # Application desktop entry
    install(
        FILES 
            deploy/linux/openexr-viewer.desktop
        DESTINATION 
            share/applications
    )

    # MIME data
    install(
        FILES 
            deploy/linux/openexr-viewer.xml
        DESTINATION 
            share/mime/packages
    )

    # Application icon
    install(
        FILES
            deploy/icon/openexr-viewer.png
        DESTINATION
            share/pixmaps
    )
elseif(WIN32)
    install(
        TARGETS 
            openexr-viewer 
        DESTINATION 
            ${CMAKE_INSTALL_BINDIR}
    )

    set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)

    install(
        FILES
            "${ZLIB_ROOT}\\bin\\zlib.dll"       # Zlib
            $<TARGET_FILE:Imath::Imath>         # OpenEXR
            $<TARGET_FILE:OpenEXR::Iex>
            $<TARGET_FILE:OpenEXR::IlmThread>
            $<TARGET_FILE:OpenEXR::OpenEXR>
            ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} # Windows DLLs
        DESTINATION 
            ${CMAKE_INSTALL_BINDIR}
    )

    include(Windeployqt)
    windeployqt(openexr-viewer ${CMAKE_INSTALL_BINDIR})
endif()


# ----------------------------------------------------------------------------
# Packaging
# ----------------------------------------------------------------------------

# set(CPACK_PACKAGE_NAME "OpenEXR Viewer")
# set(CPACK_PACKAGE_VENDOR "")
set(CPACK_PACKAGE_DESCRIPTION "Simple Viewer for OpenEXR files with detailed metadata display")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenEXR Viewer")
# set(CPACK_PACKAGE_ICON "")
# set(CPACK_PACKAGE_CHECKSUM "SHA512")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
# set(CPACK_RESOURCE_FILE_README "README.md")
set(CPACK_MONOLITHIC_INSTALL True)
set(CPACK_PACKAGE_EXECUTABLES "openexr-viewer;OpenEXR Viewer")

# NSIS specific settings
if(CPACK_GENERATOR MATCHES "NSIS")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_LIST_DIR}/deploy/icon/openexr-viewer.ico")
  set(CPACK_NSIS_HELP_LINK "")
  set(CPACK_NSIS_URL_INFO_ABOUT "")
  set(CPACK_CREATE_DESKTOP_LINKS "OpenEXR Viewer")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "openexr-viewer.exe")
  set(CPACK_NSIS_MENU_LINKS "openexr-viewer.exe" "OpenEXR Viewer")
  set(CPACK_NSIS_MODIFY_PATH OFF)
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "openexr-viewer.exe")
endif(CPACK_GENERATOR MATCHES "NSIS")

if("${CPACK_GENERATOR}" STREQUAL "PackageMaker")
  set(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications")
endif("${CPACK_GENERATOR}" STREQUAL "PackageMaker")


# CPack
include(CPack)
