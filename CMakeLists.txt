cmake_minimum_required(VERSION 3.25)

# set C++ options
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set other CMake options
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_ENABLED")
set(BUILD_SHARED_LIBS OFF)

# add modules
add_subdirectory(modules/engine)
add_subdirectory(modules/ext)
add_subdirectory(modules/util)

project(3DRendererPrototype
  LANGUAGES CXX
)

# declare executable targets
add_subdirectory(demos/normal_mapping)
add_subdirectory(demos/space)
add_subdirectory(tests)
