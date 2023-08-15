cmake_minimum_required(VERSION 3.12)

project(gpa-calculator)
set(CMAKE_BUILD_TYPE Debug)

# Include headers
include_directories(dep)

# Add all .c files in ./src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

add_executable(gpa-calculator ${SOURCES})