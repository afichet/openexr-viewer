cmake_minimum_required(VERSION 3.5)

project(simple-exr-viewer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Imath CONFIG)
find_package(OpenEXR CONFIG)

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


)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(simple-exr-viewer
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(simple-exr-viewer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(simple-exr-viewer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(simple-exr-viewer PRIVATE src)

target_link_libraries(simple-exr-viewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(simple-exr-viewer
    PRIVATE
        OpenEXR::OpenEXR
    )
