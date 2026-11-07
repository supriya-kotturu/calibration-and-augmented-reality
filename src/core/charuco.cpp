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
    // TODO(owner, D-06): detect the board and fill all four CharucoDetection vectors. Start from
    // cv::aruco::CharucoDetector (objdetect module). OpenCV 5 pitfall: hints.md H-06.
    return {};
}

std::vector<cv::Point3f> objectPointsFor(const std::vector<int>& /*ids*/) {
    // TODO(owner, D-06): map each ChArUco corner id to its 3D point in the D-07 frame (contract in
    // charuco.h). First work out how ids are laid out on the board.
    return {};
}

}  // namespace calibar
