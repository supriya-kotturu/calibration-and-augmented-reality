/*
  Author:  Sai Supriya Kotturu
  Date:    2026-11-07
  Purpose: Shared command line for the apps so they all accept the same flags and headless mode.
*/

#pragma once

#include <string>

namespace calibar {

// Usage: <app> [source] [--intrinsics path] [--headless] [--max-frames N]
//   source       device index, file or URL (default "0")
//   --headless   no imshow/waitKey; run to the end of a file source, print stats, exit 0.
//                Exists so agents and CI can verify apps without a window or camera.
//   --max-frames stop after N frames (0 = unlimited)
struct AppArgs {
    std::string source = "0";
    std::string intrinsicsPath;  // empty = not given
    bool headless = false;
    int maxFrames = 0;
    bool ok = true;  // false: `error` says why; caller prints appUsage and exits 2
    std::string error;
};

AppArgs parseAppArgs(int argc, const char* const* argv);

// One-line usage string for `prog`.
std::string appUsage(const std::string& prog);

}  // namespace calibar
