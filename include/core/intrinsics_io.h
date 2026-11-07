/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Camera intrinsics and their YAML persistence, shared by calibrate (writer) and ar
           (reader).
*/

#pragma once

#include <opencv2/core.hpp>
#include <optional>
#include <string>

namespace calibar {

struct Intrinsics {
    cv::Mat cameraMatrix;  // 3x3, CV_64F
    cv::Mat distCoeffs;    // Nx1 or 1xN, CV_64F (N = 0, 4, 5, 8, ...)
    cv::Size imageSize;    // pixels the calibration was made at
    double rmsPx = 0.0;    // reprojection RMS from calibrateCamera, pixels
    int numFrames = 0;     // frames used
};

// Writes `in` with cv::FileStorage (YAML). Layout, all top-level keys:
//   camera_matrix: 3x3 cv::Mat (CV_64F)
//   dist_coeffs:   cv::Mat (CV_64F)
//   image_width:   int
//   image_height:  int
//   rms_px:        double
//   num_frames:    int
// Throws std::runtime_error if the file cannot be opened for writing.
void saveIntrinsics(const std::string& path, const Intrinsics& in);

// Reads the layout above. Returns std::nullopt if the file is missing, unreadable or lacks any
// key. Intrinsics are only valid at the resolution they were calibrated at: callers MUST reject
// an Intrinsics whose `imageSize` differs from the frame they are about to use it on.
std::optional<Intrinsics> loadIntrinsics(const std::string& path);

}  // namespace calibar
