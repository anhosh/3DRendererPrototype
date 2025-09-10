#pragma once

#include <Assets/AssetHandle.hpp>
#include <Assets/Bitmap.hpp>
#include <Assets/Model.hpp>
#include <Util/Expected.hpp>
#include <Util/Registry.hpp>

#include <assimp/material.h>

#include <filesystem>
#include <unordered_map>

class aiMesh;
class aiNode;
class aiScene;
struct MeshData;

class AssetManager {
public:
  AssetManager();

  [[nodiscard]] AssetHandle<MeshData> addMesh(MeshData&& mesh);
  [[nodiscard]] AssetHandle<Bitmap> addBitmap(Bitmap&& bitmap);

  [[nodiscard]] Expected<AssetHandle<Bitmap>> loadBitmap(const std::filesystem::path& filePath, bool bFlipVertically = true);
  [[nodiscard]] Expected<AssetHandle<Model>> loadModel(const std::filesystem::path& filePath);

private:
  [[nodiscard]] Expected<void> locateModels();
  [[nodiscard]] Expected<void> locateTextures();

  [[nodiscard]] Expected<void> processNode(Model& model, const aiNode* node, const aiScene* scene, bool bUsesHeightForNormal);
  [[nodiscard]] Expected<void> processMesh(Model& model, const aiMesh* mesh, const aiScene* scene, bool bUsesHeightForNormal);
  [[nodiscard]] Expected<AssetHandle<Bitmap>> processTexture(const aiMaterial* material, aiTextureType type);

private:
  Registry<Bitmap> mBitmaps;
  Registry<Model> mModels;
  Registry<MeshData> mMeshes;

  std::unordered_map<std::filesystem::path, AssetHandle<Bitmap>> mLoadedBitmaps;
  std::unordered_map<std::filesystem::path, AssetHandle<Model>> mLoadedModels;

  std::filesystem::path mAssetsDir = "assets";
  std::filesystem::path mModelsDir = mAssetsDir / "models";
  std::filesystem::path mTexturesDir = mAssetsDir / "textures";
};
