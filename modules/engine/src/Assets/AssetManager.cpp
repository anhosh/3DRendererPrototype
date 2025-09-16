#include <Assets/AssetManager.hpp>

#include <Assets/Bitmap.hpp>
#include <Assets/MeshData.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Vertex.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Math/Rotation.hpp>
#include <Util/Paths.hpp>
#include <Util/Timers/ScopedTimer.hpp>

#include <glm/gtx/quaternion.hpp>

#include <tiny_obj_loader.h>

#include <ranges>

AssetManager::AssetManager() {
  ZoneScoped;

  PANIC_IF_UNEXPECTED(locateModels());
  PANIC_IF_UNEXPECTED(locateTextures());
}

AssetHandle<MeshData> AssetManager::addMesh(MeshData&& mesh) {
  ZoneScoped;

  return mMeshes.add(std::forward<MeshData>(mesh));
}

AssetHandle<Bitmap> AssetManager::addBitmap(Bitmap&& bitmap) {
  return mBitmaps.add(std::move(bitmap));
}

Expected<AssetHandle<Bitmap>> AssetManager::loadBitmap(const std::filesystem::path& filePath, const bool bSRGB, const bool bFlipVertically) {
  ZoneScoped;

  const std::filesystem::path fullPath = mTexturesDir / filePath;
  if (const auto found = mLoadedBitmaps.find(fullPath); found != mLoadedBitmaps.end()) {
    const auto& [_, handle] = *found;
    return handle;
  }

  Bitmap bitmap;
  ASSIGN_EXPECTED_OR_RETURN(bitmap, Bitmap::fromFile(fullPath, bFlipVertically));
  bitmap.bSRGB = bSRGB;
  const AssetHandle<Bitmap> handle = this->addBitmap(std::move(bitmap));
  mLoadedBitmaps.emplace(fullPath, handle);
  return handle;
}

Expected<AssetHandle<Model>> AssetManager::loadModel(const std::filesystem::path& filePath) {
  ZoneScoped;

  const std::filesystem::path fullPath = mModelsDir / filePath;
  if (const auto found = mLoadedModels.find(fullPath); found != mLoadedModels.end()) {
    const auto& [_, handle] = *found;
    return handle;
  }

  ScopedTimer timer(std::format("Load model {}", filePath.string()));

  tinyobj::ObjReader reader;
  if (!reader.ParseFromFile((mModelsDir / filePath).string())) {
    return std::unexpected(std::format("TinyObjLoader: {}", reader.Error()));
  }
  if (!reader.Warning().empty()) {
    LOG_INFO("TinyObjLoader: {}", reader.Warning());
  }

  Model model;
  const tinyobj::attrib_t& attrib = reader.GetAttrib();
  const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
  const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

  for (const tinyobj::shape_t& shape : shapes) {
    MeshData meshData;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const tinyobj::index_t& index : shape.mesh.indices) {
      Vertex vertex = {
        .position = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2],
        },
        .normal = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2],
        },
        .texCoord = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          attrib.texcoords[2 * index.texcoord_index + 1],
        },
      };
      vertex.tangent = Rotation(90.0f, 0.0f, 0.0f).asMat3() * vertex.normal;

      if (!uniqueVertices.contains(vertex)) {
        uniqueVertices.emplace(vertex, uniqueVertices.size());
        meshData.vertices.push_back(vertex);
      }

      meshData.indices.push_back(uniqueVertices[vertex]);
    }

    const AssetHandle<MeshData> meshHandle = this->addMesh(std::move(meshData));
    model.meshes.push_back(meshHandle);

    model.diffuseMaps.push_back(AssetHandle<Bitmap>::null());
    model.specularMaps.push_back(AssetHandle<Bitmap>::null());
    model.emissionMaps.push_back(AssetHandle<Bitmap>::null());
    model.normalMaps.push_back(AssetHandle<Bitmap>::null());
    if (!materials.empty() && !shape.mesh.material_ids.empty()) {
      const size_t materialIndex = shape.mesh.material_ids.front();
      const tinyobj::material_t& material = materials[materialIndex < materials.size() ? materialIndex : 0];
      ASSIGN_EXPECTED_OR_IGNORE(model.diffuseMaps.back(), this->loadBitmap(material.diffuse_texname, true, false));
      ASSIGN_EXPECTED_OR_IGNORE(model.specularMaps.back(), this->loadBitmap(material.specular_texname, false, false));
      ASSIGN_EXPECTED_OR_IGNORE(model.emissionMaps.back(), this->loadBitmap(material.emissive_texname, true, false));
      ASSIGN_EXPECTED_OR_IGNORE(model.normalMaps.back(), this->loadBitmap(material.bump_texname, false, false));
    }
  }

  const AssetHandle<Model> handle = mModels.add(std::move(model));
  mLoadedModels.emplace(fullPath, handle);
  return handle;
}

Expected<void> AssetManager::locateModels() {
  ZoneScoped;

  if (std::optional<fs::path> modelsDir = locateDirectory(mAssetsDir / "models")) {
    mModelsDir = std::move(modelsDir.value());
    return {};
  }
  return std::unexpected("Could not locate model directory");
}

Expected<void> AssetManager::locateTextures() {
  ZoneScoped;

  if (const std::optional<fs::path> texturesDir = locateDirectory(mAssetsDir / "textures")) {
    mTexturesDir = texturesDir.value();
    return {};
  }
  return std::unexpected("Could not locate texture directory");
}
