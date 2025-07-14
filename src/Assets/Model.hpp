#pragma once

#include <Assets/AssetManager.hpp>
#include <Assets/Bitmap.hpp>
#include <Assets/Mesh.hpp>

#include <vector>

struct Model {
  std::vector<AssetHandle<Mesh>> meshes;
  std::vector<std::optional<AssetHandle<Bitmap>>> diffuseMaps;
  std::vector<std::optional<AssetHandle<Bitmap>>> specularMaps;
  std::vector<std::optional<AssetHandle<Bitmap>>> emissionMaps;
};
