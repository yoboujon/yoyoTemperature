cmake_minimum_required(VERSION 3.11)

include(FetchContent)

# Sqlite3
find_package(SQLite3 REQUIRED)
if(NOT TARGET SQLite3::SQLite3) # CMake < 4.3
  add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
endif()

# CrowCpp
FetchContent_Declare(
	crowcpp
	GIT_REPOSITORY https://github.com/crowcpp/crow.git
	GIT_TAG        v1.3.3
)
FetchContent_GetProperties(crowcpp)
if (NOT crowcpp_POPULATED)
	set(FETCHCONTENT_QUIET NO)
	FetchContent_MakeAvailable(crowcpp)
endif()
