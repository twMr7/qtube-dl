cmake_minimum_required(VERSION 3.10.0)

project(qtube-dl)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run Qt tools automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(qtube-dl WIN32 main.cpp mainwindow.ui mainwindow.cpp)

target_link_libraries(qtube-dl Qt5::Core Qt5::Gui Qt5::Widgets)
