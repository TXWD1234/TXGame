# Copyright@TXGame All rights reserved.
# Author: TX Studio: TX_Jerry

cmake_minimum_required(VERSION 4.2)

# to defence being included multiple times
if(TARGET TXGame)
	return()
endif()

# add txlib
set(lib "${CMAKE_CURRENT_SOURCE_DIR}/dependency")
include("${lib}/add_txlib.cmake")
tx_add_txlib()

set(CMakeUtils "${TXLib}/TXCMakeUtils")
include("${CMakeUtils}/optimization.cmake")


# Automatically collect all nested headers and source files
file(GLOB_RECURSE TX_GRAPHICS_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE TX_GRAPHICS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(TXGame STATIC ${TX_GRAPHICS_HEADERS} ${TX_GRAPHICS_SOURCES})

tx_add_release_ops(TXGame)
target_link_libraries(TXGame PUBLIC
	TXMath
	TXGraphics
	TXData
	TXResource
	TXGrid
	TXJson
)

target_compile_features(TXGame PUBLIC cxx_std_20)

target_include_directories(TXGame PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)