#include <Graphics/Scene.hpp>

#include <Graphics/RenderingEngine.hpp>

#include <map>
#include <ranges>

DEFINE_HANDLE_ITEM_FUNCTIONS(Scene, Actor, actors)

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
  for (const Actor& actor: std::ranges::views::values(actors)) {
    for (size_t resIdx = 0; resIdx < actor.renderData.size(); ++resIdx) {
      const RenderingEngine::RenderData& resources = actor.renderData[resIdx];

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
    const RenderingEngine::RenderData& resources = actor->renderData[resIdx];
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
    const RenderingEngine::RenderData& resources = actor->renderData[resIdx];
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

Scene::Handle<Actor> Scene::addActor(Actor actor) {
  actors[mNextActorID] = std::move(actor);
  return { mNextActorID++, this };
}
