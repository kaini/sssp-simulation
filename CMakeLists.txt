cmake_minimum_required (VERSION 3.5)

# Project setup
###############
project (sssp-simulation)
include (FindPkgConfig)
enable_testing ()

option (DISABLE_CAIRO "disables the image output feature and therefore does not need Cairo anymore" OFF)

# Compiler setup
################
set (CMAKE_CXX_STANDARD 14)
# TODO With cmake 3.10+ this if can be removed.
if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++${CMAKE_CXX_STANDARD}")
	add_definitions (-D_USE_MATH_DEFINES -D_WIN32_WINNT=0x0A00 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
endif ()

# Find external libraries
#########################
find_package(Threads REQUIRED)

add_library (freetype INTERFACE)
if (PKG_CONFIG_FOUND)
	pkg_search_module (Freetype_HINTS freetype2)
endif ()
find_path (Freetype_INCLUDE_DIR ft2build.h HINTS ${Freetype_HINTS_INCLUDE_DIRS})
target_include_directories (freetype SYSTEM INTERFACE ${Freetype_INCLUDE_DIR})

add_library (cairo INTERFACE)
find_path (Cairo_INCLUDE_DIR cairo.h PATH_SUFFIXES cairo)
target_include_directories (cairo SYSTEM INTERFACE ${Cairo_INCLUDE_DIR})
find_library (Cairo_LIBRARY cairo)
target_link_libraries (cairo INTERFACE ${Cairo_LIBRARY} freetype)

add_library (sigc++ INTERFACE)
if (PKG_CONFIG_FOUND)
	pkg_search_module (Sigc++_HINTS sigc++-2.0)
endif ()
find_path (Sigc++_INCLUDE_DIR sigc++/sigc++.h HINTS ${Sigc++_HINTS_INCLUDE_DIRS})
find_path (Sigc++_Config_INCLUDE_DIR sigc++config.h HINTS ${Sigc++_HINTS_INCLUDE_DIRS})
target_include_directories (sigc++ SYSTEM INTERFACE ${Sigc++_HINTS_INCLUDE_DIRS} ${Sigc++_HINTS_INCLUDE_DIRS})

add_library (cairomm INTERFACE)
if (PKG_CONFIG_FOUND)
	pkg_search_module (Cairomm_HINTS cairomm-1.0)
endif ()
find_path (Cairomm_INCLUDE_DIR cairomm/cairomm.h HINTS ${Cairomm_HINTS_INCLUDE_DIRS})
find_path (Cairomm_Config_INCLUDE_DIR cairommconfig.h HINTS ${Cairomm_HINTS_INCLUDE_DIRS})
target_include_directories (cairomm SYSTEM INTERFACE ${Cairomm_INCLUDE_DIR} ${Cairomm_Config_INCLUDE_DIR})
find_library (Cairomm_LIBRARY cairomm-1.0 HINTS ${Cairomm_HINTS_LIBRARY_DIRS})
target_link_libraries (cairomm INTERFACE ${Cairomm_LIBRARY} cairo sigc++)

add_library (boost INTERFACE)
find_package (Boost 1.65.0 REQUIRED COMPONENTS date_time program_options unit_test_framework timer system chrono filesystem)
target_include_directories (boost SYSTEM INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries (boost INTERFACE ${Boost_LIBRARIES})
target_compile_definitions (boost INTERFACE -DBOOST_TEST_DYN_LINK)
if (MSVC)
	target_compile_definitions (boost INTERFACE -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
endif ()
if (UNIX AND NOT APPLE)
	target_link_libraries (boost INTERFACE rt)
endif ()

# sssp-simulation library
#########################
set (HEADERS
	arguments.hpp
	crit_crauser.hpp
	crit_dijkstra.hpp
	crit_heuristic.hpp
	crit_paper_in.hpp
	crit_paper_out.hpp
	crit_oracle.hpp
	crit_traff_bridge.hpp
	criteria.hpp
	dijkstra.hpp
	generate_edges.hpp
	generate_positions.hpp
	graph.hpp
	math.hpp
	partial_shuffle.hpp
	priority_queue.hpp
	run.hpp
	stringy_enum.hpp
)
set (SOURCES
	arguments.cpp
	crit_crauser.cpp
	crit_dijkstra.cpp
	crit_heuristic.cpp
	crit_paper_in.cpp
	crit_paper_out.cpp
	crit_oracle.cpp
	crit_traff_bridge.cpp
	dijkstra.cpp
	generate_edges.cpp
	generate_positions.cpp
	graph.cpp
	math.cpp
	run.cpp
)
if (NOT ${DISABLE_CAIRO})
	list (APPEND HEADERS draw_graph.hpp)
	list (APPEND SOURCES draw_graph.cpp)
endif ()
add_library (sssp STATIC ${HEADERS} ${SOURCES})
if (${DISABLE_CAIRO})
	target_compile_definitions (sssp PUBLIC -DDISABLE_CAIRO)
endif ()
target_link_libraries (sssp PUBLIC boost)
if (NOT ${DISABLE_CAIRO})
	target_link_libraries (sssp PUBLIC cairomm)
endif ()
target_include_directories (sssp INTERFACE .)

# sssp-simulation executable
############################
add_executable (sssp-simulation main_simulation.cpp)
target_link_libraries (sssp-simulation PRIVATE sssp)

# sssp-batch executable
#######################
add_executable (sssp-batch main_batch.cpp)
target_link_libraries (sssp-batch PRIVATE sssp Threads::Threads)

# tests executable
##################
set (TEST_HEADERS
	tests/test_graph.hpp
)
set (TESTS_SOURCES
	tests/criteria_test.cpp
	tests/dijkstra_basic_test.cpp
	tests/graph_test.cpp
	tests/math_test.cpp
	tests/test.cpp
	tests/test_graph.cpp
)
add_executable (tests ${TESTS_SOURCES} ${TEST_HEADERS})
target_link_libraries (tests PRIVATE sssp)
add_test (sssp-tests tests)
