project(LearnOpenGL-ext)

# add GLAD to build tree
message(STATUS "[glm]")
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

# setup Dear ImGui library
message(STATUS "[stb_image]")
add_library(stb_image)
add_library(stb_image::stb_image ALIAS stb_image)
target_include_directories(stb_image SYSTEM PUBLIC stb)
target_sources(stb_image PRIVATE
  stb/stb_image.cpp
  stb/stb_image.h
)

# declare ext library target
add_library(${PROJECT_NAME} INTERFACE)
add_library(LearnOpenGL::ext ALIAS ${PROJECT_NAME})

# link to all dependencies
target_link_libraries(${PROJECT_NAME} INTERFACE
  glad::glad
  glm::glm
  imgui::imgui
  stb_image::stb_image
)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  # link to dynamic loader
  target_link_libraries(${PROJECT_NAME} INTERFACE dl)
endif()
