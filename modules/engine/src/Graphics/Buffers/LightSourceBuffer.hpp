#pragma once

#include <Scene/Components/Camera.hpp>
#include <Scene/Components/Light.hpp>
#include <Scene/Components/Transform.hpp>
#include <Util/IntoBytes.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <vector>

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
  glm::mat4 viewProjection;
  glm::vec4 direction;

  static DirectionalLightShaderData from(const CompDirectionalLight& light, const CompCamera& camera, const CompTransform& transform) {
    ZoneScoped;

    return DirectionalLightShaderData {
      .colors = LightColorUniforms::from(light.colors),
      .viewProjection = camera.orthographic() * transform.viewMatrix(),
      .direction = glm::vec4(glm::normalize(light.direction), 0.0f),
    };
  }
};

struct PointLightShaderData {
  LightColorUniforms colors;
  glm::vec4 position;
  float constant;
  float linear;
  float quadratic;
  float _padding0 = 0.0f;

  static PointLightShaderData from(const CompPointLight& light, const CompTransform& transform) {
    ZoneScoped;

    return PointLightShaderData {
      .colors = LightColorUniforms::from(light.colors),
      .position = glm::vec4(transform.translation, 0.0f),
      .constant = light.constant,
      .linear = light.linear,
      .quadratic = light.quadratic,
    };
  }
};

struct SpotlightShaderData {
  LightColorUniforms colors;
  glm::mat4 viewProjection;
  glm::vec4 position;
  glm::vec4 direction;
  float cutOff;
  float outerCutOff;
  float _padding0 = 0.0f;
  float _padding1 = 0.0f;

  static SpotlightShaderData from(const CompSpotlight& light, const CompCamera& camera, const CompTransform& transform) {
    ZoneScoped;

    return SpotlightShaderData {
      .colors = LightColorUniforms::from(light.colors),
      .viewProjection = camera.perspective() * transform.viewMatrix(),
      .position = glm::vec4(transform.translation, 0.0f),
      .direction = glm::vec4(light.direction, 0.0f),
      .cutOff = glm::cos(glm::radians(light.cutOff)),
      .outerCutOff = glm::cos(glm::radians(light.outerCutOff)),
    };
  }
};

using DirectionalLightSourceBuffer = LightSourceBuffer<DirectionalLightShaderData>;
using PointLightSourceBuffer = LightSourceBuffer<PointLightShaderData>;
using SpotlightSourceBuffer = LightSourceBuffer<SpotlightShaderData>;
