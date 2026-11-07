/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: ChArUco board, detection and D-07 object points. Owner implements detection and
           object points by hand (D-06); the board constructor is plumbing that tests need.
*/
#include "core/charuco.h"

#include "core/board_spec.h"

namespace calibar {

const cv::aruco::CharucoBoard& charucoBoard() {
    static const cv::aruco::CharucoBoard board(
        cv::Size(board::kSquaresX, board::kSquaresY), board::kSquareLength, board::kMarkerLength,
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250));
    return board;
}

CharucoDetection detectCharuco(const cv::Mat& /*frame*/) {
    // TODO(owner, D-06): run cv::aruco::CharucoDetector::detectBoard on charucoBoard() and fill
    // all four CharucoDetection vectors.
    return {};
}

std::vector<cv::Point3f> objectPointsFor(const std::vector<int>& /*ids*/) {
    // TODO(owner, D-06): map id -> (col*L, -row*L, 0) per D-07, col = id % (kSquaresX-1),
    // row = id / (kSquaresX-1); CharucoBoard::getChessboardCorners is +y down, don't return it.
    return {};
}

}  // namespace calibar
