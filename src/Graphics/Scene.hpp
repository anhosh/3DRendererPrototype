#pragma once

#include <Graphics/Camera.hpp>
#include <Graphics/DrawContext.hpp>
#include <Graphics/Light.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/MeshGroup.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>
#include <Graphics/Texture.hpp>

#include <assimp/material.h>

#include <cassert>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

struct aiMaterial;
class aiMesh;
class aiNode;
class aiScene;

std::expected<void, std::string> locateModels();

class Scene {
public:
  void destroy();

  [[nodiscard]] std::vector<Draw> draw() const;

  size_t addMeshGroup(MeshGroup model);
  size_t addMesh(VertexArray mesh);
  size_t addTexture(Texture texture);
  size_t addShaderProgram(std::unique_ptr<ShaderProgram> material);

  template <std::derived_from<ShaderProgram> ShaderProgramClass = ShaderProgram>
  [[nodiscard]] ShaderProgramClass& shaderProgramAt(const size_t index) {
    assert(shaderPrograms.size() > index);
    return shaderPrograms[index]->as<ShaderProgramClass>();
  }

  std::expected<void, std::string> loadModel(const std::filesystem::path& path);

public:
  std::vector<MeshGroup> meshGroups;
  std::vector<VertexArray> meshes;
  std::vector<std::unique_ptr<ShaderProgram>> shaderPrograms;
  std::vector<Texture> textures;

  Camera camera;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;

private:
  std::expected<void, std::string> processNode(MeshGroup& meshGroup, aiNode* node, const aiScene* scene);
  std::expected<void, std::string> processMesh(MeshGroup& meshGroup, aiMesh* mesh, const aiScene* scene);
  std::expected<size_t, std::string> loadTexture(const aiMaterial* material, aiTextureType type);

private:
  std::unordered_map<fs::path, size_t> mLoadedTextureIndices;
};
