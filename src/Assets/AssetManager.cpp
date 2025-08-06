#include <Assets/AssetManager.hpp>

#include <Assets/Bitmap.hpp>
#include <Assets/MeshData.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Vertex.hpp>
#include <Util/Macros/Errors.hpp>
#include <Util/Paths.hpp>
#include <Util/Timers/ScopedTimer.hpp>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <print>

AssetManager::AssetManager() {
  ZoneScoped;

  PANIC_IF_UNEXPECTED(locateModels());
  PANIC_IF_UNEXPECTED(locateTextures());
}

AssetHandle<MeshData> AssetManager::addMesh(MeshData&& mesh) {
  ZoneScoped;

  return mMeshes.add(std::forward<MeshData>(mesh));
}

Expected<AssetHandle<Bitmap>> AssetManager::loadBitmap(const std::filesystem::path& filePath, bool bFlipVertically) {
  ZoneScoped;

  const std::filesystem::path fullPath = mTexturesDir / filePath;
  if (const auto found = mLoadedBitmaps.find(fullPath); found != mLoadedBitmaps.end()) {
    const auto& [_, handle] = *found;
    return handle;
  }

  Bitmap bitmap;
  ASSIGN_EXPECTED_OR_RETURN(bitmap, Bitmap::fromFile(fullPath, bFlipVertically));
  const AssetHandle<Bitmap> handle = mBitmaps.add(std::move(bitmap));
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

  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(fullPath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    return std::unexpected(std::format("Assimp: {}", importer.GetErrorString()));
  }

  ScopedTimer timer(std::format("Load model {}", filePath.string()));
  Model model;
  RETURN_ERROR_IF_UNEXPECTED(this->processNode(model, scene->mRootNode, scene));
  const AssetHandle<Model> handle = mModels.add(std::move(model));
  mLoadedModels.emplace(fullPath, handle);
  return handle;
}

Expected<void> AssetManager::locateModels() {
  ZoneScoped;

  if (std::optional<fs::path> modelsDir = locateDirectory("models")) {
    mModelsDir = std::move(modelsDir.value());
    return {};
  }
  return std::unexpected("Could not locate model directory");
}

Expected<void> AssetManager::locateTextures() {
  ZoneScoped;

  if (const std::optional<fs::path> texturesDir = locateDirectory("textures")) {
    mTexturesDir = texturesDir.value();
    return {};
  }
  return std::unexpected("Could not locate texture directory");
}

Expected<void> AssetManager::processNode(Model& model, aiNode* node, const aiScene* scene) {
  ZoneScoped;

  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    RETURN_ERROR_IF_UNEXPECTED(this->processMesh(model, mesh, scene));
  }

  for (size_t i = 0; i < node->mNumChildren; ++i) {
    RETURN_ERROR_IF_UNEXPECTED(this->processNode(model, node->mChildren[i], scene));
  }

  return {};
}

Expected<void> AssetManager::processMesh(Model& model, aiMesh* mesh, const aiScene* scene) {
  ZoneScoped;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  vertices.resize(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces * 3);

  for (size_t v = 0; v < mesh->mNumVertices; ++v) {
    const aiVector3D& vertex = mesh->mVertices[v];
    vertices[v].position = glm::vec3(vertex.x, vertex.y, vertex.z);
  }
  for (size_t v = 0; v < mesh->mNumVertices; ++v) {
    const aiVector3D& normal = mesh->mNormals[v];
    vertices[v].normal = glm::vec3(normal.x, normal.y, normal.z);
  }
  for (size_t v = 0; v < mesh->mNumVertices; ++v) {
    const aiVector3D& texCoord = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v] : aiVector3D(0.0f);
    vertices[v].texCoord = glm::vec2(texCoord.x, texCoord.y);
  }

  for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
    const aiFace face = mesh->mFaces[f];
    for (uint32_t i = 0; i < face.mNumIndices; ++i) {
      indices.push_back(face.mIndices[i]);
    }
  }

  model.meshes.push_back(this->addMesh(MeshData(std::move(vertices), std::move(indices))));

  const size_t meshIndex = model.meshes.size() - 1;
  if (mesh->mMaterialIndex < scene->mNumMaterials) {
    const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    model.diffuseMaps.emplace_back();
    model.specularMaps.emplace_back();
    model.emissionMaps.emplace_back();
    ASSIGN_EXPECTED_OR_IGNORE(model.diffuseMaps[meshIndex], processTexture(material, aiTextureType_DIFFUSE));
    ASSIGN_EXPECTED_OR_IGNORE(model.specularMaps[meshIndex], processTexture(material, aiTextureType_SPECULAR));
    ASSIGN_EXPECTED_OR_IGNORE(model.emissionMaps[meshIndex], processTexture(material, aiTextureType_EMISSIVE));
  }

  return {};
}

Expected<AssetHandle<Bitmap>> AssetManager::processTexture(const aiMaterial* material, aiTextureType type) {
  ZoneScoped;

  if (material->GetTextureCount(type) == 0) {
    return std::unexpected(std::format("Could not find a {} texture", aiTextureTypeToString(type)));
  }

  aiString pathStr;
  material->GetTexture(type, 0, &pathStr);
  return this->loadBitmap(pathStr.C_Str());
}
