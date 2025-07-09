#pragma once

#include <Graphics/Transform.hpp>

#include <optional>
#include <vector>

struct MeshData {
  size_t meshIndex = SIZE_MAX;
  size_t diffuseMapIndex = SIZE_MAX;
  size_t specularMapIndex = SIZE_MAX;
  size_t emissionMapIndex = SIZE_MAX;
};

struct Model {
  Transform transform;
  std::vector<MeshData> meshes;
  size_t shaderProgramIndex = SIZE_MAX;
  std::optional<size_t> outlineShaderProgramIndex = std::nullopt;
  bool bBackfaceCulling = true;
};
