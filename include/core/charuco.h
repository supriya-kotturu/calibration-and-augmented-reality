/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Public ChArUco API: board, detection, and the only sanctioned source of 3D board
           points (D-07 world frame).
*/

#pragma once

#include <opencv2/core.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <vector>

namespace calibar {

struct CharucoDetection {
    std::vector<cv::Point2f> corners;  // interpolated ChArUco corners, pixels
    std::vector<int> ids;              // ChArUco corner ids, parallel to `corners`
    std::vector<std::vector<cv::Point2f>> markerCorners;
    std::vector<int> markerIds;
};

// The board from board_spec.h (9x6, square 1, marker 0.7) with DICT_6X6_250. Built once; the
// reference stays valid for the program's lifetime.
const cv::aruco::CharucoBoard& charucoBoard();

// Detects the board in a BGR or grayscale frame. Partial views are fine: `corners`/`ids` hold
// whatever subset was found. Nothing found -> all vectors empty (never throws for that).
CharucoDetection detectCharuco(const cv::Mat& frame);

// 3D board points for ChArUco corner `ids`, in board squares (kSquareLength = 1), in the same
// order as `ids`. D-07 world frame: origin at the top-left inner corner (id 0), +x right,
// +y up, +z toward the viewer, so the corner one row below id 0 is (0, -1, 0). All z == 0.
// This is the ONLY allowed source of 3D board points: OpenCV's own board corner coordinates
// use a different frame (+y down) and must not be used by callers.
// Ids must be in [0, (kSquaresX-1)*(kSquaresY-1)).
std::vector<cv::Point3f> objectPointsFor(const std::vector<int>& ids);

}  // namespace calibar
