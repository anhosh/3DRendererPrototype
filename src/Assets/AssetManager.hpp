#pragma once

#include <Util/Expected.hpp>
#include <Util/Registry.hpp>

#include <assimp/material.h>

#include <filesystem>
#include <unordered_map>

class aiMesh;
class aiNode;
class aiScene;
class Bitmap;
struct Mesh;
struct Model;

template <typename Asset>
using AssetHandle = typename Registry<Asset>::Handle;

class AssetManager {
public:
  AssetManager();

public:
  AssetHandle<Mesh> addMesh(Mesh&& mesh);

  [[nodiscard]] Expected<AssetHandle<Bitmap>> loadBitmap(const std::filesystem::path& filePath);
  [[nodiscard]] Expected<AssetHandle<Model>> loadModel(const std::filesystem::path& filePath);

private:
  [[nodiscard]] Expected<void> locateModels();
  [[nodiscard]] Expected<void> locateTextures();

  [[nodiscard]] Expected<void> processNode(Model& model, aiNode* node, const aiScene* scene);
  [[nodiscard]] Expected<void> processMesh(Model& model, aiMesh* mesh, const aiScene* scene);
  [[nodiscard]] Expected<AssetHandle<Bitmap>> processTexture(const aiMaterial* material, aiTextureType type);

private:
  Registry<Bitmap> mBitmaps;
  Registry<Model> mModels;
  Registry<Mesh> mMeshes;

  std::unordered_map<std::filesystem::path, AssetHandle<Bitmap>> mLoadedBitmaps;
  std::unordered_map<std::filesystem::path, AssetHandle<Model>> mLoadedModels;

  std::filesystem::path mModelsDir = "models";
  std::filesystem::path mTexturesDir = "textures";
};
