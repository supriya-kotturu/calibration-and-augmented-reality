/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Implements openSource / isDeviceIndex.
*/
#include "core/video_source.h"

namespace calibar {

bool isDeviceIndex(const std::string& s) {
    return !s.empty() && s.find_first_not_of("0123456789") == std::string::npos;
}

cv::VideoCapture openSource(const std::string& src) {
    cv::VideoCapture cap;
    if (isDeviceIndex(src)) {
        cap.open(std::stoi(src));
    } else {
        cap.open(src);
    }
    return cap;
}

}  // namespace calibar
