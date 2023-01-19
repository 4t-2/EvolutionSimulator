cmake_minimum_required(VERSION 3.10)

project(EvolutionSimulator)

file(GLOB_RECURSE SRC ./src/*.cpp)
file(GLOB_RECURSE AGL ./lib/AGL/src/*.cpp)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug) # default build

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set (CMAKE_CXX_FLAGS "-g -fsanitize=address")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	set (CMAKE_CXX_FLAGS "-Ofast -march=native")
endif()

add_executable(EvolutionSimulator.out ${SRC} ${AGL})

target_link_libraries(EvolutionSimulator.out -lX11 -lGL -lGLEW -lSOIL -lfreetype)
include_directories("/usr/include/freetype2")
