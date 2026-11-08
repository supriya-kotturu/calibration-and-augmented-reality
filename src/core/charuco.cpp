/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: The one ChArUco board every app detects, its detector, and the only mapping from
           corner ids to D-07 world points, so calibration and pose can't disagree on the frame.
*/
#include "core/charuco.h"

#include "core/board_spec.h"

namespace calibar {

// Function-local static: built once on first use, and a const reference stays valid for callers.
const cv::aruco::CharucoBoard& charucoBoard() {
    static const cv::aruco::CharucoBoard board(
        cv::Size(board::kSquaresX, board::kSquaresY), board::kSquareLength, board::kMarkerLength,
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250));
    return board;
}

CharucoDetection detectCharuco(const cv::Mat& frame) {
    CharucoDetection det;
    if (frame.empty()) return det;

    // Built once: the detector copies the board and dictionary; rebuilding it per frame is waste.
    static const cv::aruco::CharucoDetector detector(charucoBoard());
    // Vector outputs, not cv::Mat: with Mat outputs OpenCV 5 found 1 corner instead of 40 (H-06).
    detector.detectBoard(frame, det.corners, det.ids, det.markerCorners, det.markerIds);
    return det;
}

std::vector<cv::Point3f> objectPointsFor(const std::vector<int>& ids) {
    // OpenCV numbers inner corners row-major from the top-left, (kSquaresX - 1) per row. We build
    // the points here instead of using board.getChessboardCorners(), which is +y down (D-07).
    const int cornersPerRow = board::kSquaresX - 1;
    std::vector<cv::Point3f> pts;
    pts.reserve(ids.size());

    for (const int id : ids) {
        const int col = id % cornersPerRow;
        const int row = id / cornersPerRow;
        // Rows go down the board, which is -y in D-07; z = 0 because the board is the z = 0 plane.
        pts.emplace_back(col * board::kSquareLength, -row * board::kSquareLength, 0.0f);
    }
    return pts;
}

}  // namespace calibar
