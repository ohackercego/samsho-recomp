# samshosen - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(samshosen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(SAMSHOSEN_SOURCES
    src/main.cpp
    src/stubs.cpp
    src/dispatch_tolerance.cpp
    src/crash_diag.cpp
    src/overrides.cpp
)

if(WIN32)
    add_executable(samshosen WIN32 ${SAMSHOSEN_SOURCES})
    target_link_options(samshosen PRIVATE "LINKER:/force:multiple")
    target_link_libraries(samshosen PRIVATE dbghelp)
else()
    add_executable(samshosen ${SAMSHOSEN_SOURCES})
endif()

rexglue_setup_target(samshosen GPU_PLUGINS xenos)
