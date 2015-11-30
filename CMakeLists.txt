cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(unitd C)
include(GNUInstallDirs)

find_package(JSON_C REQUIRED)

add_subdirectory(src)
