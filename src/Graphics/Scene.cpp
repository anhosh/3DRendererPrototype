#include <Graphics/Scene.hpp>

#include <Graphics/Actor.hpp>

#include <map>
#include <ranges>

void Scene::destroy() {
  actors.clear();
}

ActorHandle Scene::addActor(Actor&& actor) {
  return actors.add(std::forward<Actor>(actor));
}

std::vector<Draw> Scene::draw(const Camera& camera) const {
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
      const RenderData& resources = actor.renderData[resIdx];

      if (!resources.renderOptions.bTransparent) {
        draws.push_back(Draw {
          .transform = actor.transform,
          .shaderProgramInstance = resources.shaderProgramInstance,
          .vertexArray = resources.vertexArray,
          .diffuseMap = resources.diffuseMap,
          .specularMap = resources.specularMap,
          .emissionMap = resources.emissionMap,
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
  for (const auto& mesh : std::ranges::reverse_view(transparentMeshes)) {
    const auto& [actor, resIdx] = mesh.second;
    const RenderData& resources = actor->renderData[resIdx];
    draws.push_back(Draw {
      .transform = actor->transform,
      .shaderProgramInstance = resources.shaderProgramInstance,
      .vertexArray = resources.vertexArray,
      .diffuseMap = resources.diffuseMap,
      .specularMap = resources.specularMap,
      .emissionMap = resources.emissionMap,
      .bBackfaceCulling = resources.renderOptions.bBackfaceCulling,
      .bWriteToStencil = resources.renderOptions.outlineShaderInstance.has_value(),
      .bTransparent = true,
    });
  }

  // Object outlines
  for (const auto& [actor, resIdx]: meshesWithOutlines) {
    const RenderData& resources = actor->renderData[resIdx];
    Transform outlineTransform = actor->transform;
    outlineTransform.scale *= 1.05f;
    draws.push_back(Draw {
      .transform = outlineTransform,
      .shaderProgramInstance = resources.renderOptions.outlineShaderInstance.value(),
      .vertexArray = resources.vertexArray,
      .bBackfaceCulling = true,
      .bStencilTest = true,
      .bDepthTest = false,
    });
  }

  // Skybox
  if (skybox.has_value()) {
    draws.push_back(Draw {
      .shaderProgramInstance = skybox->shader,
      .vertexArray = skybox->cubeMesh,
      .emissionCubeMap = skybox->texture,
      .bBackfaceCulling = false,
      .bDisableCameraTranslation = true,
    });
  }

  return draws;
}
