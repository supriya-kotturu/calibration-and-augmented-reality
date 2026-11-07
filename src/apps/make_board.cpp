/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Renders the D-03 ChArUco board to data/board/ for full-screen display on the
           tablet. Run from the repo root (relative output path).
*/
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <string>

#include "core/board_spec.h"

namespace b = calibar::board;

int main() {
    const std::string outPath = "data/board/charuco_9x6_dict6x6_250.png";

    const cv::aruco::Dictionary dictionary =
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

    const cv::aruco::CharucoBoard board(cv::Size(b::kSquaresX, b::kSquaresY), b::kSquareLength,
                                        b::kMarkerLength, dictionary);

    cv::Mat image;
    board.generateImage(cv::Size(b::kImageWidthPx, b::kImageHeightPx), image, b::kMarginPx,
                        b::kBorderBits);

    if (!cv::imwrite(outPath, image)) {
        std::cerr << "failed to write " << outPath << "\n";
        return 1;
    }

    std::cout << "wrote " << outPath << '\n';
    return 0;
}
