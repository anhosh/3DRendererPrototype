#pragma once

#include <Demo/FlyCamDemoBase.hpp>

class SpaceDemo final : public FlyCamDemoBase {
public:
  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) override;

  virtual void update(double deltaTime) override;

private:
  entt::entity mEnttSun = entt::null;

  std::vector<std::pair<entt::entity, float>> mAsteroidAngles;
  float mMarsRotationSpeed = 10.0f;
};
