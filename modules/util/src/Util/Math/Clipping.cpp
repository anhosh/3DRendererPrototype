#include <Util/Math/Clipping.hpp>

#include <Util/Math/LineSegment.hpp>
#include <Util/Math/Rectangle.hpp>
#include <Util/Math/Triangle.hpp>

#include <functional>
#include <ranges>

std::optional<glm::vec3> segmentIntersectionYZ(const LineSegment3D& segment, const float x) {
  if ((segment.a.x < x && segment.b.x < x) || (segment.a.x > x && segment.b.x > x)) {
    return std::nullopt;
  }
  const float dx = segment.b.x - segment.a.x;
  const float dy = segment.b.y - segment.a.y;
  const float dz = segment.b.z - segment.a.z;
  const float dyOverDx = dx == 0.0f ? 0.0f : dy / dx;
  const float dzOverDx = dx == 0.0f ? 0.0f : dz / dx;
  const float deltaX = x - segment.a.x;
  return glm::vec3(x, segment.a.y + deltaX * dyOverDx, segment.a.z + deltaX * dzOverDx);
}

std::optional<glm::vec3> segmentIntersectionXZ(const LineSegment3D& segment, const float y) {
  return segmentIntersectionYZ({ segment.a.yxz(), segment.b.yxz() }, y)
    .and_then([](const glm::vec3& p) { return std::optional(p.yxz()); });
}

std::optional<glm::vec3> segmentIntersectionXY(const LineSegment3D& segment, const float z) {
  return segmentIntersectionYZ({ segment.a.zyx(), segment.b.zyx() }, z)
    .and_then([](const glm::vec3& p) { return std::optional(p.zyx()); });
}

template <std::invocable<glm::vec3> FnPredIsInside, std::invocable<LineSegment3D> FnIntersection>
requires std::same_as<bool, std::invoke_result_t<FnPredIsInside, glm::vec3>> &&
         std::same_as<std::optional<glm::vec3>, std::invoke_result_t<FnIntersection, LineSegment3D>>
std::vector<Triangle> clipTriangleAgainstPlane(const Triangle& tri, FnPredIsInside isInside, FnIntersection getIntersection) {
  ZoneScoped;

  std::vector<glm::vec3> points;
  for (uint32_t edge = 0; edge < 3; ++edge) {
    const glm::vec3& currPoint = tri.points[edge];
    const glm::vec3& nextPoint = tri.points[(edge + 1) % 3];
    if (std::invoke(isInside, currPoint)) {
      if (std::invoke(isInside, nextPoint)) {
        points.push_back(currPoint);
      } else {
        points.push_back(std::invoke(getIntersection, LineSegment3D { nextPoint, currPoint }).value());
        if (points.back() != currPoint) {
          points.push_back(currPoint);
        }
      }
    } else if (std::invoke(isInside, nextPoint)) {
      points.push_back(std::invoke(getIntersection, LineSegment3D { nextPoint, currPoint }).value());
    }
  }

  if (points.empty()) {
    return {};
  }

  assert(points.size() == 3 || points.size() == 4);
  std::vector<Triangle> result;
  result.reserve(points.size() - 3);
  result.push_back(Triangle { points[0], points[1], points[2] });
  if (points.size() == 4) {
    result.push_back(Triangle { points[1], points[2], points[3] });
  }
  return result;
}

std::vector<Triangle> clipTriangleAbovePlane(const Triangle& triangle, const float edgeY) {
  return clipTriangleAgainstPlane(triangle, [=](const glm::vec3 p) { return p.y >= edgeY; },
                                  std::bind_back(&segmentIntersectionXZ, edgeY));
}

std::vector<Triangle> clipTriangleBelowPlane(const Triangle& triangle, const float edgeY) {
  return clipTriangleAgainstPlane(triangle, [=](const glm::vec3 p) { return p.y <= edgeY; },
                                  std::bind_back(&segmentIntersectionXZ, edgeY));
}

std::vector<Triangle> clipTriangleRightOfPlane(const Triangle& triangle, const float edgeX) {
  return clipTriangleAgainstPlane(triangle, [=](const glm::vec3 p) { return p.x >= edgeX; },
                                  std::bind_back(&segmentIntersectionYZ, edgeX));
}

std::vector<Triangle> clipTriangleLeftOfPlane(const Triangle& triangle, const float edgeX) {
  return clipTriangleAgainstPlane(triangle, [=](const glm::vec3 p) { return p.x <= edgeX; },
                                  std::bind_back(&segmentIntersectionYZ, edgeX));
}

std::vector<Triangle> clipTriangleToRectanglePlanes(const Triangle& triangle, const Rectangle& rectangle) {
  ZoneScoped;

  const std::vector<Triangle> firstPass = clipTriangleAbovePlane(triangle, rectangle.min.y);

  std::vector<Triangle> secondPass;
  for (const Triangle& tri : firstPass) {
    secondPass.append_range(clipTriangleBelowPlane(tri, rectangle.max.y));
  }

  std::vector<Triangle> thirdPass;
  for (const Triangle& tri : secondPass) {
    thirdPass.append_range(clipTriangleRightOfPlane(tri, rectangle.min.x));
  }

  std::vector<Triangle> fourthPass;
  for (const Triangle& tri : thirdPass) {
    fourthPass.append_range(clipTriangleLeftOfPlane(tri, rectangle.max.x));
  }

  return fourthPass;
}
