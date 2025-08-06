#pragma once

#include <Scene/Components/Light.hpp>
#include <Scene/Components/Transform.hpp>
#include <Util/IntoBytes.hpp>

#include <tracy/TracyOpenGL.hpp>

#include <vector>

struct LightColorUniforms {
  glm::vec3 ambient;  float _padding0 = 0.0f;
  glm::vec3 diffuse;  float _padding1 = 0.0f;
  glm::vec3 specular; float _padding2 = 0.0f;

  static LightColorUniforms from(const LightColors& colors) {
    ZoneScoped;

    return LightColorUniforms {
      .ambient = colors.ambient,
      .diffuse = colors.diffuse,
      .specular = colors.specular,
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

  void writeToBuffer(const GLuint buffer, const size_t offset = 0) const {
    ZoneScoped;
    TracyGpuZone("LightSourceBuffer::writeToBuffer");

    const std::array lengthBytes = asBytes(glm::uvec4(this->sources.size(), 0, 0, 0));
    glNamedBufferSubData(buffer, static_cast<GLintptr>(offset), lengthBytes.size(), lengthBytes.data());
    glNamedBufferSubData(buffer, static_cast<GLintptr>(offset + lengthBytes.size()), size() - lengthBytes.size(), this->sources.data());
  }
};

struct DirectionalLightUniforms {
  LightColorUniforms colors;
  glm::vec3 direction;
  float _padding0 = 0.0f;

  static DirectionalLightUniforms from(const CompDirectionalLight& light) {
    ZoneScoped;

    return DirectionalLightUniforms {
      .colors = LightColorUniforms::from(light.colors),
      .direction = light.direction,
    };
  }
};

struct PointLightUniforms {
  LightColorUniforms colors;
  glm::vec3 position; float _padding0 = 0.0f;
  float constant;
  float linear;
  float quadratic;
  float _padding1 = 0.0f;

  static PointLightUniforms from(const CompPointLight& light, const CompTransform& transform) {
    ZoneScoped;

    return PointLightUniforms {
      .colors = LightColorUniforms::from(light.colors),
      .position = transform.translation,
      .constant = light.constant,
      .linear = light.linear,
      .quadratic = light.quadratic,
    };
  }
};

struct SpotlightUniforms {
  LightColorUniforms colors;
  glm::vec3 position; float _padding0 = 0.0f;
  glm::vec3 direction;
  float cutOff;
  float outerCutOff;
  float _padding1 = 0.0f;
  float _padding2 = 0.0f;
  float _padding3 = 0.0f;

  static SpotlightUniforms from(const CompSpotlight& light, const CompTransform& transform) {
    ZoneScoped;

    return SpotlightUniforms {
      .colors = LightColorUniforms::from(light.colors),
      .position = transform.translation,
      .direction = light.direction,
      .cutOff = glm::cos(glm::radians(light.cutOff)),
      .outerCutOff = glm::cos(glm::radians(light.outerCutOff)),
    };
  }
};

using DirectionalLightSourceBuffer = LightSourceBuffer<DirectionalLightUniforms>;
using PointLightSourceBuffer = LightSourceBuffer<PointLightUniforms>;
using SpotlightSourceBuffer = LightSourceBuffer<SpotlightUniforms>;
