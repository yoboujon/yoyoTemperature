cmake_minimum_required(VERSION 3.11)

set(ENV_FILE "${CMAKE_CURRENT_LIST_DIR}/.env")
if(NOT EXISTS "${ENV_FILE}")
    message(FATAL_ERROR ".env file not found at ${ENV_FILE}")
endif()

file(STRINGS "${ENV_FILE}" ENV_LINES)
foreach(line ${ENV_LINES})
    if(line MATCHES "^[ \t]*$" OR line MATCHES "^[ \t]*#")
        continue()
    endif()
    string(FIND "${line}" "=" eq_pos)
    if(eq_pos EQUAL -1)
        continue()
    endif()

    string(SUBSTRING "${line}" 0 ${eq_pos} key)
    math(EXPR value_start "${eq_pos} + 1")
    string(SUBSTRING "${line}" ${value_start} -1 value)
    string(REGEX REPLACE "^\"(.*)\"$" "\\1" value "${value}")
    string(REGEX REPLACE "^'(.*)'$" "\\1" value "${value}")

    set(ENV_${key} "${value}")
endforeach()

foreach(required_key WIFI_SSID WIFI_PASSWORD API_IP UDP_PORT API_PORT)
    if(NOT DEFINED ENV_${required_key})
        message(FATAL_ERROR "${required_key} missing from .env")
    endif()
endforeach()

set(WIFI_SSID     "${ENV_WIFI_SSID}")
set(WIFI_PASSWORD "${ENV_WIFI_PASSWORD}")
set(API_IP        "${ENV_API_IP}")
set(UDP_PORT      "${ENV_UDP_PORT}")
set(API_PORT      "${ENV_API_PORT}")

configure_file(
    "${CONSTANT_HEADER}"
    "${CMAKE_BINARY_DIR}/generated/constants.h"
    @ONLY
)
include_directories("${CMAKE_BINARY_DIR}/generated")
