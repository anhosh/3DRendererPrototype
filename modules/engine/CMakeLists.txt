cmake_minimum_required(VERSION 3.24)

project(3DRendererPrototype-Engine)

add_library(${PROJECT_NAME})
add_library(3DRendererPrototype::Engine ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
    3DRendererPrototype::Ext
    3DRendererPrototype::Util
    glm::glm
    Catch2::Catch2WithMain
    TracyClient
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <glad/glad.h>
    <GLFW/glfw3.h>
    <glm/glm.hpp>
    <glm/gtx/vec_swizzle.hpp>
    <tracy/Tracy.hpp>
    <cstdint>
)

target_include_directories(${PROJECT_NAME} PUBLIC src)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${sources})
