cmake_minimum_required(VERSION 2.10)

project(gpa-calculator)
set(CMAKE_BUILD_TYPE Release)

# Include headers
include_directories(dep)

# Add all .c files
file(GLOB SOURCES "src/*.c")

add_executable(gpa-calculator ${SOURCES})