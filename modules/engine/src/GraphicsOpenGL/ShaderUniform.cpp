#include <GraphicsOpenGL/ShaderUniform.hpp>

#include <Util/Visitor.hpp>

namespace GraphicsOpenGL {
  void ShaderUniform::bind() const {
    std::visit(Visitor {
      [&](const bool v)       { glUniform1i(this->location, v ? GL_TRUE : GL_FALSE); },
      [&](const GLint v)      { glUniform1i(this->location, v); },
      [&](const GLuint v)     { glUniform1ui(this->location, v); },
      [&](const GLfloat v)    { glUniform1f(this->location, v); },
      [&](const GLdouble v)   { glUniform1d(this->location, v); },
      [&](const glm::vec2& v) { glUniform2f(this->location, v.x, v.y); },
      [&](const glm::vec3& v) { glUniform3f(this->location, v.x, v.y, v.z); },
      [&](const glm::vec4& v) { glUniform4f(this->location, v.x, v.y, v.z, v.w); },
      [&](const glm::mat2& v) { glUniformMatrix2fv(this->location, 1, GL_FALSE, glm::value_ptr(v)); },
      [&](const glm::mat3& v) { glUniformMatrix3fv(this->location, 1, GL_FALSE, glm::value_ptr(v)); },
      [&](const glm::mat4& v) { glUniformMatrix4fv(this->location, 1, GL_FALSE, glm::value_ptr(v)); },
    }, this->value);
  }
}
