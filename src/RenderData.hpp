#pragma once

#include <Camera.hpp>
#include <Light.hpp>
#include <ModelTransform.hpp>

#include <vector>

struct Draw {
  size_t meshIndex = 0;
  size_t materialIndex = 0;
  size_t texturesIndex = SIZE_MAX;
  ModelTransform transform = {};
  bool backfaceCulling = true;
};

struct RenderData {
  Camera camera;
  DirectionalLight directionalLight;
  PointLight pointLight;
  Spotlight spotlight;
  std::vector<Draw> draws;
};
