cmake_minimum_required(VERSION 3.5)

project(NicQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        src/main.cpp
        src/main_window.cpp
        src/main_window.h
        src/main_window.ui
        src/nic.cpp
        src/nic.h
        src/utf8.h
)

qt_add_executable(NicQt
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(NicQt PRIVATE Qt6::Widgets)

set_target_properties(NicQt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HelloQt)
endif()
