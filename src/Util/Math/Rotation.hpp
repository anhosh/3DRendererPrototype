#pragma once

inline constexpr glm::vec3 RIGHT_AXIS   = { 0.0f, 1.0f, 0.0f };
inline constexpr glm::vec3 UP_AXIS      = { 0.0f, 1.0f, 0.0f };
inline constexpr glm::vec3 FORWARD_AXIS = { 0.0f, 1.0f, 0.0f };

struct Rotation {
  float pitch = 0.0f;
  float yaw   = 0.0f;
  float roll  = 0.0f;

  [[nodiscard]] static Rotation fromDirection(glm::vec3 direction);

  [[nodiscard]] glm::vec3 asVec3() const;
  [[nodiscard]] glm::quat asQuat() const;
  [[nodiscard]] glm::mat4 asMat4() const;
  [[nodiscard]] glm::mat3 asMat3() const;
  [[nodiscard]] const float* asFloatPtr() const;
  [[nodiscard]] float* asFloatPtr();

  bool operator==(const Rotation& other) const = default;
};

Rotation operator*(float lhs, const Rotation& rhs);
Rotation operator*(const Rotation& lhs, float rhs);
Rotation operator*(const Rotation& lhs, const Rotation& rhs);
