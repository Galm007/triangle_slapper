cmake_minimum_required(VERSION 3.10)

project(TriangleSlapper)

add_compile_options(-O3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES src/*.c)

add_executable(TriangleSlapper ${SRC_FILES})

target_include_directories(
	TriangleSlapper PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_link_libraries(TriangleSlapper PRIVATE m)
