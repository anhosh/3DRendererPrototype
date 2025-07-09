#include <map>
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
    sModelsDir = std::move(modelsDir.value());
    return {};
  }
  return std::unexpected("Could not locate model directory");
}

namespace fs = std::filesystem;

void Scene::destroy() {
  for (VertexArray& mesh : meshes) {
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

  // Opaque objects
  for (const MeshGroup& meshGroup: meshGroups) {
    for (const MeshData& meshData : meshGroup.meshes) {
      if (!meshData.bTransparent) {
        draws.push_back(Draw {
          .transform = meshGroup.transform,
          .shaderProgramIndex = meshGroup.shaderProgramIndex,
          .vertexArrayIndex = meshData.vertexArrayIndex,
          .diffuseMapIndex = meshData.diffuseMapIndex,
          .specularMapIndex = meshData.specularMapIndex,
          .emissionMapIndex = meshData.emissionMapIndex,
          .bBackfaceCulling = meshData.bBackfaceCulling,
          .bWriteToStencil = meshGroup.outlineShaderProgramIndex.has_value(),
        });
      }
    }
  }

  // Transparent objects
  struct SortedMesh {
    const MeshData* meshData;
    const Transform* transform;
    size_t shaderProgramIndex;
    std::optional<size_t> outlineShaderProgramIndex;
  };
  std::map<float, SortedMesh> sorted;
  for (const MeshGroup& meshGroup: meshGroups) {
    for (const MeshData& meshData : meshGroup.meshes) {
      if (meshData.bTransparent) {
        const float distance = glm::length(camera.position - meshGroup.transform.translation);
        sorted[distance] = SortedMesh {
          .meshData = &meshData,
          .transform = &meshGroup.transform,
          .shaderProgramIndex = meshGroup.shaderProgramIndex,
          .outlineShaderProgramIndex = meshGroup.outlineShaderProgramIndex,
        };
      }
    }
  }
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    const SortedMesh& sortedMesh = it->second;
    draws.push_back(Draw {
      .transform = *sortedMesh.transform,
      .shaderProgramIndex = sortedMesh.shaderProgramIndex,
      .vertexArrayIndex = sortedMesh.meshData->vertexArrayIndex,
      .diffuseMapIndex = sortedMesh.meshData->diffuseMapIndex,
      .specularMapIndex = sortedMesh.meshData->specularMapIndex,
      .emissionMapIndex = sortedMesh.meshData->emissionMapIndex,
      .bBackfaceCulling = sortedMesh.meshData->bBackfaceCulling,
      .bWriteToStencil = sortedMesh.outlineShaderProgramIndex.has_value(),
      .bTransparent = true,
    });
  }

  // Object outlines
  for (const MeshGroup& meshGroup: meshGroups) {
    if (meshGroup.outlineShaderProgramIndex.has_value()) {
      Transform outlineTransform = meshGroup.transform;
      outlineTransform.scale *= 1.05f;
      for (const MeshData& meshData : meshGroup.meshes) {
        draws.push_back(Draw {
          .transform = outlineTransform,
          .shaderProgramIndex = meshGroup.outlineShaderProgramIndex.value(),
          .vertexArrayIndex = meshData.vertexArrayIndex,
          .bBackfaceCulling = meshData.bBackfaceCulling,
          .bStencilTest = true,
          .bDepthTest = false,
        });
      }
    }
  }

  return draws;
}

size_t Scene::addMeshGroup(MeshGroup model) {
  meshGroups.push_back(std::move(model));
  return meshGroups.size() - 1;
}

size_t Scene::addMesh(VertexArray mesh) {
  meshes.push_back(mesh);
  return meshes.size() - 1;
}

size_t Scene::addTexture(Texture texture) {
  textures.push_back(texture);
  return textures.size() - 1;
}

size_t Scene::addShaderProgram(std::unique_ptr<ShaderProgram> material) {
  shaderPrograms.push_back(std::move(material));
  return shaderPrograms.size() - 1;
}

std::expected<void, std::string> Scene::loadModel(const fs::path& path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(sModelsDir / path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    return std::unexpected(std::format("Assimp: {}", importer.GetErrorString()));
  }

  MeshGroup meshGroup;
  RETURN_ERROR_IF_UNEXPECTED(processNode(meshGroup, scene->mRootNode, scene));
  this->addMeshGroup(std::move(meshGroup));
  return {};
}

std::expected<void, std::string> Scene::processNode(MeshGroup& meshGroup, aiNode* node, const aiScene* scene) {
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    RETURN_ERROR_IF_UNEXPECTED(processMesh(meshGroup, mesh, scene));
  }

  for (size_t i = 0; i < node->mNumChildren; ++i) {
    RETURN_ERROR_IF_UNEXPECTED(processNode(meshGroup, node->mChildren[i], scene));
  }

  return {};
}

std::expected<void, std::string> Scene::processMesh(MeshGroup& meshGroup, aiMesh* mesh, const aiScene* scene) {
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

  const size_t newMeshIndex = this->addMesh(VertexArray(vertices, indices));
  MeshData newMeshData = { .vertexArrayIndex = newMeshIndex };
  if (mesh->mMaterialIndex < scene->mNumMaterials) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    ASSIGN_EXPECTED_OR_IGNORE(newMeshData.diffuseMapIndex, loadTexture(material, aiTextureType_DIFFUSE));
    ASSIGN_EXPECTED_OR_IGNORE(newMeshData.specularMapIndex, loadTexture(material, aiTextureType_SPECULAR));
    ASSIGN_EXPECTED_OR_IGNORE(newMeshData.emissionMapIndex, loadTexture(material, aiTextureType_EMISSIVE));
  }
  meshGroup.meshes.push_back(newMeshData);

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
