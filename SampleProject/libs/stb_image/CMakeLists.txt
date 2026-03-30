# stb_image - v2.30 - public domain image loader - http://nothings.org/stb
#                                no warranty implied; use at your own risk

cmake_minimum_required(VERSION 4.2)

# to defence being included multiple times
if(TARGET stb_image)
	return()
endif()

add_library(stb_image STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image.cpp"
)

target_include_directories(stb_image PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)