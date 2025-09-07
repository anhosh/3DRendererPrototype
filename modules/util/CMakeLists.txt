cmake_minimum_required(VERSION 3.24)

project(3DRendererPrototype-Util)

add_library(${PROJECT_NAME})
add_library(3DRendererPrototype::Util ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  glm::glm
  Catch2::Catch2WithMain
  TracyClient
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
  <cstdint>
  <glm/glm.hpp>
  <tracy/Tracy.hpp>
)

target_include_directories(${PROJECT_NAME} PUBLIC src)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${sources})
