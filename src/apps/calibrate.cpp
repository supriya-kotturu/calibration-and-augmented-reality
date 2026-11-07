/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Live camera loop; grows into Tasks 1-3 (ChArUco detection, calibration-frame
           selection, calibration).
           Usage: calibrate [source]  (device index, video file, or stream URL; default 0)
*/
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <string>

namespace {

// VideoCapture treats an int as a device and a string as a file/URL; "0" as a string would
// be opened as a file named "0".
bool isDeviceIndex(const std::string& s) {
    return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

}  // namespace

int main(int argc, char** argv) {
    const std::string source = argc > 1 ? argv[1] : "0";

    cv::VideoCapture cap;
    if (isDeviceIndex(source)) {
        cap.open(std::stoi(source));
    } else {
        cap.open(source);
    }

    if (!cap.isOpened()) {
        std::cerr << "could not open source: " << source << "\n";
        return 1;
    }

    // Intrinsics are only valid at the resolution they were calibrated at, so log it.
    const int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    const int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    std::cout << "camera resolution: " << width << "x" << height << "\n";

    cv::Mat frame;
    for (;;) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "empty frame, exiting...\n";
            break;
        }

        cv::imshow("calibrate", frame);

        const int key = cv::waitKey(1);
        if (key == 'q' || key == 27) break;
    }
    return 0;
}
