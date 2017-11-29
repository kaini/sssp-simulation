cmake_minimum_required (VERSION 3.5)

# Project setup
###############
project (sssp-simulation)
enable_testing ()

# Compiler setup
################
# Try to find the host architecture. This is only used for some path hints on
# Linux and it does not matter if it fails.
if (MSVC)
	set (${C_ARCH} "c_arch_not_set_when_using_msvc")
else ()
	execute_process (COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE C_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

set (CMAKE_CXX_STANDARD 14)
# TODO With cmake 3.10+ this if can be removed.
if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
endif ()

# Find external libraries
#########################
find_package (Boost REQUIRED date_time program_options unit_test_framework REQUIRED)

find_path (Cairo_INCLUDE_DIRS cairo.h HINTS /usr/include/cairo)
find_library (Cairo_LIBRARIES cairo)

find_path (Freetype_INCLUDE_DIRS ft2build.h HINTS /usr/include/freetype2)

find_path (Sigcpp_INCLUDE_DIR_1 sigc++/sigc++.h HINTS /usr/include/sigc++-2.0)
find_path (Sigcpp_INCLUDE_DIR_2 sigc++config.h HINTS /usr/lib/${C_ARCH}/sigc++-2.0/include)
set (Sigcpp_INCLUDE_DIRS "${Sigcpp_INCLUDE_DIR_1}" "${Sigcpp_INCLUDE_DIR_2}")

find_path (Cairomm_INCLUDE_DIR_1 cairomm/cairomm.h HINTS /usr/include/cairomm-1.0)
find_path (Cairomm_INCLUDE_DIR_2 cairommconfig.h HINTS /usr/lib/${C_ARCH}/cairomm-1.0/include)
set (Cairomm_INCLUDE_DIRS "${Cairomm_INCLUDE_DIR_1}" "${Cairomm_INCLUDE_DIR_2}")
find_library (Cairomm_LIBRARIES cairomm-1.0 NAMES cairomm-1 cairomm)

find_path (Tbb_INCLUDE_DIR tbb/parallel_for.h)
find_library (Tbb_LIBRARIES_Debug tbb_debug)
find_library (Tbb_LIBRARIES tbb)

# sssp-simulation library
#########################
set (HEADERS arguments.hpp crauser.hpp dijkstra.hpp draw_graph.hpp generate_edges.hpp generate_positions.hpp graph.hpp math.hpp partial_shuffle.hpp stringy_enum.hpp)
set (SOURCES arguments.cpp crauser.cpp dijkstra.cpp draw_graph.cpp generate_edges.cpp generate_positions.cpp graph.cpp math.cpp)
add_library (sssp STATIC ${HEADERS} ${SOURCES})
target_compile_definitions (sssp PUBLIC -D_USE_MATH_DEFINES)
target_include_directories (sssp PUBLIC ${Boost_INCLUDE_DIRS} ${Cairo_INCLUDE_DIRS} ${Freetype_INCLUDE_DIRS} ${Sigcpp_INCLUDE_DIRS} ${Cairomm_INCLUDE_DIRS} ${Tbb_INCLUDE_DIR})
target_include_directories (sssp INTERFACE .)
target_link_libraries (sssp PUBLIC ${Boost_LIBRARIES} ${Cairo_LIBRARIES} ${Cairomm_LIBRARIES})
if (MSVC)
	target_link_libraries (sssp PUBLIC optimized ${Tbb_LIBRARIES})
	target_link_libraries (sssp PUBLIC debug ${Tbb_LIBRARIES_Debug})
else ()
	target_link_libraries (sssp PUBLIC ${Tbb_LIBRARIES})
endif ()

# sssp-simulation executable
############################
add_executable (sssp-simulation main.cpp)
target_link_libraries (sssp-simulation sssp)

# tests executable
##################
set (TESTS_SOURCES tests/crauser_test.cpp tests/dijkstra_test.cpp tests/graph_test.cpp tests/math_test.cpp tests/test.cpp)
add_executable (tests ${TESTS_SOURCES})
target_compile_definitions (tests PUBLIC -DBOOST_TEST_DYN_LINK)
target_link_libraries (tests sssp)
add_test (sssp-tests tests)
