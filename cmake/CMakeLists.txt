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
include_directories("${TIFF_INCLUDE_DIR}")
include_directories("${IL_INCLUDE_DIR}")
include_directories("${PODOFO_INCLUDE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -funroll-loops -O2 -std=c++11")

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

# Yes, I know this is "horrible." So, I have to rerun `cmake .' if I add a
# file... who cares.
file(GLOB src "${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}../freetron.cpp")
add_executable(freetron ${src} "${CMAKE_CURRENT_SOURCE_DIR}/../freetron.cpp")

string(REGEX REPLACE "tiff" "tiffxx" TIFFXX_LIBRARY ${TIFF_LIBRARY})
target_link_libraries(freetron "${TIFF_LIBRARY}")
target_link_libraries(freetron "${TIFFXX_LIBRARY}")
target_link_libraries(freetron "${IL_LIBRARIES}")
target_link_libraries(freetron "${PODOFO_LIBRARY}")
