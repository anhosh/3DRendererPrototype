#pragma once

#include <Assets/AssetHandle.hpp>

#include <vector>

class Bitmap;
struct MeshData;

struct Model {
  std::vector<AssetHandle<MeshData>> meshes;
  std::vector<AssetHandle<Bitmap>> diffuseMaps;
  std::vector<AssetHandle<Bitmap>> specularMaps;
  std::vector<AssetHandle<Bitmap>> emissionMaps;
  std::vector<AssetHandle<Bitmap>> normalMaps;
};
