cmake_minimum_required(VERSION 3.12)

project(gpa-calculator VERSION 1.0)
set(CMAKE_BUILD_TYPE Debug)

# Include headers
include_directories(dep)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/sqlite3.c
    ${CMAKE_SOURCE_DIR}/src/db.c
    ${CMAKE_SOURCE_DIR}/src/calculation.c
    ${CMAKE_SOURCE_DIR}/src/gui.c
    ${CMAKE_SOURCE_DIR}/src/utils.c
    ${CMAKE_SOURCE_DIR}/src/main_menu.c
)

if (WIN32 OR MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")  # chinese characters shenaningans
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

add_compile_definitions("LOCALE_DIR=\"${CMAKE_SOURCE_DIR}/i18n\"")
add_executable(gpa-calculator ${SOURCES})

if (WIN32 OR MSVC)
    message(STATUS "WINDOWS machine detected.")
    add_compile_definitions("_CRT_SECURE_NO_WARNINGS")

    message(STATUS "Loading PDCurses")
    find_package(unofficial-pdcurses CONFIG REQUIRED)
    target_link_libraries(gpa-calculator PRIVATE unofficial::pdcurses::pdcurses)

    message(STATUS "Loading Intl WIN64")
    find_package(Intl REQUIRED)
    target_link_libraries(gpa-calculator PRIVATE Intl::Intl)

else()  # GNU system
    message(STATUS "GNU LINUX system assumed.")

    find_package(Curses REQUIRED)
    if (Curses_FOUND)
        message(STATUS "ncurses found:")
        message(STATUS " CURSES_INCLUDE_DIR: ${CURSES_INCLUDE_DIRS}")
        message(STATUS " CURSES_LIBRARIES: ${CURSES_LIBRARIES}")
        message(STATUS " Version: ${CURSES_VERSION}")
        include_directories(${CURSES_INCLUDE_DIRS})
        target_link_libraries(gpa-calculator PRIVATE ${CURSES_LIBRARIES})
    else ()
        message(STATUS "Curses Not Found!")
    endif()

    # Setting up Internationalisation (i18n)
    find_package(Intl REQUIRED)
    if (Intl_FOUND)
        message(STATUS "Internationalization (i18n) found:")
        message(STATUS " INTL_INCLUDE_DIRS: ${Intl_INCLUDE_DIRS}")
        message(STATUS " INTL_LIBRARIES: ${Intl_LIBRARIES}")
        message(STATUS " Version: ${Intl_VERSION}")
        include_directories(${Intl_INCLUDE_DIRS})
        link_directories(${Intl_LIBRARY_DIRS})
    else ()
        message(STATUS "Internationalization (i18n) Not found!")
    endif ()

    find_package(Gettext REQUIRED)
    if (Gettext_FOUND)
        message(STATUS "Gettext found:")
        message(STATUS " Version: ${GETTEXT_VERSION_STRING}")
    else ()
        message(STATUS "Gettext Not found!")
    endif ()
endif()

# TODO: update pot, po, and compile mo files on build