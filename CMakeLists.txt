cmake_minimum_required (VERSION 3.5.1)
project (sssp-simulation)

set (CMAKE_CXX_STANDARD 17)
# TODO With cmake 3.10+ this if can be removed.
if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif ()

find_package (Boost REQUIRED program_options)
# TODO This is a workaround for a bug in Boost with MSVC C++17 compliance.
if (MSVC)
	add_definitions (-D_HAS_AUTO_PTR_ETC)
endif ()

find_path (Cairo_INCLUDE_DIRS cairo.h)
find_library (Cairo_LIBRARIES cairo)

find_path (Cairomm_INCLUDE_DIR_1 cairomm/cairomm.h)
find_path (Cairomm_INCLUDE_DIR_2 cairommconfig.h)
set (Cairomm_INCLUDE_DIRS "${Cairomm_INCLUDE_DIR_1}" "${Cairomm_INCLUDE_DIR_2}")
find_library (Cairomm_LIBRARIES cairomm-1.0 NAMES cairomm-1 cairomm)

set (HEADERS dijkstra.hpp draw_graph.hpp generate_edges.hpp generate_positions.hpp graph.hpp math.hpp)
set (SOURCES dijkstra.cpp draw_graph.cpp generate_edges.cpp generate_positions.cpp graph.cpp main.cpp math.cpp)
add_executable (sssp-simulation ${HEADERS} ${SOURCES})
target_compile_definitions (sssp-simulation PRIVATE -D_USE_MATH_DEFINES)
target_include_directories (sssp-simulation PRIVATE ${Boost_INCLUDE_DIRS} ${Cairo_INCLUDE_DIRS} ${Cairomm_INCLUDE_DIRS})
target_link_libraries (sssp-simulation PRIVATE ${Boost_LIBRARIES} ${Cairo_LIBRARIES} ${Cairomm_LIBRARIES})
