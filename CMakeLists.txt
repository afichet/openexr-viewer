cmake_minimum_required(VERSION 3.5)

project(openexr-viewer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(OpenMP)

# OpenEXR 3.0 provides its own find_package
find_package(Imath CONFIG)
find_package(OpenEXR CONFIG)

if (OpenEXR_FOUND)
    set(OpenEXR_LIBRARIES OpenEXR::OpenEXR Imath::Imath)
else()
    set(OPENEXR_BUILD_UTILS OFF CACHE BOOL "" FORCE)
    set(OPENEXR_INSTALL_PKG_CONFIG OFF CACHE BOOL "" FORCE)    
    set(OPENEXR_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(OPENEXR_INSTALL_TOOLS OFF CACHE BOOL "" FORCE)
    set(OPENEXR_INSTALL OFF CACHE BOOL "" FORCE)
    set(IMATH_INSTALL OFF CACHE BOOL "" FORCE)
    set(IMATH_INSTALL_PKG_CONFIG OFF CACHE BOOL "" FORCE)
    set(IMATH_INSTALL_SYM_LINK OFF CACHE BOOL "" FORCE)    

    add_subdirectory(3rdparty/openexr)
    add_subdirectory(3rdparty/Imath)

    set(OpenEXR_LIBRARIES OpenEXR::OpenEXR Imath::Imath)
endif()


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

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(openexr-viewer
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(openexr-viewer SHARED
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
target_link_libraries(openexr-viewer PRIVATE ${OpenEXR_LIBRARIES})

if((OpenMP_CXX_FOUND) OR (OpenMP_FOUND))
    target_link_libraries(openexr-viewer PRIVATE OpenMP::OpenMP_CXX)
#    target_link_libraries(openexr-viewer PRIVATE OpenMP::OpenMP_C)
endif()
