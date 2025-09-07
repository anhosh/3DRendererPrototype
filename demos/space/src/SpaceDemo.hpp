#pragma once

#include <Demo/FlyCamDemoBase.hpp>

class SpaceDemo final : public FlyCamDemoBase {
public:
  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) override;
};
