#pragma once

#include <string>

template <int32_t> struct LineSegment;
struct Rectangle;
struct Triangle;

[[nodiscard]] std::optional<glm::vec3> segmentIntersectionYZ(const LineSegment<3>& segment, float x);
[[nodiscard]] std::optional<glm::vec3> segmentIntersectionXZ(const LineSegment<3>& segment, float y);
[[nodiscard]] std::optional<glm::vec3> segmentIntersectionXY(const LineSegment<3>& segment, float z);
[[nodiscard]] std::basic_string<Triangle> clipTriangleAbovePlane(const Triangle& triangle, float edgeY);
[[nodiscard]] std::basic_string<Triangle> clipTriangleBelowPlane(const Triangle& triangle, float edgeY);
[[nodiscard]] std::basic_string<Triangle> clipTriangleRightOfPlane(const Triangle& triangle, float edgeX);
[[nodiscard]] std::basic_string<Triangle> clipTriangleLeftOfPlane(const Triangle& triangle, float edgeX);
[[nodiscard]] std::basic_string<Triangle> clipTriangleToRectanglePlanes(const Triangle& triangle, const Rectangle& rectangle);
