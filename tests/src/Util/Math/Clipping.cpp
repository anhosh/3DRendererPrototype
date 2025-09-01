#include <Catch/StringConverters.hpp>

#include <Util/Math/Clipping.hpp>
#include <Util/Math/LineSegment.hpp>
#include <Util/Math/Rectangle.hpp>
#include <Util/Math/Triangle.hpp>
#include <Util/Math/Vectors.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Segment-line intersections") {
  SECTION("Intersections of axes at origin") {
    constexpr LineSegment3D xAxis = { -DIRECTION_RIGHT, DIRECTION_RIGHT };
    constexpr LineSegment3D yAxis = { -DIRECTION_UP, DIRECTION_UP };
    constexpr LineSegment3D zAxis = { -DIRECTION_FORWARD, DIRECTION_FORWARD };

    REQUIRE(segmentIntersectionYZ(xAxis, 0.0f) == glm::vec3(0.0f));
    REQUIRE(segmentIntersectionXZ(yAxis, 0.0f) == glm::vec3(0.0f));
    REQUIRE(segmentIntersectionXY(zAxis, 0.0f) == glm::vec3(0.0f));
  }

  SECTION("Diagonal intersections") {
    constexpr LineSegment3D segment0 = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 3.0f, 0.0f) };
    REQUIRE(segmentIntersectionXZ(segment0, -1.0f) == glm::vec3(-1.0f, -1.0f, 0.0f));
    REQUIRE(segmentIntersectionXZ(segment0, 0.0f) == glm::vec3(-0.5f, 0.0f, 0.0f));
    REQUIRE(segmentIntersectionXZ(segment0, 1.0f) == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(segmentIntersectionXZ(segment0, 2.0f) == glm::vec3(0.5f, 2.0f, 0.0f));
    REQUIRE(segmentIntersectionXZ(segment0, 3.0f) == glm::vec3(1.0f, 3.0f, 0.0f));

    constexpr LineSegment3D segment1 = { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 2.0f, 0.0f) };
    REQUIRE(segmentIntersectionYZ(segment1, -1.0f) == glm::vec3(-1.0f, 0.0f, 0.0f));
    REQUIRE(segmentIntersectionYZ(segment1, -0.5f) == glm::vec3(-0.5f, 0.5f, 0.0f));
    REQUIRE(segmentIntersectionYZ(segment1, 0.0f) == glm::vec3(0.0f, 1.0f, 0.0f));
    REQUIRE(segmentIntersectionYZ(segment1, 0.5f) == glm::vec3(0.5f, 1.5f, 0.0f));
    REQUIRE(segmentIntersectionYZ(segment1, 1.0f) == glm::vec3(1.0f, 2.0f, 0.0f));
  }

  SECTION("No intersections") {
    constexpr LineSegment3D straySegment = { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-0.5f, -0.5f, -0.5f) };
    REQUIRE_FALSE(segmentIntersectionYZ(straySegment, 0.0f));
    REQUIRE_FALSE(segmentIntersectionXZ(straySegment, 0.0f));
    REQUIRE_FALSE(segmentIntersectionXY(straySegment, 0.0f));
  }
}

TEST_CASE("Triangle-plane clipping") {
  SECTION("Above plane") {
    constexpr Triangle allAbove = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(2.0f, 1.25f, 1.5f), glm::vec3(3.75f, 0.5f, -4.0f) };
    REQUIRE(clipTriangleAbovePlane(allAbove, 0.0f) == std::basic_string<Triangle> { allAbove });

    constexpr Triangle twoPointsAbove = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    REQUIRE(clipTriangleAbovePlane(twoPointsAbove, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) }
    });

    constexpr Triangle onePointAbove = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f) };
    REQUIRE(clipTriangleAbovePlane(onePointAbove, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
    });

    constexpr Triangle noneAbove = { glm::vec3(1.0f, -1.0f, 0.5f), glm::vec3(2.0f, -1.25f, 1.5f), glm::vec3(3.75f, -0.5f, -4.0f) };
    REQUIRE(clipTriangleAbovePlane(noneAbove, 0.0f) == std::basic_string<Triangle> {});
  }

  SECTION("Below plane") {
    constexpr Triangle allBelow = { glm::vec3(1.0f, -1.0f, 0.5f), glm::vec3(2.0f, -1.25f, 1.5f), glm::vec3(3.75f, -0.5f, -4.0f) };
    REQUIRE(clipTriangleBelowPlane(allBelow, 0.0f) == std::basic_string<Triangle> { allBelow });

    constexpr Triangle twoPointsBelow = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f) };
    REQUIRE(clipTriangleBelowPlane(twoPointsBelow, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f) },
    });

    constexpr Triangle onePointBelow = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    REQUIRE(clipTriangleBelowPlane(onePointBelow, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    });

    constexpr Triangle noneBelow = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(2.0f, 1.25f, 1.5f), glm::vec3(3.75f, 0.5f, -4.0f) };
    REQUIRE(clipTriangleBelowPlane(noneBelow, 0.0f) == std::basic_string<Triangle> {});
  }

  SECTION("Right of plane") {
    constexpr Triangle allRightOf = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(1.25f, 2.0f, 1.5f), glm::vec3(0.5f, 3.75f, -4.0f) };
    REQUIRE(clipTriangleRightOfPlane(allRightOf, 0.0f) == std::basic_string<Triangle> { allRightOf });

    constexpr Triangle twoPointsRightOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    REQUIRE(clipTriangleRightOfPlane(twoPointsRightOf, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
      Triangle { glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) },
    });

    constexpr Triangle onePointRightOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
    REQUIRE(clipTriangleRightOfPlane(onePointRightOf, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
    });

    constexpr Triangle noneRightOf = { glm::vec3(-1.0f, 1.0f, 0.5f), glm::vec3(-1.25f, 2.0f, 1.5f), glm::vec3(-0.5f, 3.75f, -4.0f) };
    REQUIRE(clipTriangleRightOfPlane(noneRightOf, 0.0f) == std::basic_string<Triangle> {});
  }

  SECTION("Left of plane") {
    constexpr Triangle allLeftOf = { glm::vec3(-1.0f, 1.0f, 0.5f), glm::vec3(-1.25f, 2.0f, 1.5f), glm::vec3(-0.5f, 3.75f, -4.0f) };
    REQUIRE(clipTriangleLeftOfPlane(allLeftOf, 0.0f) == std::basic_string<Triangle> { allLeftOf });

    constexpr Triangle twoPointsLeftOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
    REQUIRE(clipTriangleLeftOfPlane(twoPointsLeftOf, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) },
    });

    constexpr Triangle onePointLeftOf = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f) };
    REQUIRE(clipTriangleLeftOfPlane(onePointLeftOf, 0.0f) == std::basic_string<Triangle> {
      Triangle { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
    });

    constexpr Triangle noneLeftOf = { glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(1.25f, 2.0f, 1.5f), glm::vec3(0.5f, 3.75f, -4.0f) };
    REQUIRE(clipTriangleLeftOfPlane(noneLeftOf, 0.0f) == std::basic_string<Triangle> {});
  }
}

TEST_CASE("Triangle-rectangle clipping") {
  SECTION("All outside") {
    constexpr Rectangle rectangle  = { .min = glm::vec2(-1.0f), .max = glm::vec2(1.0f) };
    constexpr Triangle top         = { glm::vec3(-0.5f, 2.0f, 0.0f),  glm::vec3(-0.5f, 1.5f, 0.0f),  glm::vec3(0.5f, 2.0f, 0.0f)   };
    constexpr Triangle bottom      = { glm::vec3(-0.5f, -2.0f, 0.0f), glm::vec3(-0.5f, -1.5f, 0.0f), glm::vec3(0.5f, -2.0f, 0.0f)  };
    constexpr Triangle right       = { glm::vec3(2.0f, -0.5f, 0.0f),  glm::vec3(1.5f, -0.5f, 0.0f),  glm::vec3(2.0f, 0.5f, 0.0f)   };
    constexpr Triangle left        = { glm::vec3(-2.0f, -0.5f, 0.0f), glm::vec3(-1.5f, -0.5f, 0.0f), glm::vec3(-2.0f, 0.5f, 0.0f)  };
    constexpr Triangle topLeft     = { glm::vec3(-2.0f, 2.0f, 0.0f),  glm::vec3(-2.0f, 1.5f, 0.0f),  glm::vec3(-1.5f, 2.0f, 0.0f)  };
    constexpr Triangle bottomLeft  = { glm::vec3(-2.0f, -2.0f, 0.0f), glm::vec3(-2.0f, -1.5f, 0.0f), glm::vec3(-1.5f, -2.0f, 0.0f) };
    constexpr Triangle topRight    = { glm::vec3(2.0f, 2.0f, 0.0f),   glm::vec3(2.0f, 1.5f, 0.0f),   glm::vec3(1.5f, 2.0f, 0.0f)   };
    constexpr Triangle bottomRight = { glm::vec3(2.0f, -2.0f, 0.0f),  glm::vec3(2.0f, -1.5f, 0.0f),  glm::vec3(1.5f, -2.0f, 0.0f)  };
    REQUIRE(clipTriangleToRectanglePlanes(top, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(bottom, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(right, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(left, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(topLeft, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(bottomLeft, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(topRight, rectangle) == std::basic_string<Triangle> {});
    REQUIRE(clipTriangleToRectanglePlanes(bottomRight, rectangle) == std::basic_string<Triangle> {});
  }

  SECTION("All inside") {
    constexpr Rectangle rectangle = { .min = glm::vec2(-1.0f), .max = glm::vec2(1.0f) };
    constexpr Triangle fullyIn = { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) };
    constexpr Triangle edgeOnEdge = { glm::vec3(-1.0f, -0.5f, 0.0f), glm::vec3(-1.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) };
    constexpr Triangle cornerOnCorner = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) };
    constexpr Triangle cornerOnEdge = { glm::vec3(-0.5f, -1.0f, 0.0f), glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) };
    constexpr Triangle edgesAndCorners = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) };
    REQUIRE(clipTriangleToRectanglePlanes(fullyIn, rectangle) == std::basic_string<Triangle> { fullyIn });
    REQUIRE(clipTriangleToRectanglePlanes(edgeOnEdge, rectangle) == std::basic_string<Triangle> { edgeOnEdge });
    REQUIRE(clipTriangleToRectanglePlanes(cornerOnCorner, rectangle) == std::basic_string<Triangle> { cornerOnCorner });
    REQUIRE(clipTriangleToRectanglePlanes(cornerOnEdge, rectangle) == std::basic_string<Triangle> { cornerOnEdge });
    REQUIRE(clipTriangleToRectanglePlanes(edgesAndCorners, rectangle) == std::basic_string<Triangle> { edgesAndCorners });
  }

  SECTION("Partially inside") {

  }
}
