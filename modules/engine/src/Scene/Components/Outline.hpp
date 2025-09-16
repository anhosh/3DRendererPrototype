#pragma once

#include <GraphicsOpenGL/ShaderProgramInstance.hpp>

struct CompOutline {
  GraphicsOpenGL::ShaderProgramInstanceHandle outlineShader;

  static void on_destroy(entt::registry& registry, const entt::entity entity) {
    registry.get<CompOutline>(entity).outlineShader.erase();
  }
};
