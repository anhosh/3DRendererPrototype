cmake_minimum_required(VERSION 3.25)

# Add external libraries
message(STATUS "[catch]")
add_subdirectory(catch)

message(STATUS "[EnTT]")
add_subdirectory(entt)

message(STATUS "[GLAD]")
add_subdirectory(glad)

message(STATUS "[glm]")
set(GLM_ENABLE_CXX_20 ON)
add_subdirectory(glm)
target_compile_definitions(glm PUBLIC
  GLM_FORCE_SWIZZLE
  GLM_FORCE_XYZW_ONLY
  GLM_FORCE_RADIANS
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_SILENT_WARNINGS
  GLM_ENABLE_EXPERIMENTAL
  GLM_EXT_INCLUDED
)

message(STATUS "[Dear ImGui]")
add_subdirectory(imgui)

message(STATUS "[stb]")
add_subdirectory(stb)

message(STATUS "[tinyobjloader]")
add_subdirectory(tinyobjloader)

message(STATUS "[vkfw]")
set(VKFW_FETCH_DEPENDENCIES ON)
set(VKFW_NO_STRUCT_CONSTRUCTORS ON)
add_subdirectory(vkfw)

if (CMAKE_BUILD_TYPE STREQUAL Release)
  set(TRACY_ENABLE OFF)
  set(TRACY_ON_DEMAND OFF)
else ()
  set(TRACY_ENABLE ON)
  set(TRACY_ON_DEMAND ON)
endif ()
add_subdirectory(tracy)

# Declare ext library target
project(3DRendererPrototype-Ext)

add_library(${PROJECT_NAME} INTERFACE)
add_library(3DRendererPrototype::Ext ALIAS ${PROJECT_NAME})

# Link to all dependencies
target_link_libraries(${PROJECT_NAME} INTERFACE
  EnTT::EnTT
  glad::glad
  glfw
  glm::glm
  imgui::imgui
  stb::stb
  tinyobjloader
  TracyClient
  vkfw
  vulkan
)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  # Link to dynamic loader
  target_link_libraries(${PROJECT_NAME} INTERFACE dl)
endif()

# Disable compiler warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} INTERFACE -Wno-everything)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  target_compile_options(${PROJECT_NAME} INTERFACE /w)
endif ()
