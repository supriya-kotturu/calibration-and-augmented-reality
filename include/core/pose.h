/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Camera pose from a ChArUco detection (solvePnP).
*/

#pragma once

#include <opencv2/core.hpp>
#include <optional>

#include "core/charuco.h"
#include "core/intrinsics_io.h"

namespace calibar {

// Board-to-camera transform: X_cam = R(rvec) * X_board + tvec. rvec is a Rodrigues vector in
// radians; tvec is in board squares (D-07 world frame, so the board's +z faces the viewer).
struct Pose {
    cv::Vec3d rvec;
    cv::Vec3d tvec;
};

// Pose via solvePnP (SOLVEPNP_ITERATIVE) on objectPointsFor(det.ids) <-> det.corners.
// Returns nullopt if fewer than `minCorners` corners were detected or solvePnP fails.
// Callers must have already rejected `intr` if intr.imageSize differs from the frame.
std::optional<Pose> estimatePose(const CharucoDetection& det, const Intrinsics& intr,
                                 int minCorners = 6);

}  // namespace calibar
