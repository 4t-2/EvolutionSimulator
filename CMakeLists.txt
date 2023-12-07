cmake_minimum_required(VERSION 3.10)

project(EvolutionSimulator)

file(GLOB_RECURSE SRC ./src/*.cpp)
file(GLOB_RECURSE AGL ./lib/AGL/src/*.cpp)
file(GLOB_RECURSE PHY ./lib/PHY/src/*.cpp)
file(GLOB_RECURSE IN ./lib/IN/src/*.cpp)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "g++")

# set(CMAKE_BUILD_TYPE Debug) # default build

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set (CMAKE_CXX_FLAGS "-g -fsanitize=address")
	message(STATUS "Building with debug flags")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	set (CMAKE_CXX_FLAGS "-Ofast -flto=auto -g")
	message(STATUS "Building with release flags")
endif()

add_executable(EvolutionSimulator ${SRC} ${AGL} ${PHY} ${IN})

if(LINUX)
	target_link_libraries(EvolutionSimulator -lX11 -lGL -lGLEW -lSOIL -lfreetype)
	include_directories("/usr/include/freetype2")
endif()

if(WIN32)
	target_link_libraries(EvolutionSimulator "C:\\glew\\bin\\Release\\x64\\glew32.dll" "C:\\glfw\\lib\\glfw3.dll" "C:\\soil\\lib\\libsoil.a" "C:\\freetype\\objs\\libfreetype.dll" opengl32)
	include_directories("C:\\Users\\$ENV{USERNAME}\\Documents\\vcpkg\\installed\\x64-mingw-dynamic\\include")
endif()

include_directories("./lib")
