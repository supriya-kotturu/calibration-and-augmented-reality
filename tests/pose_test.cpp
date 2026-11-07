/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Detection + pose recovery on a synthetic board with known ground truth. Spec for the
           owner's detectCharuco and estimatePose (D-06).
*/
#include <gtest/gtest.h>

#include <opencv2/calib3d.hpp>

#include "core/charuco.h"
#include "core/pose.h"
#include "synthetic_board.h"

using namespace calibar;
using namespace calibar::testing;

namespace {

// Camera-frame depth of a board-frame point under `pose`.
double depthOf(const Pose& pose, const cv::Point3d& p) {
    cv::Mat R;
    cv::Rodrigues(pose.rvec, R);
    const cv::Mat pc = R * cv::Mat(cv::Vec3d(p.x, p.y, p.z)) + cv::Mat(pose.tvec);
    return pc.at<double>(2);
}

}  // namespace

TEST(SyntheticScene, TruthPlacesPlusZTowardCamera) {
    // Guards the fixture itself: if this fails the test scene, not the core, is wrong.
    const auto scene = makeScene(0, 0);
    EXPECT_LT(depthOf(scene.truth, {0, 0, 1}), depthOf(scene.truth, {0, 0, 0}))
        << "scene builder puts board +z away from the camera";
}

TEST(DetectCharuco, FindsMostCornersOnFrontalBoard) {
    const auto scene = makeScene(0, 0);
    const auto det = detectCharuco(scene.frame);
    EXPECT_GE(det.corners.size(), 36u) << "expected nearly all 40 inner corners on a clean view";
    EXPECT_EQ(det.corners.size(), det.ids.size()) << "corners and ids must be parallel";
    EXPECT_FALSE(det.markerIds.empty());
}

TEST(DetectCharuco, EmptyFrameFindsNothing) {
    const cv::Mat blank(720, 1280, CV_8UC3, cv::Scalar(128, 128, 128));
    const auto det = detectCharuco(blank);
    EXPECT_TRUE(det.corners.empty());
    EXPECT_TRUE(det.ids.empty());
}

// Chains detect -> objectPointsFor -> truth projection: points off by a frame flip (D-07) or a
// wrong id mapping show up here before pose even runs.
TEST(DetectCharuco, CornersMatchGroundTruthProjection) {
    const auto scene = makeScene(15, -20);
    const auto det = detectCharuco(scene.frame);
    ASSERT_GE(det.corners.size(), 20u) << "too few corners to compare";

    std::vector<cv::Point2f> projected;
    cv::projectPoints(objectPointsFor(det.ids), scene.truth.rvec, scene.truth.tvec,
                      scene.intrinsics.cameraMatrix, scene.intrinsics.distCoeffs, projected);
    ASSERT_EQ(projected.size(), det.corners.size());
    for (size_t i = 0; i < projected.size(); ++i) {
        EXPECT_NEAR(projected[i].x, det.corners[i].x, 2.0) << "id " << det.ids[i];
        EXPECT_NEAR(projected[i].y, det.corners[i].y, 2.0) << "id " << det.ids[i];
    }
}

class PoseRecovery : public ::testing::TestWithParam<std::pair<double, double>> {};

TEST_P(PoseRecovery, RecoversTranslationAndRotation) {
    const auto [tx, ty] = GetParam();
    const auto scene = makeScene(tx, ty);
    const auto det = detectCharuco(scene.frame);
    const auto pose = estimatePose(det, scene.intrinsics);
    ASSERT_TRUE(pose.has_value()) << "no pose from " << det.corners.size() << " corners";

    const double tNorm = cv::norm(scene.truth.tvec);
    for (int i = 0; i < 3; ++i) {
        EXPECT_NEAR(pose->tvec[i], scene.truth.tvec[i], 0.02 * tNorm)
            << "tvec[" << i << "] off by more than 2% of |t|";
    }
    EXPECT_LT(rotationErrorDeg(pose->rvec, scene.truth.rvec), 1.0) << "rotation off by > 1 deg";
}

INSTANTIATE_TEST_SUITE_P(Tilts, PoseRecovery,
                         ::testing::Values(std::make_pair(0.0, 0.0), std::make_pair(20.0, 0.0),
                                           std::make_pair(15.0, -25.0)));

TEST(PoseRecovery, PlusZFacesTheViewer) {
    const auto scene = makeScene(10, 10);
    const auto pose = estimatePose(detectCharuco(scene.frame), scene.intrinsics);
    ASSERT_TRUE(pose.has_value());
    EXPECT_LT(depthOf(*pose, {0, 0, 1}), depthOf(*pose, {0, 0, 0}))
        << "estimated board +z points away from the camera (D-07 wants toward the viewer)";
}

TEST(PoseRecovery, TooFewCornersGivesNullopt) {
    const auto scene = makeScene(0, 0);
    auto det = detectCharuco(scene.frame);
    ASSERT_GE(det.corners.size(), 6u) << "detector precondition";
    det.corners.resize(5);
    det.ids.resize(5);
    EXPECT_FALSE(estimatePose(det, scene.intrinsics, 6).has_value());
}
