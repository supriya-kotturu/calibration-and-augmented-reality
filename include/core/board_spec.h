/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Single source of truth for the ChArUco board (D-03). make_board renders it;
           calibrate and the AR apps detect it. Change a value here -> regenerate the PNG,
           or detection silently fails.
*/

#pragma once

namespace calibar::board {

constexpr int kSquaresX = 9;
constexpr int kSquaresY = 6;
constexpr float kSquareLength = 1.0f;  // world units = squares (course convention)
constexpr float kMarkerLength = 0.7f;
// Dictionary: DICT_6X6_250. Kept as a comment: the enum lives in OpenCV headers, and this
// file stays OpenCV-free.

// Rendering for Samsung Tab S10 FE at native resolution (no OS rescaling).
constexpr int kImageWidthPx = 2304;
constexpr int kImageHeightPx = 1440;
constexpr int kMarginPx = 30;
constexpr int kBorderBits = 1;

}  // namespace calibar::board
