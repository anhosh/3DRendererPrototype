#pragma once

#include <Graphics/Light.hpp>

#include <vector>

struct DirectionalLightUniforms {
  uint32_t numSources;
  std::vector<DirectionalLight> sources;
};

struct PointLightUniforms {
  uint32_t numSources;
  std::vector<PointLight> sources;
};

struct SpotlightUniforms {
  uint32_t numSources;
  std::vector<Spotlight> sources;
};
