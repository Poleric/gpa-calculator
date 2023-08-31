cmake_minimum_required(VERSION 3.12)

project(gpa-calculator)
set(CMAKE_BUILD_TYPE Debug)
find_package(Curses REQUIRED)

# Include headers
include_directories(./dep/)
include_directories(${CURSES_INCLUDE_DIRS})

set(SOURCES
        ./src/sqlite3.c
        ./src/db.c
        ./src/calculation.c
        ./src/utils.c
        ./src/gui.c
)

add_executable(gpa-calculator ${SOURCES})
target_link_libraries(gpa-calculator ${CURSES_LIBRARIES})