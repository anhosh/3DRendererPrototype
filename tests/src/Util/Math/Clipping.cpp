#include <Catch/StringConverters.hpp>

#include <Util/Math/Clipping.hpp>
#include <Util/Math/LineSegment.hpp>
#include <Util/Math/Rectangle.hpp>
#include <Util/Math/Triangle.hpp>
#include <Util/Math/Vectors.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using namespace Catch::Matchers;

TEST_CASE("Segment-line intersections") {
  SECTION("Intersections of axes at origin") {
    constexpr LineSegment3D xAxis = { -DIRECTION_RIGHT, DIRECTION_RIGHT };
    constexpr LineSegment3D yAxis = { -DIRECTION_UP, DIRECTION_UP };
    constexpr LineSegment3D zAxis = { -DIRECTION_FORWARD, DIRECTION_FORWARD };

    CHECK(segmentIntersectionYZ(xAxis, 0.0f) == glm::vec3(0.0f));
    CHECK(segmentIntersectionXZ(yAxis, 0.0f) == glm::vec3(0.0f));
    CHECK(segmentIntersectionXY(zAxis, 0.0f) == glm::vec3(0.0f));
  }

  SECTION("Diagonal intersections") {
    constexpr LineSegment3D segment0 = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 3.0f, 0.0f) };
    CHECK(segmentIntersectionXZ(segment0, -1.0f) == glm::vec3(-1.0f, -1.0f, 0.0f));
    CHECK(segmentIntersectionXZ(segment0, 0.0f) == glm::vec3(-0.5f, 0.0f, 0.0f));
    CHECK(segmentIntersectionXZ(segment0, 1.0f) == glm::vec3(0.0f, 1.0f, 0.0f));
    CHECK(segmentIntersectionXZ(segment0, 2.0f) == glm::vec3(0.5f, 2.0f, 0.0f));
    CHECK(segmentIntersectionXZ(segment0, 3.0f) == glm::vec3(1.0f, 3.0f, 0.0f));
    CHECK(segmentIntersectionXZ(segment0, 3.0f) == glm::vec3(1.0f, 3.0f, 0.0f));

    constexpr LineSegment3D segment1 = { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 2.0f, 0.0f) };
    CHECK(segmentIntersectionYZ(segment1, -1.0f) == glm::vec3(-1.0f, 0.0f, 0.0f));
    CHECK(segmentIntersectionYZ(segment1, -0.5f) == glm::vec3(-0.5f, 0.5f, 0.0f));
    CHECK(segmentIntersectionYZ(segment1, 0.0f) == glm::vec3(0.0f, 1.0f, 0.0f));
    CHECK(segmentIntersectionYZ(segment1, 0.5f) == glm::vec3(0.5f, 1.5f, 0.0f));
    CHECK(segmentIntersectionYZ(segment1, 1.0f) == glm::vec3(1.0f, 2.0f, 0.0f));
  }

  SECTION("No intersections") {
    constexpr LineSegment3D straySegment = { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-0.5f, -0.5f, -0.5f) };
    CHECK_FALSE(segmentIntersectionYZ(straySegment, 0.0f));
    CHECK_FALSE(segmentIntersectionXZ(straySegment, 0.0f));
    CHECK_FALSE(segmentIntersectionXY(straySegment, 0.0f));
  }
}

TEST_CASE("Triangle-plane clipping") {
  SECTION("Above plane") {
    constexpr Triangle allAbove = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(2.0f, 1.25f, 1.5f), glm::vec3(3.75f, 0.5f, -4.0f) };
    CHECK_THAT(clipTriangleAbovePlane(allAbove, 0.0f), UnorderedEquals(std::vector { allAbove }));

    constexpr Triangle twoPointsAbove = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    CHECK_THAT(clipTriangleAbovePlane(twoPointsAbove, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) }
    }));

    constexpr Triangle onePointAbove = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f) };
    CHECK_THAT(clipTriangleAbovePlane(onePointAbove, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
    }));

    constexpr Triangle noneAbove = { glm::vec3(1.0f, -1.0f, 0.5f), glm::vec3(2.0f, -1.25f, 1.5f), glm::vec3(3.75f, -0.5f, -4.0f) };
    CHECK_THAT(clipTriangleAbovePlane(noneAbove, 0.0f), IsEmpty());
  }

  SECTION("Below plane") {
    constexpr Triangle allBelow = { glm::vec3(1.0f, -1.0f, 0.5f), glm::vec3(2.0f, -1.25f, 1.5f), glm::vec3(3.75f, -0.5f, -4.0f) };
    CHECK_THAT(clipTriangleBelowPlane(allBelow, 0.0f), UnorderedEquals(std::vector { allBelow }));

    constexpr Triangle twoPointsBelow = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f) };
    CHECK_THAT(clipTriangleBelowPlane(twoPointsBelow, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f) },
    }));

    constexpr Triangle onePointBelow = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    CHECK_THAT(clipTriangleBelowPlane(onePointBelow, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    }));

    constexpr Triangle noneBelow = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(2.0f, 1.25f, 1.5f), glm::vec3(3.75f, 0.5f, -4.0f) };
    CHECK_THAT(clipTriangleBelowPlane(noneBelow, 0.0f), IsEmpty());
  }

  SECTION("Right of plane") {
    constexpr Triangle allRightOf = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(1.25f, 2.0f, 1.5f), glm::vec3(0.5f, 3.75f, -4.0f) };
    CHECK_THAT(clipTriangleRightOfPlane(allRightOf, 0.0f), UnorderedEquals(std::vector { allRightOf }));

    constexpr Triangle twoPointsRightOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    CHECK_THAT(clipTriangleRightOfPlane(twoPointsRightOf, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
      Triangle { glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) },
    }));

    constexpr Triangle onePointRightOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
    CHECK_THAT(clipTriangleRightOfPlane(onePointRightOf, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
    }));

    constexpr Triangle noneRightOf = { glm::vec3(-1.0f, 1.0f, 0.5f), glm::vec3(-1.25f, 2.0f, 1.5f), glm::vec3(-0.5f, 3.75f, -4.0f) };
    CHECK_THAT(clipTriangleRightOfPlane(noneRightOf, 0.0f), IsEmpty());
  }

  SECTION("Left of plane") {
    constexpr Triangle allLeftOf = { glm::vec3(-1.0f, 1.0f, 0.5f), glm::vec3(-1.25f, 2.0f, 1.5f), glm::vec3(-0.5f, 3.75f, -4.0f) };
    CHECK_THAT(clipTriangleLeftOfPlane(allLeftOf, 0.0f), UnorderedEquals(std::vector { allLeftOf }));

    constexpr Triangle twoPointsLeftOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
    CHECK_THAT(clipTriangleLeftOfPlane(twoPointsLeftOf, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) },
    }));

    constexpr Triangle onePointLeftOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    CHECK_THAT(clipTriangleLeftOfPlane(onePointLeftOf, 0.0f), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    }));

    constexpr Triangle noneLeftOf = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(1.25f, 2.0f, 1.5f), glm::vec3(0.5f, 3.75f, -4.0f) };
    CHECK_THAT(clipTriangleLeftOfPlane(noneLeftOf, 0.0f), IsEmpty());
  }
}

TEST_CASE("Triangle-rectangle clipping") {
  SECTION("All outside") {
    constexpr Rectangle rectangle = { .min = glm::vec2(-1.0f), .max = glm::vec2(1.0f) };
    const auto triangles = GENERATE(
      Triangle { glm::vec3(-0.5f, 2.0f, 0.0f),  glm::vec3(-0.5f, 1.5f, 0.0f),  glm::vec3(0.5f, 2.0f, 0.0f)   },
      Triangle { glm::vec3(-0.5f, -2.0f, 0.0f), glm::vec3(-0.5f, -1.5f, 0.0f), glm::vec3(0.5f, -2.0f, 0.0f)  },
      Triangle { glm::vec3(2.0f, -0.5f, 0.0f),  glm::vec3(1.5f, -0.5f, 0.0f),  glm::vec3(2.0f, 0.5f, 0.0f)   },
      Triangle { glm::vec3(-2.0f, -0.5f, 0.0f), glm::vec3(-1.5f, -0.5f, 0.0f), glm::vec3(-2.0f, 0.5f, 0.0f)  },
      Triangle { glm::vec3(-2.0f, 2.0f, 0.0f),  glm::vec3(-2.0f, 1.5f, 0.0f),  glm::vec3(-1.5f, 2.0f, 0.0f)  },
      Triangle { glm::vec3(-2.0f, -2.0f, 0.0f), glm::vec3(-2.0f, -1.5f, 0.0f), glm::vec3(-1.5f, -2.0f, 0.0f) },
      Triangle { glm::vec3(2.0f, 2.0f, 0.0f),   glm::vec3(2.0f, 1.5f, 0.0f),   glm::vec3(1.5f, 2.0f, 0.0f)   },
      Triangle { glm::vec3(2.0f, -2.0f, 0.0f),  glm::vec3(2.0f, -1.5f, 0.0f),  glm::vec3(1.5f, -2.0f, 0.0f)  }
    );
    CHECK_THAT(clipTriangleToRectanglePlanes(triangles, rectangle), IsEmpty());
  }

  SECTION("All inside") {
    constexpr Rectangle rectangle = { .min = glm::vec2(-1.0f), .max = glm::vec2(1.0f) };
    const auto triangles = GENERATE(
      Triangle { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -0.5f, 0.0f), glm::vec3(-1.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) },
      Triangle { glm::vec3(-0.5f, -1.0f, 0.0f), glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) }
    );
    CHECK_THAT(clipTriangleToRectanglePlanes(triangles, rectangle), UnorderedEquals(std::vector { triangles }));
  }

  SECTION("Partially inside") {
    constexpr Rectangle rectangle = { .min = glm::vec2(-1.0f), .max = glm::vec2(1.0f) };

    constexpr Triangle crossOneEdge = { glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(-2.0f, 1.5f, 0.0f), glm::vec3(-0.5f, 0.0f, 0.0f) };
    CHECK_THAT(clipTriangleToRectanglePlanes(crossOneEdge, rectangle), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.5f, 0.0f), glm::vec3(-0.5f, 0.0f, 0.0f) },
    }));

    constexpr Triangle crossTwoEdges = { glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0, 0.0f, 0.0f) };
    CHECK_THAT(clipTriangleToRectanglePlanes(crossTwoEdges, rectangle), UnorderedEquals(std::vector {
      Triangle { glm::vec3(-1.0f, -0.5f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -0.5f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
      Triangle { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
    }));
  }
}
