cmake_minimum_required(VERSION 3.5)

project(masc VERSION 1.0.0)

include(GNUInstallDirs)

set(CMAKE_C_FLAGS "-g -std=gnu99 -Wall -Werror")

file(GLOB HEADERS include/masc/*.h)
file(GLOB SOURCES src/*.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${HEADERS}")

target_include_directories(${PROJECT_NAME} PUBLIC include)


configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)

if(UNIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif(UNIX)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


# Setup testing
enable_testing()

file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/test_*.c)

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