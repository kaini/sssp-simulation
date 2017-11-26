cmake_minimum_required (VERSION 3.5)
project (sssp-simulation)

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

find_package (Boost REQUIRED program_options)

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

set (HEADERS arguments.hpp dijkstra.hpp draw_graph.hpp generate_edges.hpp generate_positions.hpp graph.hpp math.hpp)
set (SOURCES arguments.cpp dijkstra.cpp draw_graph.cpp generate_edges.cpp generate_positions.cpp graph.cpp main.cpp math.cpp)
add_executable (sssp-simulation ${HEADERS} ${SOURCES})
target_compile_definitions (sssp-simulation PRIVATE -D_USE_MATH_DEFINES)
target_include_directories (sssp-simulation PRIVATE ${Boost_INCLUDE_DIRS} ${Cairo_INCLUDE_DIRS} ${Freetype_INCLUDE_DIRS} ${Sigcpp_INCLUDE_DIRS} ${Cairomm_INCLUDE_DIRS})
target_link_libraries (sssp-simulation PRIVATE ${Boost_LIBRARIES} ${Cairo_LIBRARIES} ${Cairomm_LIBRARIES})
