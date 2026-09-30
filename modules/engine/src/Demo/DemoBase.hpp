#pragma once

#include <GraphicsOpenGL/RenderingEngine.hpp>
#include <Scene/Scene.hpp>
#include <Util/Expected.hpp>

struct AppState;

class DemoBase {
public:
  virtual ~DemoBase() = default;

  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<GraphicsOpenGL::RenderingEngine>& renderer);
  virtual void processKeyboard(GLFWwindow* window [[maybe_unused]]) {}
  virtual void processMouse(glm::vec2 mousePosition [[maybe_unused]]) {}
  virtual void update(double dt);
  [[nodiscard]] virtual GraphicsOpenGL::CommandBuffer render();

  virtual void onWindowResize(GLFWwindow* window, glm::uvec2 newSize);
  virtual void onFrameEnd() {}

  void runGUI(AppState& state);
  virtual void gui(AppState& state);

private:
  void guiStats(const AppState& state) const;
  void guiDebug();
  void guiActors();
  void guiPostProcessing(const AppState& state);

protected:
  Scene mScene;

  glm::uvec2 mWindowSize = glm::uvec2(0.0f);

  std::shared_ptr<AssetManager> mAssetManager;
  std::shared_ptr<GraphicsOpenGL::RenderingEngine> mRenderingEngine;

  std::vector<GraphicsOpenGL::ShaderProgramInstanceHandle> mPostProcessingShaderProgramInstances;

  entt::entity mMainCamera = entt::null;
  GraphicsOpenGL::Texture2DHandle mMainViewColorAttachment = GraphicsOpenGL::Texture2DHandle::null();
  GraphicsOpenGL::FramebufferHandle mMainViewFramebuffer = GraphicsOpenGL::FramebufferHandle::null();

  GraphicsOpenGL::Texture2DArrayHandle mDirectionalLightShadowMaps = GraphicsOpenGL::Texture2DArrayHandle::null();
  GraphicsOpenGL::Texture2DArrayHandle mSpotlightShadowMaps = GraphicsOpenGL::Texture2DArrayHandle::null();
  // GraphicsOpenGL::TextureCubeMapArrayHandle mPointLightShadowMaps = TextureCubeMapArrayHandle::null();
  std::vector<GraphicsOpenGL::FramebufferHandle> mDirectionalLightShadowFramebuffers;
  std::vector<GraphicsOpenGL::FramebufferHandle> mSpotlightShadowFramebuffers;
  // std::vector<GraphicsOpenGL::FramebufferHandle> mPointLightShadowFramebuffers;

  std::vector<GraphicsOpenGL::Texture2DHandle> mPostProcessingColorAttachments;
  std::vector<GraphicsOpenGL::FramebufferHandle> mPostProcessingFramebuffers;

private:
  GraphicsOpenGL::SceneRenderMode mSceneRenderMode = GraphicsOpenGL::SceneRenderMode::Full;
  bool mbDebugVisualiseVertexNormals = false;
  bool mbDrawSceneBoundingBoxes = false;

  entt::entity mViewFrustum = entt::null;
  bool mbDrawViewFrustum = false;
  bool mbDrawDirectionalLightsViewFrustums = false;
  bool mbViewFrustumFollowsMainView = true;
};
