cmake_minimum_required(VERSION 3.18)

# Project
project(
  yoyotemperature
  VERSION 1.0.0
  DESCRIPTION "Temperature gatherer and HTTP server"
  HOMEPAGE_URL "https://cmake.org/"
  LANGUAGES CXX C
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(TARGET yoyo_temperature)

# Dependencies
include("${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake")
set(CONSTANT_HEADER "${CMAKE_SOURCE_DIR}/include/constants.h.in")
include("${CMAKE_CURRENT_SOURCE_DIR}/../env.cmake")

# Source files
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_executable(
	${TARGET}
	${SOURCES}/database.cpp
	${SOURCES}/thermometer.cpp
	${SOURCES}/server.cpp
  	${SOURCES}/main.cpp
)
target_include_directories(
	${TARGET} PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${CMAKE_BINARY_DIR}/generated"
)
target_link_libraries(
	${TARGET} PRIVATE
	Crow::Crow
	SQLite3::SQLite3
)

# Compilation depending on the platform
if(MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
    	target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:WINDOWS")
    	target_compile_options(${TARGET} PRIVATE /W3 /WX )
	else()
    	target_compile_options(${TARGET} PRIVATE /W3 /WX /DEBUG )
	endif()
else()
	target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Output folder
set_target_properties(${TARGET}
	PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
