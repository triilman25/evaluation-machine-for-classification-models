cmake_minimum_required(VERSION 3.18)
project(model_eval VERSION 0.5.0)

add_library(libtransform STATIC libtransform.cpp)
add_library(element STATIC component.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libtransform PUBLIC ${TORCH_LIBRARIES} element)
target_link_libraries(element PUBLIC raylib)
