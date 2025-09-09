#pragma once

#include <Demo/FlyCamDemoBase.hpp>

class SpaceDemo final : public FlyCamDemoBase {
public:
  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) override;
  virtual void update(double deltaTime) override;
  virtual void gui(AppState& state) override;

private:
  void onTransformUpdate(entt::registry&, entt::entity entity);

private:
  float mOrbitSpeedMultiplier = 1.0f;

  entt::entity mEnttSun = entt::null;
  entt::entity mEnttEarth = entt::null;
  entt::entity mEnttMoon = entt::null;

  std::vector<std::pair<entt::entity, float>> mCelestialBodyAngles;

  float mSunRotationSpeed = 10.0f;
  float mMoonAngle = 0.0f;
};
