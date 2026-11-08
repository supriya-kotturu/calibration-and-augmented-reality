/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: AR app (Tasks 4-6): pose from the ChArUco board, axes + board outline, and a virtual
           line object floating above the board.
*/
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

#include "core/app_args.h"
#include "core/charuco.h"
#include "core/draw.h"
#include "core/intrinsics_io.h"
#include "core/objects.h"
#include "core/pose.h"
#include "core/video_source.h"

namespace {

using namespace calibar;

constexpr const char* kDefaultIntrinsics = "data/calib/phone.yml";
constexpr const char* kShotDir = "data/ar";

// Outer inner-corners of the 9x6 board (8x5 inner corners) in the D-07 frame.
const std::vector<Segment3D>& boardOutline() {
    static const std::vector<Segment3D> o = {
        {{0, 0, 0}, {7, 0, 0}, cv::Scalar(0, 255, 255)},
        {{7, 0, 0}, {7, -4, 0}, cv::Scalar(0, 255, 255)},
        {{7, -4, 0}, {0, -4, 0}, cv::Scalar(0, 255, 255)},
        {{0, -4, 0}, {0, 0, 0}, cv::Scalar(0, 255, 255)},
    };
    return o;
}

// One stdout line per frame (Task 4): rvec/tvec in board squares, easy to grep or plot later.
std::string poseLine(int frame, const Pose& p) {
    char buf[200];
    std::snprintf(buf, sizeof buf, "frame %d rvec [%.3f %.3f %.3f] tvec [%.3f %.3f %.3f]", frame,
                  p.rvec[0], p.rvec[1], p.rvec[2], p.tvec[0], p.tvec[1], p.tvec[2]);
    return buf;
}

// Two-line pose overlay; text scales with the frame so it stays readable from 540p to 1080p.
void hud(cv::Mat& img, const std::string& l1, const std::string& l2) {
    const double scale = std::max(0.5, img.cols / 1600.0);
    const int th = std::max(1, static_cast<int>(scale * 2));
    const int dy = static_cast<int>(35 * scale + 10);
    for (int i = 0; i < 2; ++i) {
        const std::string& t = i == 0 ? l1 : l2;
        cv::putText(img, t, {10, dy * (i + 1)}, cv::FONT_HERSHEY_SIMPLEX, scale, {0, 0, 0},
                    th + 2);
        cv::putText(img, t, {10, dy * (i + 1)}, cv::FONT_HERSHEY_SIMPLEX, scale, {255, 255, 255},
                    th);
    }
}

// First unused shot_NNN.png, so restarting the app never overwrites earlier screenshots.
std::string nextShotPath() {
    for (int n = 0;; ++n) {
        char name[64];
        std::snprintf(name, sizeof name, "%s/shot_%03d.png", kShotDir, n);
        if (!std::filesystem::exists(name)) return name;
    }
}

}  // namespace

// Tasks 4-6: load intrinsics, then per frame detect -> pose -> print -> draw axes, outline and
// the selected object. Exits 1 if the intrinsics are missing or were made at another resolution.
int main(int argc, char** argv) {
    AppArgs args = parseAppArgs(argc, argv);
    if (!args.ok) {
        std::cerr << "ar: " << args.error << "\n" << appUsage("ar") << "\n";
        return 2;
    }
    if (args.intrinsicsPath.empty()) args.intrinsicsPath = kDefaultIntrinsics;

    const auto intr = loadIntrinsics(args.intrinsicsPath);
    if (!intr) {
        std::cerr << "ar: cannot load intrinsics from '" << args.intrinsicsPath << "'\n";
        return 1;
    }

    cv::VideoCapture cap = openSource(args.source);
    if (!cap.isOpened()) {
        std::cerr << "ar: cannot open source '" << args.source << "'\n";
        return 1;
    }
    const cv::Size frameSize(static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
                             static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)));
    if (frameSize != intr->imageSize) {
        std::cerr << "ar: resolution mismatch: intrinsics are for " << intr->imageSize.width << "x"
                  << intr->imageSize.height << " but the source is " << frameSize.width << "x"
                  << frameSize.height << "; recalibrate at the source's resolution\n";
        return 1;
    }

    const std::vector<Segment3D> debug = debugCube();
    const std::vector<Segment3D> main3d = house();
    int object = 2;
    bool axes = true;
    int frames = 0, withPose = 0;

    if (!args.headless) cv::namedWindow("ar", cv::WINDOW_NORMAL);

    cv::Mat frame;
    while (cap.read(frame)) {
        if (args.maxFrames > 0 && frames >= args.maxFrames) break;
        ++frames;

        if (frame.size() != intr->imageSize) {
            std::cerr << "ar: frame " << frames << " is " << frame.cols << "x" << frame.rows
                      << ", intrinsics are for " << intr->imageSize.width << "x"
                      << intr->imageSize.height << "\n";
            return 1;
        }

        const auto pose = estimatePose(detectCharuco(frame), *intr);
        std::string l1, l2;
        if (pose) {
            ++withPose;
            std::cout << poseLine(frames, *pose) << "\n";
            if (!args.headless) {
                if (axes) drawAxes(frame, *pose, *intr, 2.0f);
                drawSegments(frame, boardOutline(), *pose, *intr);
                drawSegments(frame, object == 1 ? debug : main3d, *pose, *intr);
                char b[120];
                std::snprintf(b, sizeof b, "rvec %.2f %.2f %.2f", pose->rvec[0], pose->rvec[1],
                              pose->rvec[2]);
                l1 = b;
                std::snprintf(b, sizeof b, "tvec %.2f %.2f %.2f", pose->tvec[0], pose->tvec[1],
                              pose->tvec[2]);
                l2 = b;
            }
        } else {
            std::cout << "frame " << frames << " no pose\n";
            l1 = "no pose";
        }

        if (args.headless) continue;

        hud(frame, l1, l2);
        cv::imshow("ar", frame);
        const int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) break;
        if (key == '1') object = 1;
        if (key == '2') object = 2;
        if (key == 'a') axes = !axes;
        if (key == 'p') {
            std::filesystem::create_directories(kShotDir);
            const std::string path = nextShotPath();
            if (cv::imwrite(path, frame)) {
                std::cout << "saved " << path << "\n";
            } else {
                std::cerr << "ar: failed to write " << path << "\n";
            }
        }
    }

    std::cout << "frames " << frames << ", with pose " << withPose << "\n";
    return 0;
}
