cmake_minimum_required(VERSION 3.24)

project(3DRendererPrototype-ext)

# add Assimp to build tree
message(STATUS "[assimp]")
add_subdirectory(assimp)

# add EnTT to build tree
message(STATUS "[EnTT]")
add_subdirectory(entt)

# add GLAD to build tree
message(STATUS "[GLAD]")
add_library(glad)
add_library(glad::glad ALIAS glad)
target_include_directories(glad SYSTEM PUBLIC glad/include)
target_sources(glad PRIVATE glad/src/glad.c)

# add GLFW to build tree
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
message(STATUS "[glfw]")
add_subdirectory(glfw)
add_library(glfw::glfw ALIAS glfw)

# add GLM to build tree
set(GLM_ENABLE_CXX_20 ON)
message(STATUS "[glm]")
add_subdirectory(glm)
target_compile_definitions(glm PUBLIC
  GLM_FORCE_XYZW_ONLY
  GLM_FORCE_RADIANS
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_SILENT_WARNINGS
  GLM_ENABLE_EXPERIMENTAL
  GLM_EXT_INCLUDED
)

# setup Dear ImGui library
message(STATUS "[Dear ImGui]")
add_library(imgui)
add_library(imgui::imgui ALIAS imgui)
target_include_directories(imgui SYSTEM PUBLIC imgui)
target_link_libraries(imgui PUBLIC
  glfw::glfw
)
target_sources(imgui PRIVATE
  imgui/imconfig.h
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui.cpp
  imgui/imgui.h

  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_glfw.h
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_opengl3_loader.h
)

# setup stb library
message(STATUS "[stb]")
add_library(stb)
add_library(stb::stb ALIAS stb)
target_include_directories(stb SYSTEM PUBLIC stb)
target_sources(stb PRIVATE
  stb/stb_image.cpp
  stb/stb_include.cpp

  stb/stb_image.h
  stb/stb_include.h
)

# add tracy to build tree
message(STATUS "[tracy]")
if (CMAKE_BUILD_TYPE STREQUAL Release)
  set(TRACY_ENABLE OFF)
  set(TRACY_ON_DEMAND OFF)
else ()
  set(TRACY_ENABLE ON)
  set(TRACY_ON_DEMAND ON)
endif ()
add_subdirectory(tracy)

# declare ext library target
add_library(${PROJECT_NAME} INTERFACE)
add_library(3DRendererPrototype::ext ALIAS ${PROJECT_NAME})

# link to all dependencies
target_link_libraries(${PROJECT_NAME} INTERFACE
  assimp::assimp
  EnTT::EnTT
  glad::glad
  glm::glm
  imgui::imgui
  stb::stb
  TracyClient
)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  # link to dynamic loader
  target_link_libraries(${PROJECT_NAME} INTERFACE dl)
endif()

# disable compiler warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} INTERFACE -Wno-everything)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  target_compile_options(${PROJECT_NAME} INTERFACE /w)
endif()
