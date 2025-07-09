#include <Graphics/Scene.hpp>

#include <Graphics/Bitmap.hpp>
#include <Util/Log.hpp>
#include <Util/Paths.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static fs::path sModelsDir = "models";

std::expected<void, std::string> locateModels() {
  if (std::optional<fs::path> modelsDir = locateDirectory("models")) {
    sModelsDir = modelsDir.value();
    return {};
  }
  return std::unexpected("Could not locate model directory");
}

namespace fs = std::filesystem;

void Scene::destroy() {
  for (Mesh& mesh : meshes) {
    mesh.destroy();
  }
  for (const std::unique_ptr<ShaderProgram>& material : shaderPrograms) {
    material->destroy();
  }
  for (Texture& textures : textures) {
    textures.destroy();
  }

  meshes.clear();
  shaderPrograms.clear();
  textures.clear();
}

std::vector<Draw> Scene::draw() const {
  std::vector<Draw> draws;

  for (const Model& model: models) {
    for (const MeshData& meshData : model.meshes) {
      draws.push_back(Draw {
        .transform = model.transform,
        .shaderProgramIndex = model.shaderProgramIndex,
        .meshIndex = meshData.meshIndex,
        .diffuseMapIndex = meshData.diffuseMapIndex,
        .specularMapIndex = meshData.specularMapIndex,
        .emissionMapIndex = meshData.emissionMapIndex,
        .bBackfaceCulling = model.bBackfaceCulling,
        .bWriteToStencil = model.outlineShaderProgramIndex.has_value(),
      });
    }
  }

  // Object outlines
  for (const Model& model: models) {
    if (model.outlineShaderProgramIndex.has_value()) {
      Transform outlineTransform = model.transform;
      outlineTransform.scale *= 1.05f;
      for (const MeshData& meshData : model.meshes) {
        draws.push_back(Draw {
          .transform = outlineTransform,
          .shaderProgramIndex = model.outlineShaderProgramIndex.value(),
          .meshIndex = meshData.meshIndex,
          .bBackfaceCulling = model.bBackfaceCulling,
          .bStencilTest = true,
          .bDepthTest = false,
        });
      }
    }
  }

  return draws;
}

size_t Scene::addModel(Model&& model) {
  models.push_back(std::move(model));
  return models.size() - 1;
}

size_t Scene::addMesh(Mesh&& mesh) {
  meshes.push_back(mesh);
  return meshes.size() - 1;
}

size_t Scene::addTexture(Texture&& texture) {
  textures.push_back(texture);
  return textures.size() - 1;
}

size_t Scene::addShaderProgram(std::unique_ptr<ShaderProgram>&& material) {
  shaderPrograms.push_back(std::move(material));
  return shaderPrograms.size() - 1;
}

std::expected<void, std::string> Scene::loadModel(const fs::path& path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(sModelsDir / path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    return std::unexpected(std::format("Assimp: {}", importer.GetErrorString()));
  }

  Model model;
  RETURN_ERROR_IF_UNEXPECTED(processNode(model, scene->mRootNode, scene));
  this->addModel(std::move(model));
  return {};
}

std::expected<void, std::string> Scene::processNode(Model& model, aiNode* node, const aiScene* scene) {
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    RETURN_ERROR_IF_UNEXPECTED(processMesh(model, mesh, scene));
  }

  for (size_t i = 0; i < node->mNumChildren; ++i) {
    RETURN_ERROR_IF_UNEXPECTED(processNode(model, node->mChildren[i], scene));
  }

  return {};
}

std::expected<void, std::string> Scene::processMesh(Model& model, aiMesh* mesh, const aiScene* scene) {
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

  const size_t newMeshIndex = this->addMesh(Mesh(vertices, indices));
  MeshData newMeshData = { .meshIndex = newMeshIndex };
  if (mesh->mMaterialIndex < scene->mNumMaterials) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    ASSIGN_EXPECTED_OR_IGNORE(newMeshData.diffuseMapIndex, loadTexture(material, aiTextureType_DIFFUSE));
    ASSIGN_EXPECTED_OR_IGNORE(newMeshData.specularMapIndex, loadTexture(material, aiTextureType_SPECULAR));
    ASSIGN_EXPECTED_OR_IGNORE(newMeshData.emissionMapIndex, loadTexture(material, aiTextureType_EMISSIVE));
  }
  model.meshes.push_back(newMeshData);

  return {};
}

std::expected<size_t, std::string> Scene::loadTexture(const aiMaterial* material, aiTextureType type) {
  if (material->GetTextureCount(type) == 0) {
    return std::unexpected(std::format("Could not find a {} texture", aiTextureTypeToString(type)));
  }

  aiString pathStr;
  material->GetTexture(type, 0, &pathStr);
  if (const auto found = mLoadedTextureIndices.find(pathStr.C_Str()); found != mLoadedTextureIndices.end()) {
    const auto& [path, index] = *found;
    return index;
  }

  std::expected<Bitmap, std::string> bitmap = Bitmap::fromFile(pathStr.C_Str());
  RETURN_ERROR_IF_UNEXPECTED(bitmap);
  const size_t newTextureIndex = this->addTexture(Texture(bitmap.value()));
  mLoadedTextureIndices[pathStr.C_Str()] = newTextureIndex;
  return newTextureIndex;
}
