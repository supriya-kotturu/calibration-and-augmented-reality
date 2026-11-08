/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Board pose from one ChArUco detection, so ar (and any later app) gets rvec/tvec in the
           D-07 frame without knowing how corners map to world points.
*/
#include "core/pose.h"

#include <opencv2/core.hpp>
#include <opencv2/geometry.hpp>  // solvePnP moved here from calib3d in OpenCV 5

namespace calibar {

std::optional<Pose> estimatePose(const CharucoDetection& det, const Intrinsics& intr,
                                 int minCorners) {
    // solvePnP needs only 4 points, but a handful of nearly collinear corners (a board edge)
    // gives a pose that jumps between frames; minCorners trades detection rate for stability.
    // The size check guards against a detection whose corners and ids are out of step.
    if (static_cast<int>(det.corners.size()) < minCorners || det.corners.size() != det.ids.size()) {
        return std::nullopt;
    }

    // Same id -> point mapping as calibration, so the pose is in the D-07 frame (+z to viewer).
    const std::vector<cv::Point3f> objectPts = objectPointsFor(det.ids);

    cv::Vec3d rvec, tvec;
    const bool ok = cv::solvePnP(objectPts, det.corners, intr.cameraMatrix, intr.distCoeffs, rvec,
                                 tvec, false, cv::SOLVEPNP_ITERATIVE);
    if (!ok) return std::nullopt;

    return Pose{rvec, tvec};
}

}  // namespace calibar
