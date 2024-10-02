cmake_minimum_required(VERSION 3.9)
project(mpt LANGUAGES C CXX VERSION 0.1)

if (vamos_DIR AND NOT vamos-common_DIR)
	set(vamos-common_DIR "${vamos_DIR}/vamos-common/cmake/vamos-common")
	get_filename_component(vamos-common_DIR ${vamos-common_DIR} ABSOLUTE)
	message(STATUS "Inferring vamos-common_DIR from vamos_DIR: ${vamos-common_DIR}")
endif()

find_package(vamos-common REQUIRED)
find_package(vamos-sources)

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "Build type not set. Setting Debug")
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

OPTION(ENABLE_IPO "Enable interprocedural optimizations" ON)
if (ENABLE_IPO)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(vamos-common_DIR ${vamos-common_DIR} ABSOLUTE)
if (vamos-sources_DIR)
	get_filename_component(vamos-sources_DIR ${vamos-sources_DIR} ABSOLUTE)
endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.py.in
	       ${CMAKE_CURRENT_SOURCE_DIR}/config.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.sh.in
	       ${CMAKE_CURRENT_SOURCE_DIR}/config.sh)

enable_testing()
add_subdirectory(tests)
