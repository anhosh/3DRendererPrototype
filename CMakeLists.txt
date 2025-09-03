cmake_minimum_required(VERSION 3.24)

project(3DRendererPrototype)

# set C++ options
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set other CMake options
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_ENABLED")
set(BUILD_SHARED_LIBS OFF)

# add modules
add_subdirectory(modules/engine)
add_subdirectory(modules/ext)
add_subdirectory(modules/tests)
add_subdirectory(modules/util)

# declare executable target
add_executable(${PROJECT_NAME})

# link to ext target
target_link_libraries(${PROJECT_NAME} PRIVATE
  3DRendererPrototype::engine
  3DRendererPrototype::ext
  3DRendererPrototype::util
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
file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${sources})

# setup compiler warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
    $<$<NOT:$<CONFIG:Debug>>:-Werror> # warnings as errors if not Debug
  )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:/WX> # warnings as errors if not Debug
  )
endif()
