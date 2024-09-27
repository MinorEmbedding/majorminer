cmake_minimum_required(VERSION 3.7)

project(example_majorminer)

enable_language(C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(MM_INCLUDES ../include/ ../src/ ../src/common/ ../src/lmrp/ ../src/evolutionary/ ../src/initial ../)
set(INCLUDE_LIBS ../external/oneTBB/include ../build/external/lemon/ ../external/lemon/ ../external/ ${MM_INCLUDES})
set(LINK_LIBS TBB::tbb lemon majorminer pthread)

# neither build tests nor tbbmalloc in oneTBB
set(TBB_TEST CACHE BOOL OFF)
set(TBBMALLOC_BUILD CACHE BOOL OFF)
set(TBB_DISABLE_HWLOC_AUTOMATIC_SEARCH CACHE BOOL OFF)

set(LEMON_ENABLE_GLPK CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_COIN CACHE BOOL OFF)
set(LEMON_ENABLE_SOPLEX CACHE BOOL OFF)




if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -fdiagnostics-color=always")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4 /WX")
endif()

add_executable(example_majorminer example_majorminer.cpp)

target_include_directories(example_majorminer PRIVATE ${INCLUDE_LIBS})
target_link_libraries(example_majorminer ${LINK_LIBS})
