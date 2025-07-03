#pragma once

#include <RenderData.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>
#include <Mesh.hpp>
#include <Textures.hpp>

#include <vector>

class SceneRenderer {
public:
  void destroy();

  void render(const RenderData& renderData, glm::uvec2 windowSize);

  size_t addMaterial(std::unique_ptr<ShaderProgram>&& material);

  template <std::derived_from<ShaderProgram> MaterialClass = ShaderProgram>
  [[nodiscard]] MaterialClass& materialAt(size_t materialIndex) {
    assert(materials.size() > materialIndex);
    return materials[materialIndex]->as<MaterialClass>();
  }

  std::vector<Mesh> meshes;
  std::vector<std::unique_ptr<ShaderProgram>> materials;
  std::vector<Textures> textureBundles;

  bool mBackfaceCullingEnabled = false;
};
