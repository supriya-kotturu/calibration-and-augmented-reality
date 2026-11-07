/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Implements the synthetic ChArUco scene used by the pose tests.
*/
#include "synthetic_board.h"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include "core/board_spec.h"
#include "core/charuco.h"

namespace calibar::testing {

namespace {

constexpr int kSquarePx = 100;
constexpr int kMarginPx = 20;
constexpr double kDegToRad = CV_PI / 180.0;

cv::Matx33d rotX(double deg) {
    const double c = std::cos(deg * kDegToRad), s = std::sin(deg * kDegToRad);
    return {1, 0, 0, 0, c, -s, 0, s, c};
}

cv::Matx33d rotY(double deg) {
    const double c = std::cos(deg * kDegToRad), s = std::sin(deg * kDegToRad);
    return {c, 0, s, 0, 1, 0, -s, 0, c};
}

}  // namespace

SyntheticScene makeScene(double tiltXDeg, double tiltYDeg) {
    namespace b = calibar::board;
    SyntheticScene scene;

    const cv::Size frameSize(1280, 720);
    scene.intrinsics.imageSize = frameSize;
    const double f = 0.9 * frameSize.width;
    scene.intrinsics.cameraMatrix =
        cv::Mat(cv::Matx33d(f, 0, frameSize.width / 2.0, 0, f, frameSize.height / 2.0, 0, 0, 1))
            .clone();
    scene.intrinsics.distCoeffs = cv::Mat::zeros(5, 1, CV_64F);

    // D-07 board axes in camera axes (x right, y down, z forward): x -> +x, y(up) -> -y,
    // z(toward viewer) -> -z, i.e. a 180 degree turn about x.
    const cv::Matx33d facing(1, 0, 0, 0, -1, 0, 0, 0, -1);
    const cv::Matx33d R = rotY(tiltYDeg) * rotX(tiltXDeg) * facing;

    // Board image: top-left of the outer board at pixel (margin, margin); id 0 (the D-07
    // origin) is one square in from there.
    const cv::Size boardPx(b::kSquaresX * kSquarePx + 2 * kMarginPx,
                           b::kSquaresY * kSquarePx + 2 * kMarginPx);
    cv::Mat boardImg;
    charucoBoard().generateImage(boardPx, boardImg, kMarginPx, b::kBorderBits);

    auto pixelToWorld = [&](double px, double py) {
        return cv::Point3d((px - (kMarginPx + kSquarePx)) / kSquarePx,
                           -(py - (kMarginPx + kSquarePx)) / kSquarePx, 0.0);
    };

    // Put the board's centre 13 squares straight ahead; the origin's tvec follows from that.
    const cv::Point3d centre = pixelToWorld(boardPx.width / 2.0, boardPx.height / 2.0);
    const cv::Vec3d Rc = R * cv::Vec3d(centre.x, centre.y, centre.z);
    const cv::Vec3d tvec = cv::Vec3d(0, 0, 13.0) - Rc;

    cv::Mat rvecMat;
    cv::Rodrigues(cv::Mat(R), rvecMat);
    scene.truth.rvec = cv::Vec3d(rvecMat);
    scene.truth.tvec = tvec;

    const std::vector<cv::Point2d> srcPx = {{0, 0},
                                            {double(boardPx.width), 0},
                                            {double(boardPx.width), double(boardPx.height)},
                                            {0, double(boardPx.height)}};
    std::vector<cv::Point3d> world;
    for (const auto& p : srcPx) world.push_back(pixelToWorld(p.x, p.y));

    std::vector<cv::Point2d> dst;
    cv::projectPoints(world, scene.truth.rvec, scene.truth.tvec, scene.intrinsics.cameraMatrix,
                      scene.intrinsics.distCoeffs, dst);

    std::vector<cv::Point2f> src32, dst32;
    for (const auto& p : srcPx) src32.emplace_back(float(p.x), float(p.y));
    for (const auto& p : dst) dst32.emplace_back(float(p.x), float(p.y));

    const cv::Mat H = cv::getPerspectiveTransform(src32, dst32);
    cv::Mat warped;
    cv::warpPerspective(boardImg, warped, H, frameSize, cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                        cv::Scalar(128));
    cv::cvtColor(warped, scene.frame, cv::COLOR_GRAY2BGR);
    return scene;
}

double rotationErrorDeg(const cv::Vec3d& a, const cv::Vec3d& b) {
    cv::Mat Ra, Rb;
    cv::Rodrigues(a, Ra);
    cv::Rodrigues(b, Rb);
    const cv::Mat rel = Ra * Rb.t();
    cv::Mat relVec;
    cv::Rodrigues(rel, relVec);
    return cv::norm(relVec) / kDegToRad;
}

}  // namespace calibar::testing
