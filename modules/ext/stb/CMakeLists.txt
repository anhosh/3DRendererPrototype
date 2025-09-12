cmake_minimum_required(VERSION 3.25)

project(stb)

add_library(stb)
add_library(stb::stb ALIAS stb)
target_include_directories(stb SYSTEM PUBLIC include)
target_sources(stb PRIVATE
  src/stb_image.cpp
  src/stb_include.cpp

  include/stb_image.h
  include/stb_include.h
)
