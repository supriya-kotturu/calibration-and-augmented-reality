/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Pins the D-07 world frame in objectPointsFor: origin, axes directions, spacing.
*/
#include <gtest/gtest.h>

#include <numeric>

#include "core/board_spec.h"
#include "core/charuco.h"

using namespace calibar;
namespace b = calibar::board;

namespace {
constexpr int kCornersPerRow = b::kSquaresX - 1;
constexpr int kCornerCount = (b::kSquaresX - 1) * (b::kSquaresY - 1);

std::vector<int> allIds() {
    std::vector<int> ids(kCornerCount);
    std::iota(ids.begin(), ids.end(), 0);
    return ids;
}
}  // namespace

TEST(ObjectPointsFor, OneOutputPerIdInInputOrder) {
    const auto pts = objectPointsFor({7, 0, 3});
    ASSERT_EQ(pts.size(), 3u);
    EXPECT_NEAR(pts[0].x, 7 * b::kSquareLength, 1e-5) << "id 7 must come first";
    EXPECT_NEAR(pts[1].x, 0.0, 1e-5) << "id 0 must come second";
}

TEST(ObjectPointsFor, FirstCornerIsOrigin) {
    const auto pts = objectPointsFor({0});
    ASSERT_EQ(pts.size(), 1u);
    EXPECT_NEAR(pts[0].x, 0.0, 1e-5);
    EXPECT_NEAR(pts[0].y, 0.0, 1e-5);
    EXPECT_NEAR(pts[0].z, 0.0, 1e-5);
}

TEST(ObjectPointsFor, NextColumnIsPlusX) {
    const auto pts = objectPointsFor({1});
    ASSERT_EQ(pts.size(), 1u);
    EXPECT_NEAR(pts[0].x, b::kSquareLength, 1e-5) << "+x must point right";
    EXPECT_NEAR(pts[0].y, 0.0, 1e-5);
}

TEST(ObjectPointsFor, NextRowIsMinusY) {
    const auto pts = objectPointsFor({kCornersPerRow});
    ASSERT_EQ(pts.size(), 1u);
    EXPECT_NEAR(pts[0].x, 0.0, 1e-5);
    EXPECT_NEAR(pts[0].y, -b::kSquareLength, 1e-5) << "D-07: the row below is (0, -1, 0)";
    EXPECT_NEAR(pts[0].z, 0.0, 1e-5);
}

TEST(ObjectPointsFor, FullGridIsPlanarWithSquareSpacing) {
    const auto pts = objectPointsFor(allIds());
    ASSERT_EQ(pts.size(), static_cast<size_t>(kCornerCount));
    for (int id = 0; id < kCornerCount; ++id) {
        const int col = id % kCornersPerRow;
        const int row = id / kCornersPerRow;
        EXPECT_NEAR(pts[id].x, col * b::kSquareLength, 1e-5) << "id " << id;
        EXPECT_NEAR(pts[id].y, -row * b::kSquareLength, 1e-5) << "id " << id;
        EXPECT_EQ(pts[id].z, 0.0f) << "id " << id;
    }
}

TEST(ObjectPointsFor, EmptyIdsGiveEmptyPoints) {
    EXPECT_TRUE(objectPointsFor({}).empty());
}
