#pragma once

#include <Graphics/RenderingEngine.hpp>
#include <Scene/Scene.hpp>
#include <Util/Expected.hpp>

struct AppState;

class DemoBase {
public:
  virtual ~DemoBase() = default;

  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer);
  virtual void processKeyboard(GLFWwindow* window) {}
  virtual void processMouse(glm::vec2 mousePosition) {}
  virtual void update(double dt) {}
  [[nodiscard]] virtual CommandBuffer render();

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
  std::shared_ptr<RenderingEngine> mRenderingEngine;

  std::vector<ShaderProgramInstanceHandle> mPostProcessingShaderProgramInstances;

  entt::entity mMainCamera = entt::null;
  Texture2DHandle mMainViewColorAttachment = Texture2DHandle::null();
  FramebufferHandle mMainViewFramebuffer = FramebufferHandle::null();

  Texture2DArrayHandle mDirectionalLightShadowMaps = Texture2DArrayHandle::null();
  Texture2DArrayHandle mPointLightShadowMaps = Texture2DArrayHandle::null();
  Texture2DArrayHandle mSpotlightShadowMaps = Texture2DArrayHandle::null();
  std::vector<FramebufferHandle> mDirectionalLightShadowFramebuffers;
  std::vector<FramebufferHandle> mPointLightShadowFramebuffers;
  std::vector<FramebufferHandle> mSpotlightShadowFramebuffers;

  std::vector<Texture2DHandle> mPostProcessingColorAttachments;
  std::vector<FramebufferHandle> mPostProcessingFramebuffers;

  SceneRenderMode mSceneRenderMode = SceneRenderMode::Full;
  bool mbDebugVisualiseVertexNormals = false;
};
