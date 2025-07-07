#include <Graphics/SceneRenderer.hpp>

#include <Graphics/DrawContext.hpp>
#include <Graphics/Mesh.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderPrograms/ShaderProgram.hpp>

void SceneRenderer::render(const Scene& scene, glm::uvec2 windowSize) {
  const std::vector<Draw> draws = scene.draw();
  if (draws.empty()) {
    return;
  }

  TransformMatrices transforms{};
  transforms.view = scene.camera.view();
  transforms.projection = scene.camera.projection(windowSize);

  const Draw* lastDraw = &draws.front();
  for (size_t i = 0; i < draws.size(); i++) {
    const Draw* currDraw = &draws[i];
    const Mesh& currMesh = scene.meshes[currDraw->meshIndex];

    if (i == 0 || lastDraw->meshIndex != currDraw->meshIndex) {
      scene.meshes[currDraw->meshIndex].bind();
    }
    if (i == 0 || currDraw->shaderProgramIndex != lastDraw->shaderProgramIndex) {
      scene.shaderPrograms[currDraw->shaderProgramIndex]->use();
    }

    GLuint slot = GL_TEXTURE0;
    const auto bindTextures = [&, this](std::span<const size_t> textureIndices) {
      for (const size_t index : textureIndices) {
        scene.textures[index].bind(slot);
        mBoundTextureSlots.insert(slot);
        ++slot;
      }
    };
    bindTextures(currMesh.diffuseMapIndices);
    bindTextures(currMesh.specularMapIndices);
    bindTextures(currMesh.emissionMapIndices);

    if (i == 0 || currDraw->backfaceCulling != lastDraw->backfaceCulling) {
      if (currDraw->backfaceCulling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }

    lastDraw = currDraw;

    transforms.model = currDraw->transform.matrix();
    transforms.normal = glm::transpose(glm::inverse(transforms.model));

    scene.shaderPrograms[currDraw->shaderProgramIndex]->bindUniforms(transforms);
    currMesh.draw();
  }

  scene.meshes[lastDraw->meshIndex].unbind();
  scene.shaderPrograms[lastDraw->shaderProgramIndex]->stopUsing();
  for (const GLuint slot : mBoundTextureSlots) {
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  mBoundTextureSlots.clear();
}
