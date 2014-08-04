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
file(GLOB TEMPLATES "${CMAKE_CURRENT_SOURCE_DIR}/../website/*.tmpl")
list(REMOVE_ITEM TEMPLATES "${CMAKE_CURRENT_SOURCE_DIR}/../website/master.tmpl")
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../website/skin.cpp"
    COMMAND ${EXE_TMPL_CC}
        ${CMAKE_CURRENT_BINARY_DIR}/../website/master.tmpl
        ${TEMPLATES}
        -o "${CMAKE_CURRENT_BINARY_DIR}/../website/skin.cpp"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${TEMPLATES})

# C++ files
file(GLOB src "${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}../freetron.cpp")
list(APPEND src "${CMAKE_CURRENT_SOURCE_DIR}/../website/skin.cpp")
file(GLOB websrc "${CMAKE_CURRENT_SOURCE_DIR}/../website/*.cpp")
add_executable(freetron ${src} ${websrc} "${CMAKE_CURRENT_SOURCE_DIR}/../freetron.cpp")

string(REGEX REPLACE "tiff" "tiffxx" TIFFXX_LIBRARY ${TIFF_LIBRARY})
target_link_libraries(freetron "${TIFF_LIBRARY}")
target_link_libraries(freetron "${TIFFXX_LIBRARY}")
target_link_libraries(freetron "${IL_LIBRARIES}")
target_link_libraries(freetron "${PODOFO_LIBRARY}")
target_link_libraries(freetron "${CPPDB_LIBRARY}")
target_link_libraries(freetron "${CPPCMS_LIBRARY}")
target_link_libraries(freetron "${BOOSTER_LIBRARY}")

# Install
install(TARGETS freetron
    RUNTIME DESTINATION bin)

# Website JS/CSS files
file(GLOB MINFILES "${CMAKE_CURRENT_SOURCE_DIR}/../website/files/*.min.*")
install(FILES ${MINFILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../website/files/form.pdf
    DESTINATION share/freetron/files)

# Config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../website/config.js.example
    ${CMAKE_CURRENT_SOURCE_DIR}/../website/config.js COPYONLY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../website/config.js
    DESTINATION share/freetron)

# Blank uploads/ directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../website/uploads)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../website/uploads
    DESTINATION share/freetron)

install(CODE "MESSAGE(\"To start website, run this to generate keys for config.js:\")")
install(CODE "MESSAGE(\"  cppcms_make_key --hmac sha256 --cbc aes256\")")
install(CODE "MESSAGE(\"  $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/freetron --daemon $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/freetron\")")
