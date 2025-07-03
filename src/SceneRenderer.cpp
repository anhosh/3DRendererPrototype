#include <SceneRenderer.hpp>

void SceneRenderer::destroy() {
  for (Mesh& mesh : meshes) {
    mesh.destroy();
  }
  for (const std::unique_ptr<ShaderProgram>& material : materials) {
    material->destroy();
  }
  for (Textures& textures : textureBundles) {
    textures.destroy();
  }

  meshes.clear();
  materials.clear();
  textureBundles.clear();
}

void SceneRenderer::render(const RenderData& renderData, glm::uvec2 windowSize) {
  if (renderData.draws.empty()) {
    return;
  }

  TransformMatrices transforms{};
  transforms.view = renderData.camera.view();
  transforms.projection = renderData.camera.projection(windowSize);

  const Draw* lastDraw = &renderData.draws.front();
  for (size_t i = 0; i < renderData.draws.size(); i++) {
    const Draw* currDraw = &renderData.draws[i];

    if (i == 0 || lastDraw->meshIndex != currDraw->meshIndex) {
      meshes[currDraw->meshIndex].bind();
    }
    if (i == 0 || lastDraw->materialIndex != currDraw->materialIndex) {
      materials[currDraw->materialIndex]->use();
    }
    if (i == 0 || lastDraw->texturesIndex != currDraw->texturesIndex) {
      if (currDraw->texturesIndex != SIZE_MAX) {
        textureBundles[currDraw->texturesIndex].bindAll();
      } else {
        for (GLint slot = 0; slot < 32; slot++) {
          glActiveTexture(GL_TEXTURE0 + slot);
          glBindTexture(GL_TEXTURE_2D, 0);
        }
      }
    }

    if (currDraw->backfaceCulling != mBackfaceCullingEnabled) {
      mBackfaceCullingEnabled = currDraw->backfaceCulling;
      if (mBackfaceCullingEnabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }

    lastDraw = currDraw;

    transforms.model = currDraw->transform.matrix();
    transforms.normal = glm::transpose(glm::inverse(transforms.view * transforms.model));

    materials[currDraw->materialIndex]->bindUniforms(transforms);
    meshes[currDraw->meshIndex].draw();
  }

  meshes[lastDraw->meshIndex].unbind();
  materials[lastDraw->materialIndex]->stopUsing();
  if (lastDraw->texturesIndex != SIZE_MAX) {
    textureBundles[lastDraw->texturesIndex].unbindAllSlots();
  }
}

size_t SceneRenderer::addMaterial(std::unique_ptr<ShaderProgram>&& material) {
  const size_t lastIndex = materials.size();
  materials.push_back(std::move(material));
  return lastIndex;
}
