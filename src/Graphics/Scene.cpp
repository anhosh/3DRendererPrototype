#include <Graphics/Scene.hpp>

#include <Graphics/SceneRenderer.hpp>

#include <map>

void Scene::destroy() {
  actors.clear();
}

std::vector<Draw> Scene::draw() const {
  struct MeshDataReference {
    const Actor* actor = nullptr;
    size_t videoResourcesIndex = SIZE_MAX;
  };
  std::vector<Draw> draws;
  std::vector<MeshDataReference> meshesWithOutlines;
  std::map<float, MeshDataReference> transparentMeshes;

  // Opaque objects
  for (const Actor& actor: actors) {
    for (size_t resIdx = 0; resIdx < actor.renderData.size(); ++resIdx) {
      const SceneRenderer::RenderData& resources = actor.renderData[resIdx];

      if (!resources.renderOptions.bTransparent) {
        draws.push_back(Draw {
          .transform = actor.transform,
          .shaderProgramInstanceIndex = resources.shaderProgramInstance.index,
          .vertexArrayIndex = resources.vertexArray.index,
          .diffuseMapIndex = resources.diffuseMap.transform([](auto v) { return v.index; }).value_or(SIZE_MAX),
          .specularMapIndex = resources.specularMap.transform([](auto v) { return v.index; }).value_or(SIZE_MAX),
          .emissionMapIndex = resources.emissionMap.transform([](auto v) { return v.index; }).value_or(SIZE_MAX),
          .bBackfaceCulling = resources.renderOptions.bBackfaceCulling,
          .bWriteToStencil = resources.renderOptions.outlineShaderInstance.has_value(),
        });
      } else {
        const float distance = glm::length(camera.position - actor.transform.translation);
        transparentMeshes[distance] = MeshDataReference { &actor, resIdx };
      }

      if (resources.renderOptions.outlineShaderInstance.has_value()) {
        meshesWithOutlines.emplace_back(&actor, resIdx);
      }
    }
  }

  // Sorted transparent objects
  for (auto it = transparentMeshes.rbegin(); it != transparentMeshes.rend(); ++it) {
    const auto& [actor, resIdx] = it->second;
    const SceneRenderer::RenderData& resources = actor->renderData[resIdx];
    draws.push_back(Draw {
      .transform = actor->transform,
      .shaderProgramInstanceIndex = resources.shaderProgramInstance.index,
      .vertexArrayIndex = resources.vertexArray.index,
      .diffuseMapIndex = resources.diffuseMap.transform([](auto v) { return v.index; }).value_or(SIZE_MAX),
      .specularMapIndex = resources.specularMap.transform([](auto v) { return v.index; }).value_or(SIZE_MAX),
      .emissionMapIndex = resources.emissionMap.transform([](auto v) { return v.index; }).value_or(SIZE_MAX),
      .bBackfaceCulling = resources.renderOptions.bBackfaceCulling,
      .bWriteToStencil = resources.renderOptions.outlineShaderInstance.has_value(),
      .bTransparent = true,
    });
  }

  // Object outlines
  for (const auto& [actor, resIdx]: meshesWithOutlines) {
    const SceneRenderer::RenderData& resources = actor->renderData[resIdx];
    Transform outlineTransform = actor->transform;
    outlineTransform.scale *= 1.05f;
    draws.push_back(Draw {
      .transform = outlineTransform,
      .shaderProgramInstanceIndex = resources.renderOptions.outlineShaderInstance->index,
      .vertexArrayIndex = resources.vertexArray.index,
      .bBackfaceCulling = true,
      .bStencilTest = true,
      .bDepthTest = false,
    });
  }

  return draws;
}

Scene::ActorHandle Scene::addActor(Actor actor) {
  actors.push_back(std::move(actor));
  return { actors.size() - 1, this };
}
