#include <Graphics/SceneRenderer.hpp>

#include <Graphics/DrawContext.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/Scene.hpp>
#include <Graphics/ShaderProgram.hpp>

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

    if (i == 0 || currDraw->bBackfaceCulling != lastDraw->bBackfaceCulling) {
      if (currDraw->bBackfaceCulling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
      } else {
        glDisable(GL_CULL_FACE);
      }
    }
    if (i == 0 || currDraw->bDepthTest != lastDraw->bDepthTest) {
      if (currDraw->bDepthTest) {
        glEnable(GL_DEPTH_TEST);
      } else {
        glDisable(GL_DEPTH_TEST);
      }
    }
    if (i == 0 || currDraw->bStencilTest != lastDraw->bStencilTest) {
      glStencilFunc(currDraw->bStencilTest ? GL_NOTEQUAL : GL_ALWAYS, 1, 0xff);
    }
    if (i == 0 || currDraw->bWriteToStencil != lastDraw->bWriteToStencil) {
      glStencilMask(currDraw->bWriteToStencil ? 0xff : 0x00);
    }
    if (i == 0 || currDraw->bTransparent != lastDraw->bTransparent) {
      if (currDraw->bTransparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      } else {
        glDisable(GL_BLEND);
      }
    }

    const VertexArray& currMesh = scene.meshes[currDraw->vertexArrayIndex];
    if (i == 0 || lastDraw->vertexArrayIndex != currDraw->vertexArrayIndex) {
      currMesh.bind();
    }
    if (i == 0 || currDraw->shaderProgramIndex != lastDraw->shaderProgramIndex) {
      scene.shaderPrograms[currDraw->shaderProgramIndex]->use();
    }

    GLuint slot = GL_TEXTURE0;
    const auto bindTextures = [&, this](const size_t textureIndex) {
      if (textureIndex == SIZE_MAX) {
        glActiveTexture(slot);
        glBindTexture(GL_TEXTURE_2D, 0);
        mBoundTextureSlots.erase(slot);
      } else {
        scene.textures[textureIndex].bind(slot);
        mBoundTextureSlots.insert(slot);
      }
      ++slot;
    };
    bindTextures(currDraw->diffuseMapIndex);
    bindTextures(currDraw->specularMapIndex);
    bindTextures(currDraw->emissionMapIndex);

    transforms.model = currDraw->transform.matrix();
    transforms.normal = glm::transpose(glm::inverse(transforms.model));

    scene.shaderPrograms[currDraw->shaderProgramIndex]->bindUniforms(transforms);
    currMesh.draw();

    lastDraw = currDraw;
  }

  scene.meshes[lastDraw->vertexArrayIndex].unbind();
  scene.shaderPrograms[lastDraw->shaderProgramIndex]->stopUsing();
  for (const GLuint slot : mBoundTextureSlots) {
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  mBoundTextureSlots.clear();
}
