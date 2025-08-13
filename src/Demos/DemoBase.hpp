#pragma once

#include <Graphics/RenderingEngine.hpp>
#include <Scene/Scene.hpp>
#include <Util/Expected.hpp>

struct AppState;

class DemoBase {
public:
  virtual ~DemoBase() = default;

  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer);
  virtual void processKeyboard(GLFWwindow* window) = 0;
  virtual void processMouse(glm::vec2 mousePosition) = 0;
  virtual void update(double dt) = 0;
  [[nodiscard]] virtual std::vector<RenderPass> render();

  virtual void onWindowResize(GLFWwindow* window, glm::uvec2 newSize);

  void runGUI(AppState& state);
  virtual void gui(AppState& state);

private:
  void guiStats(const AppState& state) const;
  void guiDebug() const;
  void guiActors();
  void guiPostProcessing(const AppState& state);

protected:
  Scene mScene;

  entt::entity mMainCamera = entt::null;

  std::shared_ptr<AssetManager> mAssetManager;
  std::shared_ptr<RenderingEngine> mRenderingEngine;

  std::vector<ShaderProgramInstanceHandle> mPostProcessingShaderProgramInstances;

  std::optional<FramebufferHandle> mMainSceneFramebuffer;
  std::vector<FramebufferHandle> mPostProcessingFramebuffers;
};
