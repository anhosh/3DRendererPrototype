cmake_minimum_required(VERSION 3.25)

project(3DRendererPrototype-Engine)

add_library(${PROJECT_NAME})
add_library(3DRendererPrototype::Engine ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  3DRendererPrototype::Ext
  3DRendererPrototype::Util
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
  <cstdint>
  <glad/glad.h>
  <glm/glm.hpp>
  <glm/gtx/vec_swizzle.hpp>
  <tracy/Tracy.hpp>
  <vkfw/vkfw.hpp>
  <vulkan/vulkan.hpp>
)

target_include_directories(${PROJECT_NAME} PUBLIC src)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${sources})

# setup compiler warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
      $<$<NOT:$<CONFIG:Debug>>:-Werror> # warnings as errors if not Debug
  )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
      $<$<NOT:$<CONFIG:Debug>>:/WX> # warnings as errors if not Debug
      /Zc:preprocessor
  )
endif ()
