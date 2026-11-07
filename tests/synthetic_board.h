/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Builds a camera frame of the ChArUco board from a known intrinsics + pose, so pose
           tests have exact ground truth without a camera.
*/

#pragma once

#include <opencv2/core.hpp>

#include "core/intrinsics_io.h"
#include "core/pose.h"

namespace calibar::testing {

struct SyntheticScene {
    cv::Mat frame;  // BGR
    Intrinsics intrinsics;
    Pose truth;  // board (D-07 frame) -> camera
};

// Camera at 1280x720, fx = fy = 0.9 * width, principal point at centre, no distortion. The
// board faces the camera (+z toward the viewer, as D-07 requires), tilted by `tiltXDeg` about
// the camera x axis and `tiltYDeg` about its y axis, centred ~13 squares in front of it.
SyntheticScene makeScene(double tiltXDeg, double tiltYDeg);

// Angle in degrees between two rotation vectors' rotations.
double rotationErrorDeg(const cv::Vec3d& a, const cv::Vec3d& b);

}  // namespace calibar::testing
