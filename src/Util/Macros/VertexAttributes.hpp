#pragma once

#include <Util/Macros/Common.hpp>

#define VERTEX_ATTRIBUTE_FLOATS(VertexType, index, name) \
  TO_STATEMENT( \
    glEnableVertexAttribArray(index); \
    glVertexAttribPointer(index, (sizeof(VertexType::name) / sizeof(float)), GL_FLOAT, GL_FALSE, sizeof(VertexType), \
    reinterpret_cast<void*>(offsetof(VertexType, name))); \
  )
