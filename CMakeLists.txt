# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(win32_emoji_draw CXX RC)

# statically linked
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    set(CMAKE_C_FLAGS "-static")
    set(CMAKE_CXX_FLAGS "-static")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_C_FLAGS "-static")
    set(CMAKE_CXX_FLAGS "-static")
elseif (MSVC)
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

##############################################################################

# dwrite_ver.exe
add_executable(dwrite_ver WIN32 dwrite_ver.cpp resource.rc)
target_link_libraries(dwrite_ver PRIVATE gdi32 user32 comctl32 d2d1 dwrite)

# gdi_ver.exe
add_executable(gdi_ver WIN32 gdi_ver.cpp resource.rc)
target_link_libraries(gdi_ver PRIVATE gdi32 user32 comctl32)

##############################################################################
