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
        src/model/OpenEXRImage.cpp
        src/model/OpenEXRImage.h
        src/model/OpenEXRLayerItem.cpp
        src/model/OpenEXRLayerItem.h
        src/model/OpenEXRHeaderItem.cpp
        src/model/OpenEXRHeaderItem.h
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
