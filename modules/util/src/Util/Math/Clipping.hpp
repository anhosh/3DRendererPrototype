#pragma once

#include <optional>
#include <vector>

template <int32_t> struct LineSegment;
struct Rectangle;
struct Triangle;

[[nodiscard]] std::optional<glm::vec3> segmentIntersectionYZ(const LineSegment<3>& segment, float x);
[[nodiscard]] std::optional<glm::vec3> segmentIntersectionXZ(const LineSegment<3>& segment, float y);
[[nodiscard]] std::optional<glm::vec3> segmentIntersectionXY(const LineSegment<3>& segment, float z);
[[nodiscard]] std::vector<Triangle> clipTriangleAbovePlane(const Triangle& triangle, float edgeY);
[[nodiscard]] std::vector<Triangle> clipTriangleBelowPlane(const Triangle& triangle, float edgeY);
[[nodiscard]] std::vector<Triangle> clipTriangleRightOfPlane(const Triangle& triangle, float edgeX);
[[nodiscard]] std::vector<Triangle> clipTriangleLeftOfPlane(const Triangle& triangle, float edgeX);
[[nodiscard]] std::vector<Triangle> clipTriangleToRectanglePlanes(const Triangle& triangle, const Rectangle& rectangle);
