/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Calibration-frame bookkeeping and calibrateCamera wrapper for the calibrate app
           (Tasks 2-3). Header-only so the app stays one CMake target.
*/

#pragma once

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "core/charuco.h"
#include "core/intrinsics_io.h"

namespace calibar::calib {

// calibrateCamera's pose solve is unstable below this many corners per view.
constexpr int kMinCorners = 6;
constexpr int kMinFrames = 5;
constexpr const char* kImageDir = "data/calib/images";
constexpr const char* kDefaultIntrinsics = "data/calib/phone.yml";

struct Session {
    // Latest frame with a usable board. `s` on a frame without one must still save this.
    cv::Mat goodFrame;
    CharucoDetection goodDet;
    bool haveGood = false;

    std::vector<std::vector<cv::Point2f>> cornerList;
    std::vector<std::vector<cv::Point3f>> pointList;
    cv::Size imageSize;

    double lastRms = -1.0;
    std::optional<Intrinsics> result;
};

// Remembers the latest frame with enough corners, so `s` still works if the board has just
// left the view (Task 2: save the last *successful* detection).
inline void noteDetection(Session& s, const cv::Mat& frame, const CharucoDetection& det) {
    if (static_cast<int>(det.corners.size()) < kMinCorners) return;
    s.goodFrame = frame.clone();
    s.goodDet = det;
    s.haveGood = true;
}

// Draws marker outlines and the ChArUco corners with their ids.
inline void drawDetection(cv::Mat& img, const CharucoDetection& det) {
    if (!det.markerIds.empty()) {
        cv::aruco::drawDetectedMarkers(img, det.markerCorners, det.markerIds);
    }
    if (!det.ids.empty()) cv::aruco::drawDetectedCornersCharuco(img, det.corners, det.ids);
}

// Saves the last good detection. Returns false (with a reason on stderr) if there is none.
// `writeImages` is off in headless mode so test runs don't litter data/.
inline bool saveLast(Session& s, bool writeImages) {
    if (!s.haveGood) {
        std::cerr << "no board detected yet; nothing to save\n";
        return false;
    }
    s.imageSize = s.goodFrame.size();
    s.cornerList.push_back(s.goodDet.corners);
    // Per-frame subset: each view sees different ChArUco corner ids.
    s.pointList.push_back(objectPointsFor(s.goodDet.ids));

    const int n = static_cast<int>(s.cornerList.size());
    if (writeImages) {
        std::filesystem::create_directories(kImageDir);
        std::ostringstream name;
        name << kImageDir << "/calib_" << std::setw(3) << std::setfill('0') << n;
        cv::imwrite(name.str() + ".png", s.goodFrame);
        cv::Mat annotated = s.goodFrame.clone();
        drawDetection(annotated, s.goodDet);
        cv::imwrite(name.str() + "_corners.png", annotated);
    }
    std::cout << "saved frame " << n << " (" << s.goodDet.corners.size() << " corners)\n";
    return true;
}

// task.md's starting guess: f = 1, principal point at the image centre. Without
// CALIB_USE_INTRINSIC_GUESS, OpenCV keeps only the centre and the fx/fy ratio from it.
inline cv::Mat initialCameraMatrix(const cv::Size& size) {
    cv::Mat k = cv::Mat::eye(3, 3, CV_64F);
    k.at<double>(0, 2) = size.width / 2.0;
    k.at<double>(1, 2) = size.height / 2.0;
    return k;
}

// Prints K and the distortion coefficients; called before and after calibration (Task 3).
inline void printCalib(const char* label, const cv::Mat& k, const cv::Mat& dist) {
    std::cout << label << " camera matrix:\n"
              << k << "\n"
              << label << " dist coeffs: " << dist.t() << "\n";
}

// Returns false if there are too few saved frames. On success fills s.result and s.lastRms.
inline bool calibrate(Session& s) {
    if (static_cast<int>(s.cornerList.size()) < kMinFrames) {
        std::cerr << "need at least " << kMinFrames << " saved frames, have "
                  << s.cornerList.size() << "\n";
        return false;
    }
    cv::Mat k = initialCameraMatrix(s.imageSize);
    cv::Mat dist = cv::Mat::zeros(5, 1, CV_64F);
    printCalib("before:", k, dist);

    std::vector<cv::Mat> rvecs, tvecs;
    cv::Mat stdIntr, stdExtr, perView;
    const double rms =
        cv::calibrateCamera(s.pointList, s.cornerList, s.imageSize, k, dist, rvecs, tvecs, stdIntr,
                            stdExtr, perView, cv::CALIB_FIX_ASPECT_RATIO);
    printCalib("after: ", k, dist);
    std::cout << "RMS reprojection error: " << rms << " px over " << s.cornerList.size()
              << " frames\nper-view RMS (px):" << std::fixed << std::setprecision(2);
    for (int i = 0; i < perView.rows; ++i) std::cout << " " << perView.at<double>(i);
    std::cout << std::defaultfloat << "\n";

    s.lastRms = rms;
    s.result = Intrinsics{k, dist, s.imageSize, rms, static_cast<int>(s.cornerList.size())};
    return true;
}

}  // namespace calibar::calib
