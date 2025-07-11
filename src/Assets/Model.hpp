#pragma once

#include <Assets/AssetManager.hpp>
#include <Assets/Bitmap.hpp>
#include <Assets/Mesh.hpp>

#include <vector>

struct Model {
  std::vector<AssetManager::Handle<Mesh>> meshes;
  std::vector<std::optional<AssetManager::Handle<Bitmap>>> diffuseMaps;
  std::vector<std::optional<AssetManager::Handle<Bitmap>>> specularMaps;
  std::vector<std::optional<AssetManager::Handle<Bitmap>>> emissionMaps;
};
