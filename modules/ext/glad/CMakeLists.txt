cmake_minimum_required(VERSION 3.25)

project(glad)

add_library(${PROJECT_NAME})
add_library(glad::glad ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC include)
target_sources(${PROJECT_NAME} PRIVATE src/glad.c)
