cmake_minimum_required(VERSION 2.6)

project(bacon)
find_package(CURL)

include(CheckIncludeFiles)
include(CheckFunctionExists)

check_include_files(inttypes.h HAVE_INTTYPES_H)
check_function_exists(malloc HAVE_MALLOC)
check_include_files(memory.h HAVE_MEMORY_H)
check_function_exists(memset HAVE_MEMSET)
check_function_exists(mkdir HAVE_MKDIR)
check_function_exists(realloc HAVE_REALLOC)
check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_function_exists(strrchr HAVE_STRRCHR)
check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

set(bacon_SOURCES
    bacon-actions.cpp
    bacon-cmd.cpp
    bacon-device.cpp
    bacon-devicelist.cpp
    bacon-env.cpp
    bacon-file.cpp
    bacon-htmldoc.cpp
    bacon-htmlparser.cpp
    bacon-main.cpp
    bacon-md5.cpp
    bacon-net.cpp
    bacon-progress.cpp
    bacon-rom.cpp
    bacon-util.cpp)

set(bacon_HEADERS
    config.h
    bacon-actions.h
    bacon-cmd.h
    bacon-device.h
    bacon-devicelist.h
    bacon-env.h
    bacon-file.h
    bacon-htmldoc.h
    bacon-htmlparser.h
    bacon-md5.h
    bacon-net.h
    bacon-progress.h
    bacon-rom.h
    bacon-util.h)

add_executable(bacon ${bacon_SOURCES} ${bacon_HEADERS})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(bacon ${CURL_LIBRARIES})
endif(CURL_FOUND)
install(TARGETS bacon RUNTIME DESTINATION bin)
