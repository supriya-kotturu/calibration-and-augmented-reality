/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Opens a camera index, video file or stream URL behind one call, so every app parses
           its source argument the same way.
*/

#pragma once

#include <opencv2/videoio.hpp>
#include <string>

namespace calibar {

// True if `s` is non-empty and all ASCII digits (e.g. "0"). VideoCapture treats an int as a
// device and a string as a file/URL, so "0" must not be opened as a file named "0".
bool isDeviceIndex(const std::string& s);

// Opens `src` as a device index if isDeviceIndex(src), else as a file path or URL.
// Returns the capture; check isOpened(): failure is not an exception.
cv::VideoCapture openSource(const std::string& src);

}  // namespace calibar
