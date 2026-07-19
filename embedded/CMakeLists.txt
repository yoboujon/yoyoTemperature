# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(CONSTANT_HEADER "${CMAKE_SOURCE_DIR}/main/constants.h.in")
include("${CMAKE_CURRENT_SOURCE_DIR}/../env.cmake")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(yoyo_temperature)
