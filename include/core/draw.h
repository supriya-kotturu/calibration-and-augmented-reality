/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Projects 3D board-frame geometry (axes, line objects) onto the frame.
*/

#pragma once

#include <opencv2/core.hpp>
#include <vector>

#include "core/intrinsics_io.h"
#include "core/pose.h"

namespace calibar {

// A 3D line segment in the D-07 board frame (squares), drawn in `color` (BGR).
struct Segment3D {
    cv::Point3f a, b;
    cv::Scalar color;
};

// Draws X (red), Y (green), Z (blue) axes from the board origin, each `length` squares long,
// in the D-07 frame (so the Z axis points toward the viewer). Draws in place on a BGR `img`.
// Callers must have rejected `intr` if its imageSize differs from img.size().
void drawAxes(cv::Mat& img, const Pose& pose, const Intrinsics& intr, float length);

// Draws each segment, projecting all endpoints with a single projectPoints call. Endpoints
// outside the image are clipped by OpenCV's line drawing, not dropped.
void drawSegments(cv::Mat& img, const std::vector<Segment3D>& segments, const Pose& pose,
                  const Intrinsics& intr, int thickness = 2);

}  // namespace calibar
