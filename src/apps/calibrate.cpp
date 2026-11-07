/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Live calibration tool (Tasks 1-3): ChArUco detection overlay, calibration-frame
           selection, calibrateCamera and intrinsics export.
           Usage: calibrate [source] [--intrinsics path] [--headless] [--max-frames N]
           Headless: no windows; auto-saves every kAutoSaveEvery-th successful detection (up to
           kAutoSaveMax), calibrates at the end if >= 5, writes intrinsics only with --intrinsics.
*/
#include <iomanip>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "calibrate/session.h"
#include "core/app_args.h"
#include "core/charuco.h"
#include "core/intrinsics_io.h"
#include "core/video_source.h"

namespace {

constexpr int kAutoSaveEvery = 15;
constexpr int kAutoSaveMax = 15;

void putHud(cv::Mat& img, const std::vector<std::string>& lines) {
    int y = 40;
    for (const auto& l : lines) {
        cv::putText(img, l, {15, y}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {0, 0, 0}, 4);
        cv::putText(img, l, {15, y}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {0, 255, 255}, 2);
        y += 42;
    }
}

std::string firstCornerText(const calibar::CharucoDetection& det) {
    if (det.corners.empty()) return "first corner: -";
    std::ostringstream o;
    o << "first corner: id " << det.ids[0] << " @ (" << std::fixed << std::setprecision(1)
      << det.corners[0].x << ", " << det.corners[0].y << ")";
    return o.str();
}

void writeResult(const calibar::calib::Session& s, const std::string& path) {
    if (!s.result) {
        std::cerr << "nothing to write: calibrate first (c)\n";
        return;
    }
    calibar::saveIntrinsics(path, *s.result);
    std::cout << "wrote intrinsics to " << path << "\n";
}

int runHeadless(cv::VideoCapture& cap, const calibar::AppArgs& args) {
    calibar::calib::Session s;
    const int stride = (args.maxFrames > 0 && args.maxFrames <= 100) ? 1 : 10;
    int frameNo = 0, detected = 0;
    cv::Mat frame;
    while ((args.maxFrames == 0 || frameNo < args.maxFrames) && cap.read(frame) && !frame.empty()) {
        const auto det = calibar::detectCharuco(frame);
        calibar::calib::noteDetection(s, frame, det);
        if (static_cast<int>(det.corners.size()) >= calibar::calib::kMinCorners) {
            ++detected;
            if (detected % kAutoSaveEvery == 0 &&
                static_cast<int>(s.cornerList.size()) < kAutoSaveMax) {
                calibar::calib::saveLast(s, false);
            }
        }
        if (frameNo % stride == 0) {
            std::cout << "frame " << frameNo << ": " << det.corners.size() << " corners, "
                      << det.markerIds.size() << " markers, saved " << s.cornerList.size() << "\n";
        }
        ++frameNo;
    }
    std::cout << "frames: " << frameNo << ", with board: " << detected
              << ", saved: " << s.cornerList.size() << "\n";
    if (calibar::calib::calibrate(s) && !args.intrinsicsPath.empty()) {
        writeResult(s, args.intrinsicsPath);
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const calibar::AppArgs args = calibar::parseAppArgs(argc, argv);
    if (!args.ok) {
        std::cerr << args.error << "\n" << calibar::appUsage("calibrate") << "\n";
        return 2;
    }
    cv::VideoCapture cap = calibar::openSource(args.source);
    if (!cap.isOpened()) {
        std::cerr << "could not open source: " << args.source << "\n";
        return 1;
    }
    // Intrinsics are only valid at the resolution they were calibrated at, so log it.
    std::cout << "camera resolution: " << static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH))
              << "x" << static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)) << "\n";

    if (args.headless) return runHeadless(cap, args);

    std::cout << "keys: s save last good detection | c calibrate (>=5 saves) | w write intrinsics"
                 " | q/ESC quit\n";
    cv::namedWindow("calibrate", cv::WINDOW_NORMAL);
    cv::resizeWindow("calibrate", 540, 960);

    const std::string outPath =
        args.intrinsicsPath.empty() ? calibar::calib::kDefaultIntrinsics : args.intrinsicsPath;
    calibar::calib::Session s;
    cv::Mat frame;
    int frameNo = 0;
    while ((args.maxFrames == 0 || frameNo < args.maxFrames) && cap.read(frame) && !frame.empty()) {
        ++frameNo;
        const auto det = calibar::detectCharuco(frame);
        calibar::calib::noteDetection(s, frame, det);

        cv::Mat view = frame.clone();
        calibar::calib::drawDetection(view, det);
        std::ostringstream rms;
        if (s.lastRms >= 0) {
            rms << std::fixed << std::setprecision(3) << s.lastRms << " px";
        } else {
            rms << "-";
        }
        putHud(view, {"corners: " + std::to_string(det.corners.size()), firstCornerText(det),
                      "saved: " + std::to_string(s.cornerList.size()), "last RMS: " + rms.str()});
        cv::imshow("calibrate", view);

        const int key = cv::waitKey(1);
        if (key == 'q' || key == 27) break;
        if (key == 's') calibar::calib::saveLast(s, true);
        if (key == 'c') calibar::calib::calibrate(s);
        if (key == 'w') writeResult(s, outPath);
    }
    return 0;
}
