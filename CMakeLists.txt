# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# enable testing
enable_testing()

# project name and language
project(comp_decomp CXX)

# check build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# make uppercase string of build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

# check build type again
if (NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

## set output directory (build/)
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

## enable Win32 resource
#enable_language(RC)

if (WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # using Clang
        set(CMAKE_C_FLAGS "-static")
        set(CMAKE_CXX_FLAGS "-static")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        set(CMAKE_C_FLAGS "-static -static-libgcc -static-libstdc++")
        set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # replace "/MD" with "/MT" (building without runtime DLLs)
        set(CompilerFlags
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        foreach(CompilerFlags ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
        endforeach()
    endif()
endif()

##############################################################################

# zlib
find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIRS})
if (ZLIB_FOUND)
    add_definitions(-DHAVE_ZLIB)
    message(STATUS "ZLIB found.")
else()
    message(STATUS "ZLIB not found.")
endif()
#add_definitions(-DZLIB_DLL)  # dynamic

# bzip2
find_package(BZip2)
include_directories(${BZIP2_INCLUDE_DIR})
if (BZIP2_FOUND)
    add_definitions(-DHAVE_BZLIB)
    message(STATUS "BZIP2 found")
else()
    message(STATUS "BZIP2 not found.")
endif()

# lzma
find_package(LibLZMA)
include_directories(${LIBLZMA_INCLUDE_DIRS})
if (LIBLZMA_FOUND)
    add_definitions(-DHAVE_LZMA)
    message(STATUS "LZMA found")
else()
    message(STATUS "LZMA not found.")
endif()
add_definitions(-DLZMA_API_STATIC)  # static

# executable
add_executable(comp_decomp_test comp_decomp_test.cpp)

# link
target_link_libraries(
    comp_decomp_test
    ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${LIBLZMA_LIBRARIES})

# threading
find_package(Threads REQUIRED)
target_link_libraries(comp_decomp_test Threads::Threads)

# add test
add_test(NAME comp_decomp_test
         COMMAND $<TARGET_FILE:comp_decomp_test>)

# C++11
set_property(TARGET comp_decomp_test PROPERTY CXX_STANDARD 11)

##############################################################################
