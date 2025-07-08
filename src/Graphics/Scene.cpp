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
    for (const size_t meshIndex : model.meshIndices) {
      draws.push_back(Draw {
        .meshIndex = meshIndex,
        .shaderProgramIndex = model.shaderProgramIndex,
        .transform = model.transform,
        .bWriteToStencil = model.outlineShaderProgramIndex.has_value(),
      });
    }
  }

  // Object outlines
  for (const Model& model: models) {
    if (model.outlineShaderProgramIndex.has_value()) {
      ModelTransform outlineTransform = model.transform;
      outlineTransform.scale *= 1.05f;
      for (const size_t meshIndex : model.meshIndices) {
        draws.push_back(Draw {
          .meshIndex = meshIndex,
          .shaderProgramIndex = model.outlineShaderProgramIndex.value(),
          .transform = outlineTransform,
          .bStencilTest = true,
          .bDepthTest = false,
        });
      }
    }
  }

  return draws;
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

  auto newMesh = Mesh(vertices, indices);
  if (mesh->mMaterialIndex < scene->mNumMaterials) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::expected<std::vector<size_t>, std::string> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE);
    std::expected<std::vector<size_t>, std::string> specularMaps = loadTextures(material, aiTextureType_SPECULAR);
    std::expected<std::vector<size_t>, std::string> emissionMaps = loadTextures(material, aiTextureType_EMISSIVE);
    RETURN_ERROR_IF_UNEXPECTED(diffuseMaps);
    RETURN_ERROR_IF_UNEXPECTED(specularMaps);
    RETURN_ERROR_IF_UNEXPECTED(emissionMaps);
    newMesh.diffuseMapIndices = std::move(diffuseMaps.value());
    newMesh.specularMapIndices = std::move(specularMaps.value());
    newMesh.emissionMapIndices = std::move(emissionMaps.value());
  }
  meshes.push_back(newMesh);
  model.meshIndices.push_back(meshes.size() - 1);

  return {};
}

std::expected<std::vector<size_t>, std::string> Scene::loadTextures(const aiMaterial* material, aiTextureType type) {
  std::vector<size_t> textureIndices;
  for (size_t i = 0; i < material->GetTextureCount(type); i++) {
    aiString pathStr;
    material->GetTexture(type, static_cast<uint32_t>(i), &pathStr);
    if (const auto found = mLoadedTextureIndices.find(pathStr.C_Str()); found != mLoadedTextureIndices.end()) {
      const auto& [path, index] = *found;
      textureIndices.emplace_back(index);
    } else {
      std::expected<Bitmap, std::string> bitmap = Bitmap::fromFile(pathStr.C_Str());
      RETURN_ERROR_IF_UNEXPECTED(bitmap);
      textures.emplace_back(bitmap.value());

      const size_t index = textures.size() - 1;
      mLoadedTextureIndices[pathStr.C_Str()] = index;
      textureIndices.push_back(index);
    }
  }
  return textureIndices;
}
