cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(emotify-stderr
	VERSION 0.0.1
	DESCRIPTION "Spice up stderr output with emojis!"
	LANGUAGES C)

file(GLOB_RECURSE _SOURCES src/*.c)

add_library(${CMAKE_PROJECT_NAME} SHARED ${_SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} pthread)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-nostartfiles")
