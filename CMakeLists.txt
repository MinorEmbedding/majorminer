cmake_minimum_required(VERSION 3.7)

project(majorminer)

option(MAJORMINER_BUILD_TESTS       "Build majorminer test executable"      ON)
option(MAJORMINER_BUILD_EXAMPLES    "Build majorminer example executable"   ON)
enable_language(C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(MM_INCLUDE_LIBS external/oneTBB/include external/eigen build/external/lemon/ external/lemon/ external/ src/)
set(MM_LINK_LIBS TBB::tbb lemon)

# neither build tests nor tbbmalloc in oneTBB
set(TBB_TEST CACHE BOOL OFF)
set(TBBMALLOC_BUILD CACHE BOOL OFF)
set(TBB_DISABLE_HWLOC_AUTOMATIC_SEARCH CACHE BOOL OFF)

set(LEMON_ENABLE_GLPK CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_COIN CACHE BOOL OFF)
set(LEMON_ENABLE_SOPLEX CACHE BOOL OFF)


# build NetworkSimplex, gtest and oneTBB
add_subdirectory(external)


if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -fdiagnostics-color=always")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4 /WX")
endif()

add_library(majorminer)


target_include_directories(majorminer PRIVATE ${MM_INCLUDE_LIBS})

add_subdirectory(src)
target_link_libraries(majorminer ${MM_LINK_LIBS})

if ( ${MAJORMINER_BUILD_TESTS} )
    add_executable(majorminer_test)
    add_subdirectory(test)
    target_include_directories(majorminer_test PRIVATE src ${MM_INCLUDE_LIBS})
    target_link_libraries(majorminer_test majorminer gtest_main)
endif()

if ( ${MAJORMINER_BUILD_EXAMPLES} )
    add_subdirectory(examples)
endif()