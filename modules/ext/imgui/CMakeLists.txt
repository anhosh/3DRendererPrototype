cmake_minimum_required(VERSION 3.25)

project(imgui)

add_library(${PROJECT_NAME})
add_library(imgui::imgui ALIAS ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC .)
target_sources(${PROJECT_NAME} PRIVATE
  imconfig.h
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_internal.h
  imgui_tables.cpp
  imgui_widgets.cpp
  imgui.cpp
  imgui.h

  backends/imgui_impl_glfw.cpp
  backends/imgui_impl_glfw.h
  backends/imgui_impl_opengl3.cpp
  backends/imgui_impl_opengl3.h
  backends/imgui_impl_opengl3_loader.h
)
