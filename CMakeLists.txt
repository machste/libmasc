cmake_minimum_required(VERSION 3.5)

project(masc VERSION 1.2.0)

include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(UNIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif(UNIX)

option(DEBUG "Enable Debugging (GDB support and no optimisation)." OFF)
option(LOG_ENABLED "Use logging for more detailed information." ON)

if(DEBUG)
	add_definitions(-O0 -ggdb3 -DDEBUG)
else()
	add_definitions(-Os -g)
endif()
add_definitions(-Wall -Werror -std=gnu11 -fplan9-extensions)

file(GLOB HEADERS include/masc/*.h)
file(GLOB_RECURSE SOURCES src/*.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${HEADERS}")

target_include_directories(${PROJECT_NAME} PUBLIC include)

configure_file(lib${PROJECT_NAME}.pc.in lib${PROJECT_NAME}.pc @ONLY)

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)
add_definitions("-include config.h")

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/masc)

install(FILES "include/masc.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/lib${PROJECT_NAME}.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

# Setup examples
file(GLOB EXAMPLE_SRCS example/*.c)

foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)

    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})

    target_link_libraries(${EXAMPLE_NAME} ${PROJECT_NAME})

    set_target_properties(${EXAMPLE_NAME} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/example_bin)
endforeach(EXAMPLE_SRC)

# Setup testing
enable_testing()

file(GLOB TEST_SRCS test/*.c)

find_program(VALGRIND valgrind)

set(VALGRIND_FLAGS --tool=memcheck --leak-check=full --error-exitcode=1)

foreach(TEST_SRC ${TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SRC})

    target_link_libraries(${TEST_NAME} ${PROJECT_NAME})

    set_target_properties(${TEST_NAME} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/test_bin)

    add_test(NAME ${TEST_NAME} 
             WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin 
             COMMAND ${CMAKE_BINARY_DIR}/test_bin/${TEST_NAME} )
    if(VALGRIND)
        add_test(NAME ${TEST_NAME}.valgrind 
                 WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin 
                 COMMAND ${VALGRIND} ${VALGRIND_FLAGS}
                 ${CMAKE_BINARY_DIR}/test_bin/${TEST_NAME} )
    endif(VALGRIND)
endforeach(TEST_SRC)
