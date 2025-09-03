#pragma once

inline constexpr uint32_t BINDING_UBO_CAMERA = 0;

inline constexpr uint32_t BINDING_SSBO_INSTANCES = 0;
inline constexpr uint32_t BINDING_SSBO_DIRECTIONAL_LIGHTS = 1;
inline constexpr uint32_t BINDING_SSBO_POINT_LIGHTS = 2;
inline constexpr uint32_t BINDING_SSBO_SPOTLIGHTS = 3;

inline constexpr int32_t BINDING_SAMPLER_SCREEN = 0;
inline constexpr int32_t BINDING_SAMPLER_DIFFUSE = 0;
inline constexpr int32_t BINDING_SAMPLER_SPECULAR = 1;
inline constexpr int32_t BINDING_SAMPLER_EMISSION = 2;
inline constexpr int32_t BINDING_SAMPLER_ENVIRONMENT = 3;
inline constexpr int32_t BINDING_SAMPLER_DIRECTIONAL_SHADOWS = 4;
inline constexpr int32_t BINDING_SAMPLER_POINT_SHADOWS = 5;
inline constexpr int32_t BINDING_SAMPLER_SPOTLIGHT_SHADOWS = 6;
