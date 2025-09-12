cmake_minimum_required(VERSION 3.25)

project(tinyobjloader)

add_library(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_sources(${PROJECT_NAME} PRIVATE
  include/tiny_obj_loader.h
  src/tiny_obj_loader.cpp
)
