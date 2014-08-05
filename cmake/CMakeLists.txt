#
# Note: This is the first time I have ever written a cmake file,
# so this may 1) not work and 2) be horrible coding.
#

cmake_minimum_required(VERSION 2.6)
project(FREETRON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(TIFF)
find_package(DevIL)
find_package(Podofo)
find_package(CppDB)
find_package(CppCMS)
find_program(EXE_TMPL_CC cppcms_tmpl_cc)
find_program(EXE_MAKE_KEY cppcms_make_key)
include_directories("${TIFF_INCLUDE_DIR}")
include_directories("${IL_INCLUDE_DIR}")
include_directories("${PODOFO_INCLUDE_DIR}")
include_directories("${CPPDB_INCLUDE_DIR}")
include_directories("${CPPCMS_INCLUDE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -funroll-loops -O3 -std=c++11")

# Eventually detect this from the options.h file? Or, maybe, modify the
# options.h file here (definitely not until this is actually tested on a few
# different systems).
set(DEBUG true)

if (DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    endif()
endif()

# For C++11 threading support on a Unix system
if(UNIX)
    set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -pthreads")
endif()

# CppCMS views
set(WEBSITE "${CMAKE_CURRENT_SOURCE_DIR}/../website")
file(GLOB TEMPLATES "${CMAKE_CURRENT_SOURCE_DIR}/../website/*.tmpl")
list(REMOVE_ITEM TEMPLATES "${WEBSITE}/master.tmpl")
add_custom_command(
    OUTPUT "${WEBSITE}/skin.cpp"
    COMMAND ${EXE_TMPL_CC}
        "${WEBSITE}/master.tmpl"
        ${TEMPLATES}
        -o "${WEBSITE}/skin.cpp"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${TEMPLATES})

# C++ files
file(GLOB src "${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/../freetron.cpp")
list(APPEND src "${WEBSITE}/skin.cpp")
file(GLOB websrc "${WEBSITE}/*.cpp")
add_executable(freetron ${src} ${websrc} "${CMAKE_CURRENT_SOURCE_DIR}/../freetron.cpp")

string(REGEX REPLACE "tiff" "tiffxx" TIFFXX_LIBRARY ${TIFF_LIBRARY})
target_link_libraries(freetron "${TIFF_LIBRARY}")
target_link_libraries(freetron "${TIFFXX_LIBRARY}")
target_link_libraries(freetron "${IL_LIBRARIES}")
target_link_libraries(freetron "${PODOFO_LIBRARY}")
target_link_libraries(freetron "${CPPDB_LIBRARY}")
target_link_libraries(freetron "${CPPCMS_LIBRARY}")
target_link_libraries(freetron "${BOOSTER_LIBRARY}")

# Install application
install(TARGETS freetron
    RUNTIME DESTINATION bin)

# Create keys if they don't exist
if(NOT EXISTS "${WEBSITE}/hmac.txt")
    add_custom_command(
        OUTPUT "${WEBSITE}/hmac.txt"
               "${WEBSITE}/cbc.txt"
        COMMAND "${EXE_MAKE_KEY}" --hmac sha256 --cbc aes256
        --hmac-file "${WEBSITE}/hmac.txt"
        --hmac-cbc  "${WEBSITE}/cbc.txt"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Website files
file(GLOB MINFILES "${WEBSITE}/files/*.min.*")
add_custom_target(keys ALL DEPENDS
    "${WEBSITE}/hmac.txt"
    "${WEBSITE}/cbc.txt")

# Blank uploads/ directory
file(MAKE_DIRECTORY "${WEBSITE}/uploads")

# Install website files
install(FILES
    "${WEBSITE}/hmac.txt"
    "${WEBSITE}/cbc.txt"
    "${WEBSITE}/config.js"
    DESTINATION /srv/freetron)
install(FILES
    ${MINFILES}
    "${WEBSITE}/files/form.pdf"
    DESTINATION /srv/freetron/files)
install(DIRECTORY "${WEBSITE}/uploads"
    DESTINATION /srv/freetron)

install(CODE "MESSAGE(\"To start website, run:\")")
install(CODE "MESSAGE(\"  ${CMAKE_INSTALL_PREFIX}/bin/freetron --daemon /srv/freetron\")")
