#pragma once

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Light.hpp>
#include <Scene/Components/Transform.hpp>
#include <Util/IntoBytes.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <span>
#include <vector>

struct LightViewUniforms {
  glm::mat4 view;
  glm::mat4 projection;
  float zMin;
  float zMax;
  float _padding0 = 0.0f;
  float _padding1 = 0.0f;
};

struct LightColorUniforms {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;

  static LightColorUniforms from(const LightColors& colors) {
    ZoneScoped;

    return LightColorUniforms {
      .ambient = glm::vec4(colors.ambient, 0.0f),
      .diffuse = glm::vec4(colors.diffuse, 0.0f),
      .specular = glm::vec4(colors.specular, 0.0f),
    };
  }
};

template <typename LightSourceUniforms>
struct LightSourceBuffer {
  std::vector<LightSourceUniforms> sources;

  [[nodiscard]] size_t size() const {
    //     [ count          ]   [ sources                                        ]
    return sizeof(glm::uvec4) + this->sources.size() * sizeof(LightSourceUniforms);
  }

  void writeToBuffer(const std::span<uint8_t> buffer, const size_t offset = 0) const {
    ZoneScoped;

    const std::array lengthBytes = asBytes(glm::uvec4(this->sources.size(), 0, 0, 0));
    std::ranges::copy(lengthBytes, buffer.subspan(offset).data());
    std::ranges::copy(this->sources, reinterpret_cast<LightSourceUniforms*>(buffer.subspan(offset + lengthBytes.size()).data()));
  }
};

struct DirectionalLightShaderData {
  LightColorUniforms colors;
  LightViewUniforms view;
  glm::vec3 direction;
  float _padding0 = 0.0f;

  static DirectionalLightShaderData from(const CompDirectionalLight& light, const CompCamera& camera, const CompTransform& transform) {
    ZoneScoped;

    return DirectionalLightShaderData {
      .colors = LightColorUniforms::from(light.colors),
      .view = LightViewUniforms {
        .view = transform.viewMatrix(),
        .projection = camera.orthographic(),
        .zMin = camera.clipBox.min.z,
        .zMax = camera.clipBox.max.z,
      },
      .direction = glm::normalize(light.direction),
    };
  }
};

struct PointLightShaderData {
  LightColorUniforms colors;
  glm::vec3 position;
  float constant;
  float linear;
  float quadratic;
  float _padding1 = 0.0f;

  static PointLightShaderData from(const CompPointLight& light, const CompTransform& transform) {
    ZoneScoped;

    return PointLightShaderData {
      .colors = LightColorUniforms::from(light.colors),
      .position = transform.translation,
      .constant = light.constant,
      .linear = light.linear,
      .quadratic = light.quadratic,
    };
  }
};

struct SpotlightShaderData {
  LightColorUniforms colors;
  LightViewUniforms view;
  glm::vec3 position;
  float _padding0 = 0.0f;
  glm::vec3 direction;
  float cutOff;
  float outerCutOff;
  float _padding1 = 0.0f, _padding2 = 0.0f, _padding3 = 0.0f;

  static SpotlightShaderData from(const CompSpotlight& light, const CompCamera& camera, const CompTransform& transform) {
    ZoneScoped;

    CompTransform transformCopy = transform;
    transformCopy.translation -= transformCopy.forward() * light.debugZOffset;
    return SpotlightShaderData {
      .colors = LightColorUniforms::from(light.colors),
      .view = LightViewUniforms {
        .view = transformCopy.viewMatrix(),
        .projection = camera.perspective(),
        .zMin = camera.clipBox.min.z,
        .zMax = camera.clipBox.max.z,
      },
      .position = transform.translation,
      .direction = light.direction,
      .cutOff = glm::cos(glm::radians(light.cutOff)),
      .outerCutOff = glm::cos(glm::radians(light.outerCutOff)),
    };
  }
};

using DirectionalLightSourceBuffer = LightSourceBuffer<DirectionalLightShaderData>;
using PointLightSourceBuffer = LightSourceBuffer<PointLightShaderData>;
using SpotlightSourceBuffer = LightSourceBuffer<SpotlightShaderData>;
