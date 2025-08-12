#pragma once

#include <Demos/FlyCamDemoBase.hpp>

class SpaceDemo : public FlyCamDemoBase {
public:
  virtual Expected<void> init(const std::shared_ptr<AssetManager>& assets, const std::shared_ptr<RenderingEngine>& renderer) override;
};
