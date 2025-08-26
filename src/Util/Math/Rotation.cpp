#include <Util/Math/Rotation.hpp>

#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>

Rotation Rotation::fromDirection(glm::vec3 direction) {
  direction = glm::normalize(direction);
  return Rotation {
    .pitch = glm::degrees(glm::asin(-direction.y)),
    .yaw = glm::degrees(glm::atan2(direction.z, direction.x)),
    .roll = 0.0f,
  };
}

glm::vec3 Rotation::asVec3() const {
  return { pitch, yaw, roll };
}

glm::quat Rotation::asQuat() const {
  return glm::quat(glm::radians(this->asVec3()));
}

glm::mat4 Rotation::asMat4() const {
  return glm::toMat4(this->asQuat());
}

glm::mat3 Rotation::asMat3() const {
  return glm::toMat3(this->asQuat());

  // glm::mat4 rot = glm::mat4(1.0f);
  // rot = glm::rotate(rot, glm::radians(pitch), RIGHT_AXIS);
  // rot = glm::rotate(rot, glm::radians(yaw), UP_AXIS);
  // rot = glm::rotate(rot, glm::radians(roll), FORWARD_AXIS);
  // return rot;

  const float cosPitch = cos(glm::radians(this->pitch));
  const float sinPitch = sin(glm::radians(this->pitch));
  const float cosYaw = cos(glm::radians(this->yaw));
  const float sinYaw = sin(glm::radians(this->yaw));
  const float cosRoll = cos(glm::radians(this->roll));
  const float sinRoll = sin(glm::radians(this->roll));
  // return {
  //   cosRoll * cosYaw - sinRoll * sinPitch * sinYaw, -sinRoll * cosPitch, cosRoll * sinYaw + sinRoll * sinPitch * cosYaw,
  //   sinRoll * cosYaw + cosRoll * sinPitch * sinYaw, cosRoll * cosPitch,  sinRoll * sinYaw - cosRoll * sinPitch * cosYaw,
  //   -cosPitch * sinYaw,                             sinPitch,            cosPitch * cosYaw,
  // };
  return {
    cosRoll * cosYaw - sinRoll * sinPitch * sinYaw,  sinRoll * cosPitch, cosRoll * sinYaw + sinRoll * sinPitch * sinYaw,
    -sinRoll * cosYaw - cosRoll * sinPitch * sinYaw, cosRoll * cosPitch, -sinRoll * sinYaw + cosRoll * sinPitch * sinYaw,
    -cosPitch * sinYaw,                              -sinPitch,          cosPitch * cosYaw,
  };
}

const float* Rotation::asFloatPtr() const {
  return reinterpret_cast<const float*>(this);
}

float* Rotation::asFloatPtr() {
  return reinterpret_cast<float*>(this);
}

Rotation operator*(const float lhs, const Rotation& rhs) {
  return {
    .pitch = lhs * rhs.pitch,
    .yaw = lhs * rhs.yaw,
    .roll = lhs * rhs.roll,
  };
}

Rotation operator*(const Rotation& lhs, float rhs) {
  return {
    .pitch = lhs.pitch * rhs,
    .yaw = lhs.yaw * rhs,
    .roll = lhs.roll * rhs,
  };
}

Rotation operator*(const Rotation& lhs, const Rotation& rhs) {
  return {
    .pitch = lhs.pitch * rhs.pitch,
    .yaw = lhs.yaw * rhs.yaw,
    .roll = lhs.roll * rhs.roll,
  };
}
