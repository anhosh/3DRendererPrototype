#pragma once

#define TO_STATEMENT(code) \
  do { code } while (false)

#define RETURN_ERROR_IF_UNEXPECTED(expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); !result.has_value()) { \
      return std::unexpected(std::move(result.error())); \
    } \
  )

#define ASSIGN_EXPECTED_OR_RETURN(variable, expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); result.has_value()) { \
      variable = std::move(expr.value()); \
    } else { \
      return std::unexpected(std::move(result.error())); \
    } \
  )

#define ASSIGN_EXPECTED_OR_IGNORE(variable, expr) \
  TO_STATEMENT( \
    if (const auto& result = (expr); result.has_value()) { \
      variable = std::move(expr.value()); \
    } \
  )

#define VERTEX_ATTRIBUTE_FLOATS(index, name) \
  TO_STATEMENT( \
    glEnableVertexAttribArray(index); \
    glVertexAttribPointer(index, (sizeof(Vertex::name) / sizeof(float)), GL_FLOAT, GL_FALSE, sizeof(Vertex), \
    reinterpret_cast<void*>(offsetof(Vertex, name))); \
  )
