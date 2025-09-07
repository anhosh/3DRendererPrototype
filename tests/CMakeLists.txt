cmake_minimum_required(VERSION 3.24)

project(3DRendererPrototype-Tests)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  3DRendererPrototype::Util
  Catch2::Catch2WithMain
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
  <glm/glm.hpp>
  <cstdint>
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

file(GLOB_RECURSE test-sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${test-sources})
