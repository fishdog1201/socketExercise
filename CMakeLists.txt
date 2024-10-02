cmake_minimum_required(VERSION 3.20)
project(socketExercise)

set(CXX_STANDARD 17)
set(CMAKE_CXX_FALGS ${CMAKE_CXX_FALGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(basic)
add_subdirectory(multiThread)