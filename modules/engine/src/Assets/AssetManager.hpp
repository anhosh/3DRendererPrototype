#pragma once

#include <Assets/AssetHandle.hpp>
#include <Assets/Bitmap.hpp>
#include <Assets/Model.hpp>
#include <Util/Expected.hpp>
#include <Util/Registry.hpp>

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

  [[nodiscard]] Expected<AssetHandle<Bitmap>> loadBitmap(const std::filesystem::path& filePath, bool bSRGB, bool bFlipVertically = true);
  [[nodiscard]] Expected<AssetHandle<Model>> loadModel(const std::filesystem::path& filePath);

private:
  [[nodiscard]] Expected<void> locateModels();
  [[nodiscard]] Expected<void> locateTextures();

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
