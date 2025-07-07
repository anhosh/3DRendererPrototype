#pragma once

#include <Camera.hpp>
#include <DrawContext.hpp>
#include <Light.hpp>
#include <Mesh.hpp>
#include <Model.hpp>
#include <Scene.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>
#include <Texture.hpp>

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

bool locateModels();

class Scene {
public:
  void destroy();

  [[nodiscard]] std::vector<Draw> draw() const;

  std::expected<void, std::string> loadModel(const std::filesystem::path& path);

  size_t addModel(Model&& model);
  size_t addMesh(Mesh&& mesh);
  size_t addTexture(Texture&& texture);
  size_t addShaderProgram(std::unique_ptr<ShaderProgram>&& material);

  template <std::derived_from<ShaderProgram> ShaderProgramClass = ShaderProgram>
  [[nodiscard]] ShaderProgramClass& shaderProgramAt(const size_t index) {
    assert(shaderPrograms.size() > index);
    return shaderPrograms[index]->as<ShaderProgramClass>();
  }

public:
  std::vector<Model> models;
  std::vector<Mesh> meshes;
  std::vector<std::unique_ptr<ShaderProgram>> shaderPrograms;
  std::vector<Texture> textures;

  Camera camera;

  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;

private:
  std::expected<void, std::string> processNode(Model& model, aiNode* node, const aiScene* scene);
  std::expected<void, std::string> processMesh(Model& model, aiMesh* mesh, const aiScene* scene);
  std::expected<std::vector<size_t>, std::string> loadTextures(const aiMaterial* material, aiTextureType type);

private:
  std::unordered_map<fs::path, size_t> mLoadedTextureIndices;
};
