cmake_minimum_required(VERSION 3.25)

project(3DRendererPrototype-FloatingBackpackDemo)

add_executable(${PROJECT_NAME})

# copy assets folder to build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# link to ext target
target_link_libraries(${PROJECT_NAME} PRIVATE
    3DRendererPrototype::Engine
    3DRendererPrototype::Ext
    3DRendererPrototype::Util
    tbb
)

# setup precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <glad/glad.h>
    <GLFW/glfw3.h>
    <glm/glm.hpp>
    <glm/gtx/vec_swizzle.hpp>
    <tracy/Tracy.hpp>
    <cstdint>
)

# enable including headers in 'src/'
target_include_directories(${PROJECT_NAME} PRIVATE src)

# add all source files in 'src/' to target
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
  )
endif ()
