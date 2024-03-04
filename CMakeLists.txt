cmake_minimum_required(VERSION 3.10)

project(EvolutionSimulator)

file(GLOB_RECURSE SRC ./src/*.cpp)
file(GLOB_RECURSE AGL ./lib/AGL/src/*.cpp)
file(GLOB_RECURSE PHY ./lib/PHY/src/*.cpp)
file(GLOB_RECURSE IN ./lib/IN/src/*.cpp)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_COMPILER "g++")

if(WIN32)
	find_package(GLEW REQUIRED)
	find_package(GLFW3 3.3 REQUIRED)
endif()


# set(CMAKE_BUILD_TYPE Debug) # default build

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set (CMAKE_CXX_FLAGS "-g -fsanitize=address")
	message(STATUS "Building with debug flags")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	set (CMAKE_CXX_FLAGS "-Ofast -flto=auto")
	message(STATUS "Building with release flags")
endif()

add_executable(EvolutionSimulator ${SRC} ${AGL} ${PHY} ${IN})

if(LINUX)
	target_link_libraries(EvolutionSimulator -lX11 -lGL -lGLEW -lSOIL -lfreetype)
	target_include_directories(EvolutionSimulator PUBLIC "/usr/include/freetype2")
	include_directories("./lib")
endif()

if(WIN32)
	#target_link_libraries(EvolutionSimulator "C:\\soil\\lib\\libsoil.a" "C:\\freetype\\objs\\libfreetype.dll" opengl32)
	target_link_libraries(EvolutionSimulator opengl32)
	#include_directories("C:\\Users\\$ENV{USERNAME}\\Documents\\vcpkg\\installed\\x64-mingw-dynamic\\include")
	target_include_directories(EvolutionSimulator 
	                    PUBLIC ${GLEW_INCLUDE_DIRS}
						PUBLIC "./lib")
	target_link_libraries(EvolutionSimulator glfw )
	#target_link_libraries(EvolutionSimulator glew )
	target_link_libraries(EvolutionSimulator ${GLEW_STATIC_LIBRARY} )
endif()

#target_include_directories(EvolutionSimulator INTERFACE )
#link_libraries(${GLEW_LIBRARIES})
#target_include_directories(EvolutionSimulator )
