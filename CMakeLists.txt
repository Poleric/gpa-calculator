cmake_minimum_required(VERSION 3.12)

project(gpa-calculator)
set(CMAKE_BUILD_TYPE Debug)

# Include headers
include_directories(dep)

set(SOURCES
        ./src/sqlite3.c
        ./src/db.c
        ./src/calculation.c
        ./src/gui.c
        ./src/utils.c
        src/main_menu.c
)
add_executable(gpa-calculator ${SOURCES})


find_package(Curses REQUIRED)
# Use PDCurses if WIN else ncurses
if (Curses_FOUND)
    include_directories(${CURSES_INCLUDE_DIRS})
    target_link_libraries(gpa-calculator PRIVATE ${CURSES_LIBRARIES})
elseif (WIN32 OR MSVC)
     find_package(unofficial-pdcurses CONFIG REQUIRED)
     target_link_libraries(gpa-calculator PRIVATE unofficial::pdcurses::pdcurses)
 else ()
     message(STATUS "Curses Not Found!")
endif()

find_package(Gettext REQUIRED)
if (!Gettext_FOUND)
    find_package(unofficial-gettext CONFIG REQUIRED)
    target_link_libraries(main PRIVATE unofficial::gettext::libintl)
endif()