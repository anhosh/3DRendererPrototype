#pragma once

#include <Assets/AssetHandle.hpp>

#include <vector>

class Bitmap;
struct MeshData;

struct Model {
  std::vector<AssetHandle<MeshData>> meshes;
  std::vector<std::optional<AssetHandle<Bitmap>>> diffuseMaps;
  std::vector<std::optional<AssetHandle<Bitmap>>> specularMaps;
  std::vector<std::optional<AssetHandle<Bitmap>>> emissionMaps;
};
