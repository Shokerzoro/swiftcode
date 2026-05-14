cmake_minimum_required(VERSION 3.10)
project(swiftcode)

set(CMAKE_CXX_STANDARD 17)

include(src/domain/sources.cmake)
include(src/enum/sources.cmake)
include(src/sql/sources.cmake)
include(src/verbose/sources.cmake)

add_executable(swiftcode
        src/main.cpp
        ${DOMAIN_SOURCES}
        ${ENUM_SOURCES}
        ${SQL_SOURCES}
        ${VERBOSE_SOURCES}
)


if(WIN32)
    set(SWIFTCODE_DEPLOY_PLATFORM windows)
elseif(UNIX)
    set(SWIFTCODE_DEPLOY_PLATFORM linux)
else()
    message(FATAL_ERROR "Unsupported deploy platform")
endif()

install(TARGETS swiftcode
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/deploy/${SWIFTCODE_DEPLOY_PLATFORM}
        CONFIGURATIONS Release
)
