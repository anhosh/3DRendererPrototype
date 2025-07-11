#include <Assets/AssetManager.hpp>

#include <Assets/Bitmap.hpp>
#include <Assets/Mesh.hpp>
#include <Assets/Model.hpp>
#include <Graphics/Vertex.hpp>
#include <Util/Macros.hpp>
#include <Util/Paths.hpp>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <print>

template <>
Bitmap& AssetManager::Handle<Bitmap>::get() {
  return assetManager->bitmaps[index];
}

template <>
Mesh& AssetManager::Handle<Mesh>::get() {
  return assetManager->meshes[index];
}

template <>
Model& AssetManager::Handle<Model>::get() {
  return assetManager->models[index];
}

AssetManager::AssetManager() {
  PANIC_IF_UNEXPECTED(locateModels());
  PANIC_IF_UNEXPECTED(locateTextures());
}

Expected<void> AssetManager::locateModels() {
  if (std::optional<fs::path> modelsDir = locateDirectory("models")) {
    mModelsDir = std::move(modelsDir.value());
    return {};
  }
  return std::unexpected("Could not locate model directory");
}

Expected<void> AssetManager::locateTextures() {
  if (std::optional<fs::path> texturesDir = locateDirectory("textures")) {
    mTexturesDir = texturesDir.value();
    return {};
  }
  return std::unexpected("Could not locate texture directory");
}

AssetManager::Handle<Mesh> AssetManager::addMesh(Mesh mesh) {
  this->meshes.emplace_back(std::move(mesh));
  const size_t index = this->meshes.size() - 1;
  return Handle<Mesh> { index, this };
}

Expected<AssetManager::Handle<Bitmap>> AssetManager::loadBitmap(const std::filesystem::path& filePath) {
  const std::filesystem::path fullPath = mTexturesDir / filePath;
  if (const auto found = mLoadedAssetIndices.find(fullPath); found != mLoadedAssetIndices.end()) {
    const auto& [_, index] = *found;
    return Handle<Bitmap> { index, this };
  }

  Bitmap bitmap;
  ASSIGN_EXPECTED_OR_RETURN(bitmap, Bitmap::fromFile(fullPath));
  bitmaps.push_back(std::move(bitmap));
  const Handle<Bitmap> handle = { bitmaps.size() - 1, this };
  mLoadedAssetIndices[fullPath] = handle.index;
  return handle;
}

Expected<AssetManager::Handle<Model>> AssetManager::loadModel(const std::filesystem::path& filePath) {
  const std::filesystem::path fullPath = mModelsDir / filePath;
  if (const auto found = mLoadedAssetIndices.find(fullPath); found != mLoadedAssetIndices.end()) {
    const auto& [_, index] = *found;
    return Handle<Model> { index, this };
  }

  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(fullPath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    return std::unexpected(std::format("Assimp: {}", importer.GetErrorString()));
  }

  Model model;
  RETURN_ERROR_IF_UNEXPECTED(this->processNode(model, scene->mRootNode, scene));
  this->models.push_back(std::move(model));
  const size_t index = this->models.size() - 1;
  mLoadedAssetIndices[fullPath] = index;
  return Handle<Model> { index, this };
}

Expected<void> AssetManager::processNode(Model& model, aiNode* node, const aiScene* scene) {
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
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  vertices.reserve(mesh->mNumVertices);
  indices.reserve(mesh->mNumFaces * 3);

  for (size_t v = 0; v < mesh->mNumVertices; ++v) {
    const aiVector3D& vertex = mesh->mVertices[v];
    const aiVector3D& normal = mesh->mNormals[v];
    const aiVector3D& uv = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v] : aiVector3D(0.0f);
    vertices.push_back(Vertex {
      .position = glm::vec3(vertex.x, vertex.y, vertex.z),
      .normal = glm::vec3(normal.x, normal.y, normal.z),
      .texCoord = glm::vec2(uv.x, uv.y),
    });
  }

  for (uint32_t f = 0; f < mesh->mNumFaces; ++f) {
    aiFace face = mesh->mFaces[f];
    for (uint32_t i = 0; i < face.mNumIndices; ++i) {
      indices.push_back(face.mIndices[i]);
    }
  }

  model.meshes.push_back(this->addMesh(Mesh(vertices, indices)));
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

Expected<AssetManager::Handle<Bitmap>> AssetManager::processTexture(const aiMaterial* material, aiTextureType type) {
  if (material->GetTextureCount(type) == 0) {
    return std::unexpected(std::format("Could not find a {} texture", aiTextureTypeToString(type)));
  }

  aiString pathStr;
  material->GetTexture(type, 0, &pathStr);
  Expected<Handle<Bitmap>> bitmap = this->loadBitmap(pathStr.C_Str());
  RETURN_ERROR_IF_UNEXPECTED(bitmap);
  return bitmap.value();
}
