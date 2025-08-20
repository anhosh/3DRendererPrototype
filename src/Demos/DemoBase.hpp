#pragma once

#include <Graphics/RenderingEngine.hpp>
#include <Graphics/Textures/MultiTexture.hpp>
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

  void runGUI(AppState& state);
  virtual void gui(AppState& state);

private:
  void guiStats(const AppState& state) const;
  void guiDebug();
  void guiActors();
  void guiPostProcessing(const AppState& state);

protected:
  Scene mScene;

  entt::entity mMainCamera = entt::null;

  std::shared_ptr<AssetManager> mAssetManager;
  std::shared_ptr<RenderingEngine> mRenderingEngine;

  std::vector<ShaderProgramInstanceHandle> mPostProcessingShaderProgramInstances;

  FramebufferHandle mMainSceneFramebuffer = FramebufferHandle::null();
  std::vector<FramebufferHandle> mDirectionalShadowFramebuffers;
  std::vector<FramebufferHandle> mPostProcessingFramebuffers;

  Texture2DArrayHandle mDirectionalLightShadowMaps = Texture2DArrayHandle::null();

  SceneRenderMode mSceneRenderMode = SceneRenderMode::Full;
  bool mbDebugVisualiseVertexNormals = false;
};
