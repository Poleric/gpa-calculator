cmake_minimum_required(VERSION 3.12)

project(gpa-calculator)
set(CMAKE_BUILD_TYPE Debug)

# Include headers
include_directories(dep)

set(SOURCES
        ./src/sqlite3.c
        ./src/db.c
        ./src/calculation.c
        ./src/utils.c
        ./src/gui.c
)
add_executable(gpa-calculator ${SOURCES})

# Use PDCurses if WIN else ncurses
if(WIN32 OR MSVC)
        find_package(unofficial-pdcurses CONFIG REQUIRED)
        target_link_libraries(gpa-calculator PRIVATE unofficial::pdcurses::pdcurses)
else()
        find_package(Curses REQUIRED)
        include_directories(${CURSES_INCLUDE_DIRS})
        target_link_libraries(gpa-calculator PRIVATE ${CURSES_LIBRARIES})
endif()

