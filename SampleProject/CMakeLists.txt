cmake_minimum_required(VERSION 4.2)

set(libs "${CMAKE_SOURCE_DIR}/libs")
set(src "${CMAKE_SOURCE_DIR}/src")

set(TXGame "${CMAKE_SOURCE_DIR}/../")
set(CMakeUtils "${TXGame}/TXCMakeUtils")

include("${CMakeUtils}/optimization.cmake")
include("${CMakeUtils}/link_dir_to_bin.cmake")

project(TXProject)

# install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix to ~/.local" FORCE)
endif()

# dependencies

add_subdirectory("${TXGame}" "${CMAKE_BINARY_DIR}/libs/TXGame")

add_executable(TXProject 
	"${src}/main.cpp"
	"${src}/Project.hpp"
	"${src}/utility_dump.hpp")
target_include_directories(TXProject PRIVATE
	"${CMAKE_SOURCE_DIR}"
	"${libs}"
	"${src}"
)

# project settings
target_compile_features(TXProject PUBLIC cxx_std_20)

# dependencies
target_link_libraries(TXProject PRIVATE 
	TXGame
)

# optimization
tx_add_release_ops(TXProject)

# build
set_target_properties(TXProject PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)
tx_link_dir_to_bin(TXProject "${CMAKE_SOURCE_DIR}/assets")

install(TARGETS TXProject
    RUNTIME DESTINATION bin
)