cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ImageSpaceRefractions C CXX)

# Set this before including framework such that it knows to use the OpenGL4.5 version of GLAD
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	add_subdirectory("framework") # Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
else()
	add_subdirectory("../../../framework/" "${CMAKE_BINARY_DIR}/framework/") # During development the framework lives in parent folder.
endif()

# Additional source files
add_library(RefractionsLib "")
enable_sanitizers(RefractionsLib)
set_project_warnings(RefractionsLib)
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
target_include_directories(RefractionsLib PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/")
target_compile_features(RefractionsLib PUBLIC cxx_std_23)
target_link_libraries(RefractionsLib PUBLIC CGFramework)

# Main executable config
add_executable(RefractionsExec "src/main.cpp")
enable_sanitizers(RefractionsExec)
set_project_warnings(RefractionsExec)
target_compile_features(RefractionsExec PUBLIC cxx_std_23)
target_link_libraries(RefractionsExec PRIVATE RefractionsLib)

# Preprocessor definitions for paths
target_compile_definitions(RefractionsLib PUBLIC
	"-DRESOURCES_DIR=\"${CMAKE_CURRENT_LIST_DIR}/resources/\""
	"-DSHADERS_DIR=\"${CMAKE_CURRENT_LIST_DIR}/shaders/\"")
