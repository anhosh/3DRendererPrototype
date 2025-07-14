#pragma once

#include <Util/Expected.hpp>
#include <Util/Macros/Classes.hpp>

#include <assimp/material.h>

#include <filesystem>
#include <unordered_map>
#include <vector>

class aiMesh;
class aiNode;
class aiScene;
class Bitmap;
struct Mesh;
struct Model;

class AssetManager {
public:
  DECLARE_ITEM_HANDLE(AssetManager)

  AssetManager();

public:
  Handle<Mesh> addMesh(Mesh mesh);

  [[nodiscard]] Expected<Handle<Bitmap>> loadBitmap(const std::filesystem::path& filePath);
  [[nodiscard]] Expected<Handle<Model>> loadModel(const std::filesystem::path& filePath);

private:
  [[nodiscard]] Expected<void> locateModels();
  [[nodiscard]] Expected<void> locateTextures();

  [[nodiscard]] Expected<void> processNode(Model& model, aiNode* node, const aiScene* scene);
  [[nodiscard]] Expected<void> processMesh(Model& model, aiMesh* mesh, const aiScene* scene);
  [[nodiscard]] Expected<Handle<Bitmap>> processTexture(const aiMaterial* material, aiTextureType type);

public:
  std::vector<Bitmap> bitmaps;
  std::vector<Model> models;
  std::vector<Mesh> meshes;

private:
  std::unordered_map<std::filesystem::path, size_t> mLoadedAssetIndices;
  std::filesystem::path mModelsDir = "models";
  std::filesystem::path mTexturesDir = "textures";
};

template <typename Asset>
using AssetHandle = AssetManager::Handle<Asset>;
